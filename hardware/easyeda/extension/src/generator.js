/* Official EasyEDA Pro API only. No DOM, GUI automation or PCB creation. */
export function Design_NormalizeName(value) {
    return String(value ?? '').toUpperCase().replace(/[^A-Z0-9]/g, '');
}

export function Design_CheckPinName(expected, actual) {
    const e = Design_NormalizeName(expected), a = Design_NormalizeName(actual);
    const aliases = {K: ['K', 'CATHODE'], A: ['A', 'ANODE'], EP: ['EP', 'EPAD', 'GND', 'PAD', 'THERMALPAD'],
        NC: ['NC', 'NOCONNECT'], CD: ['CD', 'DETECT', 'CARDDETECT'],
        DAT0: ['DAT0', 'DATA0', 'D0'], DAT1: ['DAT1', 'DATA1', 'D1'],
        DAT2: ['DAT2', 'DATA2', 'D2'], DAT3: ['DAT3', 'DATA3', 'D3', 'CDDAT3']};
    if (aliases[e]) return aliases[e].includes(a);
    return a === e || a.startsWith(e + 'OSC') || a.startsWith(e + 'TAMP') ||
        (e === 'VDD' && /^VDD\d*$/.test(a)) || (e === 'VSS' && /^VSS\d*$/.test(a));
}

export async function Library_ResolvePart(eda, part) {
    const libraryUuid = await eda.lib_LibrariesList.getSystemLibraryUuid();
    if (!libraryUuid) throw new Error('Official system library unavailable. Sign in/connect the official library.');
    const candidates = await eda.lib_Device.getByLcscIds([part.lcsc], libraryUuid, true);
    const matches = [];
    for (const item of candidates || []) {
        if (item.libraryUuid !== libraryUuid) continue;
        if (item.supplierId !== part.lcsc || Design_NormalizeName(item.manufacturerId) !== Design_NormalizeName(part.mpn)) continue;
        let detail, detailError;
        try { detail = await eda.lib_Device.get(item.uuid, libraryUuid); }
        catch (error) { detailError = String(error); }
        // Some client/catalog versions return no DeviceItem for a valid official
        // search result. create() explicitly supports ILIB_DeviceSearchItem.
        // Use only IDs actually returned by the official query; pin checks follow.
        if (detail && (detail.property?.supplierId !== part.lcsc ||
            detail.association?.footprint?.uuid !== item.footprintUuid ||
            detail.association?.symbol?.uuid !== item.otherProperty?.Symbol)) continue;
        if (!detail) {
            const symbolUuid = item.otherProperty?.Symbol;
            if (!symbolUuid || !item.footprintUuid) continue;
            detail = {...item, association: {
                symbol: {uuid:symbolUuid,libraryUuid},
                footprint: {uuid:item.footprintUuid,libraryUuid,name:item.footprintName}
            }, metadataSource:'OFFICIAL_SEARCH_ITEM', detailError};
        }
        if (!detail.association?.symbol || !detail.association?.footprint) continue;
        detail.catalogMpn = item.manufacturerId;
        detail.catalogFootprintName = item.footprintName;
        detail.catalogMetadataDiffers = Boolean(detail.property?.manufacturerId && detail.property.manufacturerId !== item.manufacturerId);
        matches.push(detail);
    }
    const unique = [...new Map(matches.map(d => [d.uuid, d])).values()];
    if (unique.length !== 1) throw new Error(`${part.lcsc} ${part.mpn}: expected one exact official device, found ${unique.length}. No substitute or UUID is guessed.`);
    return unique[0];
}

export async function Project_Create(eda, design) {
    const current = await eda.dmt_Project.getCurrentProjectInfo();
    if (current) throw new Error('Use a new empty EasyEDA window. Existing project is left untouched; openProject can discard unsaved changes.');
    const suffix = new Date().toISOString().replace(/[^0-9]/g, '');
    const projectUuid = await eda.dmt_Project.createProject(`${design.name}_${suffix}`, `arm-six-axes-reva-${suffix}`);
    if (!projectUuid) throw new Error('createProject returned no UUID. Check online/half-offline mode and project creation permission.');
    // Some desktop versions activate the created project automatically.
    // Opening that same project again can leave openProject's promise pending.
    if ((await eda.dmt_Project.getCurrentProjectInfo())?.uuid !== projectUuid &&
        !await eda.dmt_Project.openProject(projectUuid)) throw new Error(`Project ${projectUuid} created but openProject failed. Inspect it before any retry.`);
    return await Project_PrepareSchematic(eda, design, projectUuid);
}

export async function Project_PrepareSchematic(eda, design, projectUuid) {
    const active = await eda.dmt_Project.getCurrentProjectInfo();
    if (active?.uuid !== projectUuid) throw new Error('New project was not activated.');
    if (!active.friendlyName?.startsWith(design.name + '_')) throw new Error('Not a project created by this generator.');
    if ((await eda.dmt_Schematic.getAllSchematicPagesInfo()).length) throw new Error('Project already has schematic pages. Inspect before recovery.');
    const schematicUuid = await eda.dmt_Schematic.createSchematic();
    if (!schematicUuid) throw new Error('Cannot create schematic.');
    await eda.dmt_Schematic.modifySchematicName(schematicUuid, design.name);
    const existing = (await eda.dmt_Schematic.getAllSchematicPagesInfo()).filter(p => p.parentSchematicUuid === schematicUuid);
    const pages = {};
    for (let i = 0; i < design.sheets.length; i++) {
        const uuid = existing[i]?.uuid || await eda.dmt_Schematic.createSchematicPage(schematicUuid);
        if (!uuid || !await eda.dmt_Schematic.modifySchematicPageName(uuid, design.sheets[i])) throw new Error('Cannot create/name schematic page.');
        pages[design.sheets[i]] = uuid;
    }
    return {projectUuid, schematicUuid, pages};
}

export async function Sheet_Open(eda, project, sheet) {
    const active = await eda.dmt_Project.getCurrentProjectInfo();
    if (active?.uuid !== project.projectUuid) throw new Error('Project changed. Stop without touching the current project.');
    const pageUuid = project.pages[sheet];
    const tab = await eda.dmt_EditorControl.openDocument(pageUuid);
    if (!tab || !await eda.dmt_EditorControl.activateDocument(tab)) throw new Error(`Cannot activate ${sheet}`);
    const page = await eda.dmt_Schematic.getCurrentSchematicPageInfo();
    if (page?.uuid !== pageUuid) throw new Error('Wrong active schematic page.');
}

export async function Sheet_AddNotes(eda, design, sheet) {
    await eda.sch_PrimitiveText.create(30, -70, `${design.name} / ${sheet}`, 0, null, null, 18, true);
    for (const [i, note] of design.notes[sheet].entries()) {
        await eda.sch_PrimitiveText.create(30, -40 + i * 18, note, 0, null, null, 10);
    }
}

export async function Component_Place(eda, component, part, device) {
    if ((device.subPartNames || []).length > 1) throw new Error(`${component.ref}: multi-part library symbol requires explicit reviewed placement; not flattened.`);
    const primitive = await eda.sch_PrimitiveComponent.create(device, component.x, component.y,
        device.subPartNames?.[0] || '', 0, false, !component.dnp, true);
    if (!primitive) throw new Error(`Cannot place ${component.ref}`);
    const id = primitive.getState_PrimitiveId();
    const changed = await eda.sch_PrimitiveComponent.modify(id, {designator: component.ref,
        name: part.value, manufacturer: part.manufacturer, manufacturerId: part.mpn,
        supplier: 'LCSC', supplierId: part.lcsc,
        otherProperty: {Datasheet: part.datasheet, Notes: component.notes, DNP: component.dnp ? 'YES' : 'NO', Revision: 'A'}});
    if (!changed) throw new Error(`Cannot set ${component.ref} properties.`);
    const pinObjects = await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(id);
    if (!pinObjects?.length) throw new Error(`${component.ref}: no symbol pins.`);
    const pins = pinObjects.map(p => ({number: p.getState_PinNumber(), name: p.getState_PinName(),
        x: p.getState_X(), y: p.getState_Y(), rotation: p.getState_Rotation(), id: p.getState_PrimitiveId()}));
    const connected = [];
    for (const pin of pins) {
        if (!Object.hasOwn(component.pins, pin.number)) {
            const shell = component.extraGroundPinsByName?.includes(Design_NormalizeName(pin.name));
            if (!shell) throw new Error(`${component.ref}: unexpected pin ${pin.number} (${pin.name}). Must be reviewed before wiring.`);
            pin.net = 'GND';
        } else pin.net = component.pins[pin.number];
        const expected = component.pinNames[pin.number];
        if (expected && !Design_CheckPinName(expected, pin.name)) throw new Error(`${component.ref}.${pin.number}: datasheet=${expected}, library=${pin.name}; not wired.`);
        connected.push(pin);
    }
    for (const number of Object.keys(component.pins)) {
        if (!pins.some(p => p.number === number)) throw new Error(`${component.ref}: missing pin ${number}, including mandatory exposed pads.`);
    }
    if (component.part === 'tf' && !pins.some(p => !Object.hasOwn(component.pins,p.number) && p.net === 'GND')) {
        throw new Error('TF socket shell missing from symbol. Card detect return and shell pads require review.');
    }
    return {ref: component.ref, id, pins: connected, deviceUuid: device.uuid,
        libraryUuid: device.libraryUuid, association: device.association};
}

export async function Component_WirePins(eda, placed, offset = 0, count = 10) {
    const objects = await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(placed.id);
    if (!objects) throw new Error('Component pins disappeared.');
    const xs = placed.pins.map(p => p.x), ys = placed.pins.map(p => p.y);
    const midX = (Math.min(...xs) + Math.max(...xs)) / 2;
    const midY = (Math.min(...ys) + Math.max(...ys)) / 2;
    const records = [];
    for (const pin of placed.pins.slice(offset, offset + count)) {
        const object = objects.find(p => p.getState_PrimitiveId() === pin.id);
        if (!object) throw new Error('Pin identity changed.');
        if (pin.net === null) {
            if (!await eda.sch_PrimitivePin.modify(object, {noConnected: true})) throw new Error('Cannot set no-connect.');
            records.push({pin: pin.number, net: null});
            continue;
        }
        // Pin X/Y is the external electrical endpoint (official format/schematic/pin.md).
        // Orientation axis follows the observed library pin rotation; choose outward sign geometrically.
        const horizontal = Math.abs(Math.cos(pin.rotation * Math.PI / 180)) > 0.5;
        const dx = horizontal ? (pin.x <= midX ? -25 : 25) : 0;
        const dy = horizontal ? 0 : (pin.y <= midY ? -25 : 25);
        const endX = pin.x + dx, endY = pin.y + dy;
        const wire = await eda.sch_PrimitiveWire.create([pin.x, pin.y, endX, endY], pin.net);
        const port = await eda.sch_PrimitiveComponent.createNetPort('BI', pin.net, endX, endY,
            horizontal ? (dx < 0 ? 180 : 0) : (dy < 0 ? 270 : 90), false);
        if (!wire || !port) throw new Error(`Wire/net port failed for ${placed.ref}.${pin.number}`);
        records.push({pin: pin.number, net: pin.net});
    }
    return records;
}

export function Footprint_ReadPads(source) {
    // Official public v3 document format: header || data |, one record per line.
    const pads = [];
    for (const line of source.split(/\r?\n/)) {
        if (!line.trim()) continue;
        const split = line.indexOf('||');
        if (split < 0) continue;
        const header = JSON.parse(line.slice(0, split));
        if (header.type !== 'PAD') continue;
        const data = JSON.parse(line.slice(split + 2).replace(/\|\s*$/, ''));
        pads.push({number: String(data.num), x: data.centerX, y: data.centerY,
            hole: data.hole, plated: data.plated, shape: data.defaultPad});
    }
    if (!pads.length) throw new Error('No pads decoded: unsupported footprint document format; manual review required.');
    return pads;
}

export async function Sheet_AuditAndSave(eda, placed) {
    const sources = await eda.sys_FileManager.getDocumentFootprintSources();
    const audits = [];
    for (const component of placed) {
        const footprint = component.association.footprint;
        const source = sources.find(s => s.footprintUuid === footprint.uuid);
        if (!source) throw new Error(`${component.ref}: footprint source absent.`);
        const pads = Footprint_ReadPads(source.documentSource);
        for (const pin of component.pins) {
            if (!pads.some(p => p.number === pin.number)) throw new Error(`${component.ref}: symbol pin ${pin.number} has no footprint pad.`);
        }
        const extraPads = pads.filter(p => !component.pins.some(pin => pin.number === p.number));
        // Extra numbered pads can be electrically meaningful. Do not silently treat them as mechanics.
        if (extraPads.some(p => p.number && p.number !== '0')) throw new Error(`${component.ref}: extra numbered footprint pads require explicit mechanical/electrical review.`);
        audits.push({ref:component.ref,footprintUuid:footprint.uuid,pads,
            status:'PIN_NUMBERS_MATCH; DIMENSIONS_AND_POLARITY_REVIEW_REQUIRED'});
    }
    if (!await eda.sch_Document.save()) throw new Error('Schematic save failed.');
    await eda.dmt_EditorControl.zoomToAllPrimitives();
    return audits;
}

export async function Sheet_CheckERC(eda) {
    const errors = await eda.sch_Drc.check(true, false, true);
    // Verbose array support was added in newer clients. Older clients expose
    // the documented boolean result only; preserve it without inventing details.
    if (typeof errors === 'boolean') return {executed:true, errors:null,
        passed:errors, detailAvailable:false,
        status:errors ? 'LEGACY_CHECK_PASSED_NO_DETAILS' : 'LEGACY_CHECK_FAILED_NO_DETAILS'};
    if (!Array.isArray(errors)) throw new Error('Unexpected ERC response; do not report PASS.');
    return {executed: true, errors, detailAvailable:true, status: errors.length ? 'ISSUES_REPORTED' : 'NO_ISSUES_REPORTED'};
}

export async function Project_Export(eda, outputPath) {
    const file = await eda.sys_FileManager.getProjectFile('arm_6_axes_0_revA.epro', undefined, 'epro');
    if (!file) throw new Error('Editable project export failed.');
    if (outputPath) {
        if (!await eda.sys_FileSystem.saveFileToFileSystem(outputPath, file, undefined, false)) throw new Error('Export path exists or is not writable; no file overwritten.');
    } else await eda.sys_FileSystem.saveFile(file, 'arm_6_axes_0_revA.epro');
    return {editableProjectExported:true,size:file.size};
}

export async function Review_ExportSheet(eda, outputDirectory, sheet) {
    const svg = await eda.sch_ManufactureData.getSvgFile(sheet + '.svg');
    const netlist = await eda.sch_ManufactureData.getNetlistFile(sheet + '.net');
    if (!svg || !netlist) throw new Error('Review SVG/netlist export unavailable.');
    for (const [file, suffix] of [[svg,'.svg'],[netlist,'.net']]) {
        if (!await eda.sys_FileSystem.saveFileToFileSystem(outputDirectory + '/' + sheet + suffix, file, undefined, false)) throw new Error('Review export failed or file exists.');
    }
    return {svg:true,netlist:true};
}

export async function Design_Generate(eda, design) {
    const devices = {};
    for (const [key, part] of Object.entries(design.parts)) devices[key] = await Library_ResolvePart(eda, part);
    const project = await Project_Create(eda, design);
    const report = {project,parts:Object.fromEntries(Object.entries(devices).map(([k,d]) => [k,{uuid:d.uuid,libraryUuid:d.libraryUuid,association:d.association}])),sheets:{},pcb:'NOT_STARTED'};
    for (const sheet of design.sheets) {
        await Sheet_Open(eda, project, sheet);
        await Sheet_AddNotes(eda, design, sheet);
        const placed = [];
        for (const component of design.components.filter(c => c.sheet === sheet)) {
            const item = await Component_Place(eda, component, design.parts[component.part], devices[component.part]);
            for (let i=0;i<item.pins.length;i+=10) await Component_WirePins(eda,item,i,10);
            placed.push(item);
        }
        report.sheets[sheet] = {placed,audit:await Sheet_AuditAndSave(eda,placed),erc:await Sheet_CheckERC(eda)};
    }
    await eda.sys_FileSystem.saveFile(new Blob([JSON.stringify(report,null,2)],{type:'application/json'}),'revA_runtime_audit.json');
    report.export = await Project_Export(eda);
    return report;
}
