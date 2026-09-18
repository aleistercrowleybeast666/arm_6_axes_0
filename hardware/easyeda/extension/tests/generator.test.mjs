import test from 'node:test';
import assert from 'node:assert/strict';
import {Design_CheckPinName,Footprint_ReadPads,Library_ResolvePart,Project_Create,Component_WirePins,Sheet_CheckERC} from '../src/generator.js';

test('reject EN/VIN and CAN TX/RX swapped symbol pins',()=>{
    assert.equal(Design_CheckPinName('EN','VIN'),false);
    assert.equal(Design_CheckPinName('TXD','RXD'),false);
    assert.equal(Design_CheckPinName('PH0','PH0-OSC_IN'),true);
    assert.equal(Design_CheckPinName('EP','GND'),true);
    assert.equal(Design_CheckPinName('PA6','PA7'),false);
});
test('parse official footprint pads without creating a PCB',()=>{
    assert.deepEqual(Footprint_ReadPads('{"type":"PAD","id":"p"}||{"num":"9","centerX":0,"centerY":0,"hole":null,"plated":1,"defaultPad":["RECT",3,3]}|')[0].number,'9');
    assert.throws(()=>Footprint_ReadPads('unknown format'),/unsupported/);
});
test('do not replace an open user project',async()=>{
    let created=false;
    await assert.rejects(Project_Create({dmt_Project:{getCurrentProjectInfo:async()=>({uuid:'user-project'}),createProject:async()=>{created=true;}}},{}),/empty/);
    assert.equal(created,false);
});
test('reject unknown or ambiguous library devices before project mutation',async()=>{
    const eda={lib_LibrariesList:{getSystemLibraryUuid:async()=>'official'},lib_Device:{getByLcscIds:async()=>[{uuid:'private',libraryUuid:'personal'}]}};
    await assert.rejects(Library_ResolvePart(eda,{lcsc:'C28730',mpn:'STM32F407VET6'}),/found 0/);
});
test('pins connect at electrical endpoints and explicit unused pin gets NC',async()=>{
    const wires=[],ports=[],nc=[];
    const pins=[{id:'p1',number:'1',name:'VDD',x:10,y:20,rotation:0,net:'3V3'},{id:'p2',number:'2',name:'NC',x:30,y:20,rotation:180,net:null}];
    const eda={sch_PrimitiveComponent:{getAllPinsByPrimitiveId:async()=>pins.map(p=>({getState_PrimitiveId:()=>p.id})),createNetPort:async(...p)=>{ports.push(p);return {};}},sch_PrimitiveWire:{create:async(...p)=>{wires.push(p);return {};}},sch_PrimitivePin:{modify:async(...p)=>{nc.push(p);return {};}}};
    await Component_WirePins(eda,{id:'u',ref:'U',pins});
    assert.deepEqual(wires[0],[[10,20,-15,20],'3V3']);
    assert.deepEqual(ports[0].slice(0,4),['BI','3V3',-15,20]);
    assert.equal(nc.length,1);assert.equal(nc[0][1].noConnected,true);
});

test('official search result remains usable when offline metadata is absent',async()=>{
    const item={uuid:'device',libraryUuid:'official',supplierId:'C28730',manufacturerId:'STM32F407VET6',footprintUuid:'footprint',otherProperty:{Symbol:'symbol'}};
    const eda={lib_LibrariesList:{getSystemLibraryUuid:async()=>'official'},lib_Device:{getByLcscIds:async()=>[item],get:async()=>null}};
    const result=await Library_ResolvePart(eda,{lcsc:'C28730',mpn:'STM32F407VET6'});
    assert.equal(result.association.symbol.uuid,'symbol');
    assert.equal(result.metadataSource,'OFFICIAL_SEARCH_ITEM');
    await assert.rejects(Library_ResolvePart(eda,{lcsc:'C28730',mpn:'STM32F407VGT6'}),/found 0/);
});

test('legacy ERC boolean cannot be confused with an empty detailed report',async()=>{
    const result=await Sheet_CheckERC({sch_Drc:{check:async()=>false}});
    assert.equal(result.executed,true);assert.equal(result.passed,false);
    assert.equal(result.errors,null);assert.equal(result.detailAvailable,false);
    await assert.rejects(Sheet_CheckERC({sch_Drc:{check:async()=>null}}),/Unexpected/);
});
