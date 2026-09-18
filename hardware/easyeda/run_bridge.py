"""Run the reviewed generator through the official local EasyEDA bridge.

Default is read-only discovery. --generate creates a NEW project in an EMPTY
window. Mutations are never retried after a timeout: inspect the partial project.
Only loopback ports 49620..49629 are accepted. No GUI or PCB operations.
"""
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import sys
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[1]


def request(port, path, payload=None, timeout=40):
    body = None if payload is None else json.dumps(payload).encode('utf-8')
    req = Request(f'http://127.0.0.1:{port}{path}', data=body,
                  headers={'Content-Type': 'application/json'})
    try:
        with urlopen(req, timeout=timeout) as response:
            return json.load(response)
    except HTTPError as error:
        raise RuntimeError(error.read().decode('utf-8', errors='replace')) from error


def discover():
    bridges = []
    for port in range(49620, 49630):
        try:
            health = request(port, '/health', timeout=1)
            if health.get('service') == 'easyeda-bridge':
                bridges.append({'port': port, 'health': health,
                                'clients': request(port, '/eda-windows', timeout=2)})
        except (OSError, URLError, RuntimeError, ValueError):
            continue
    return bridges


def generate(port, window_id, resume_empty_project=None):
    design = json.loads((ROOT/'design/controller_revA.json').read_text(encoding='utf-8'))
    bundle = (ROOT/'easyeda/extension/dist/bridge.js').read_text(encoding='utf-8')
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S_%fZ')
    output = ROOT/'easyeda/output'/stamp
    output.mkdir(parents=True, exist_ok=False)
    report = {'status': 'RUNNING', 'startedAt': stamp, 'project': None, 'parts': {},
              'sheets': {}, 'pcb': 'NOT_STARTED', 'lastOperation': None}

    def checkpoint():
        temporary = output/'runtime_audit.json.tmp'
        temporary.write_text(
            json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
        temporary.replace(output/'runtime_audit.json')

    def invoke(name, *args, sheet=None):
        # Recheck client identity and the active document before EVERY mutation.
        health = request(port, '/health', timeout=2)
        if health.get('service') != 'easyeda-bridge' or not health.get('edaConnected'):
            raise RuntimeError('Official bridge disconnected. No retry of previous mutations.')
        guard = ''
        if report['project']:
            project = report['project']
            guard = ('if ((await eda.dmt_Project.getCurrentProjectInfo())?.uuid !== '
                     +json.dumps(project['projectUuid'])+') throw new Error("Project changed");\n')
            if sheet:
                guard += ('if ((await eda.dmt_Schematic.getCurrentSchematicPageInfo())?.uuid !== '
                          +json.dumps(project['pages'][sheet])+') throw new Error("Page changed");\n')
        report['lastOperation'] = {'name': name, 'sheet': sheet, 'completed': False}
        checkpoint()
        code = bundle+'\n'+guard+'return await ArmRevA.'+name+'(eda'
        code += ''.join(','+json.dumps(arg, ensure_ascii=False) for arg in args)+');'
        response = request(port, '/execute', {'windowId': window_id, 'code': code})
        if not response.get('success') or response.get('windowId') != window_id:
            raise RuntimeError(f'Unexpected bridge response: {response}')
        report['lastOperation']['completed'] = True
        checkpoint()
        return response.get('result')

    try:
        devices = {}
        for key, part in design['parts'].items():
            print(f'Library: {part["lcsc"]} {part["mpn"]}', flush=True)
            devices[key] = invoke('Library_ResolvePart', part)
            report['parts'][key] = devices[key]
            checkpoint()
        report['project'] = (invoke('Project_PrepareSchematic', design, resume_empty_project)
                             if resume_empty_project else invoke('Project_Create', design))
        checkpoint()
        for sheet in design['sheets']:
            print(f'Schematic: {sheet}', flush=True)
            invoke('Sheet_Open', report['project'], sheet)
            invoke('Sheet_AddNotes', design, sheet, sheet=sheet)
            placed = []
            report['sheets'][sheet] = {'placed': placed}
            for component in (c for c in design['components'] if c['sheet'] == sheet):
                item = invoke('Component_Place', component, design['parts'][component['part']],
                              devices[component['part']], sheet=sheet)
                placed.append(item)
                checkpoint()
                for offset in range(0, len(item['pins']), 10):
                    invoke('Component_WirePins', item, offset, 10, sheet=sheet)
            report['sheets'][sheet]['audit'] = invoke('Sheet_AuditAndSave', placed, sheet=sheet)
            report['sheets'][sheet]['erc'] = invoke('Sheet_CheckERC', sheet=sheet)
            report['sheets'][sheet]['exports'] = invoke('Review_ExportSheet', output.as_posix(), sheet, sheet=sheet)
            checkpoint()
        report['export'] = invoke('Project_Export', (output/'arm_6_axes_0_revA.epro').as_posix())
        report['status'] = 'GENERATED_REQUIRES_VISUAL_AND_NETLIST_REVIEW'
        checkpoint()
        print(f'Generated: {output}\nERC findings are in runtime_audit.json; not automatically approved.')
    except Exception as error:
        report['status'] = 'STOPPED_REQUIRES_INSPECTION'
        report['error'] = str(error)
        checkpoint()
        print(f'Stopped: {error}\nAudit: {output}\nDo not retry into a partially generated project.', file=sys.stderr)
        return 1
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--status', action='store_true', help='read-only discovery (default)')
    parser.add_argument('--generate', action='store_true', help='create a new schematic project')
    parser.add_argument('--port', type=int, choices=range(49620, 49630))
    parser.add_argument('--window-id', help='explicit choice when more than one window is connected')
    parser.add_argument('--resume-empty-project', help='after inspection only: UUID of the empty project created by a timed-out run')
    args = parser.parse_args()
    bridges = discover()
    if not args.generate:
        print(json.dumps(bridges, indent=2, ensure_ascii=False))
        return 0
    targets = [(b['port'], w['windowId']) for b in bridges for w in b['clients']['windows']
               if w.get('connected') and (args.port is None or args.port == b['port'])
               and (args.window_id is None or args.window_id == w['windowId'])]
    if len(targets) != 1:
        print(json.dumps(bridges, indent=2, ensure_ascii=False))
        print('Need exactly one connected target. Re-run the official gateway or select --port/--window-id.', file=sys.stderr)
        return 2
    return generate(*targets[0], args.resume_empty_project)


if __name__ == '__main__':
    raise SystemExit(main())
