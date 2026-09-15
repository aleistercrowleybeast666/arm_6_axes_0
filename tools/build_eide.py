"""Exercise the installed EIDE backend from the committed project (requires PyYAML).

Optional arguments: --eide-root /path/to/cl.eide-version --gcc-root /path/to/toolchain.
EIDE itself reads eide.yml directly; this script is an additional CLI reproducibility check.
"""
import argparse
import copy
import fnmatch
import json
import os
from pathlib import Path
import shutil
import subprocess
import yaml

ROOT=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser()
parser.add_argument('--eide-root',type=Path)
parser.add_argument('--gcc-root',type=Path)
args=parser.parse_args()
if args.eide_root is None:
    matches=sorted((Path.home()/'.vscode/extensions').glob('cl.eide-*'))
    if not matches:raise SystemExit('EIDE not found; pass --eide-root')
    args.eide_root=matches[-1]
if args.gcc_root is None:
    gcc=shutil.which('arm-none-eabi-gcc')
    if not gcc:raise SystemExit('arm-none-eabi-gcc must be on PATH or pass --gcc-root')
    args.gcc_root=Path(gcc).parent.parent
project=yaml.safe_load((ROOT/'.eide/eide.yml').read_text(encoding='utf-8'))
target=project['targets']['Debug'];config=target['toolchainConfigMap']['GCC'];options=copy.deepcopy(config['options'])
sources=[]
for directory in project['srcDirs']:
    sources.extend(p.relative_to(ROOT).as_posix() for p in (ROOT/directory).rglob('*.c'))
sources.extend(f['path'] for f in project['virtualFolder']['files'])
sources=sorted(set(sources)-set(target['excludeList']))
file_options=yaml.safe_load((ROOT/'.eide/files.options.yml').read_text())['options']['Debug']['files']
source_params={s:' '.join(v for pattern,v in file_options.items() if fnmatch.fnmatch(s,pattern)) for s in sources}
options['global'].update({'toolPrefix':'arm-none-eabi-','microcontroller-cpu':'cortex-m4-sp','microcontroller-fpu':'cortex-m4-sp','microcontroller-float':'cortex-m4-sp','$arch-extensions':'','$clang-arch-extensions':'','$armlink-arch-extensions':''})
options['linker'].update({'$toolName':'gcc','link-scatter':[(ROOT/config['scatterFilePath']).as_posix()]})
out=Path(project['outDir'])/'Debug';(ROOT/out).mkdir(parents=True,exist_ok=True)
backend=args.eide_root/'res/tools/win32/unify_builder/unify_builder.exe'
params=dict(name=project['name'],target='Debug',toolchain='GCC',toolchainLocation=str(args.gcc_root),
    toolchainCfgFile=str(args.eide_root/'res/data/models/arm.gcc.model.json'),buildMode='fast|multhread',
    showRepathOnLog=True,threadNum=min(8,os.cpu_count() or 1),rootDir=str(ROOT),outDir=out.as_posix(),dumpPath=out.as_posix(),
    incDirs=target['cppPreprocessAttrs']['incList'],libDirs=[],defines=target['cppPreprocessAttrs']['defineList'],
    sourceList=sources,alwaysInBuildSources=[],sourceParams=source_params,options=options,sysPaths=[],env={
        'workspaceFolder':str(ROOT),'ProjectRoot':str(ROOT),'ProjectName':project['name'],'ConfigName':'Debug',
        'OutDir':str(ROOT/out),'OutDirRoot':project['outDir'],'OutDirBase':out.as_posix(),
        'ToolchainRoot':str(args.gcc_root),'SYS_Platform':'win32','SYS_DirSep':'\\','SYS_PathSep':';'})
path=ROOT/out/'builder.params';path.write_text(json.dumps(params,indent=2),encoding='utf-8')
subprocess.run([str(backend),'-p',str(path),'--rebuild','--no-color'],cwd=ROOT,check=True)
