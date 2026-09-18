import {build} from 'esbuild';
import {mkdir,writeFile} from 'node:fs/promises';
await mkdir('dist',{recursive:true});
await build({entryPoints:['src/index.js'],bundle:true,format:'iife',globalName:'edaEsbuildExportName',platform:'browser',outfile:'dist/index.js',minify:false});
await build({entryPoints:['src/generator.js'],bundle:true,format:'iife',globalName:'ArmRevA',platform:'browser',outfile:'dist/bridge.js',minify:false});
const manifest={name:'arm-six-axes-reva-schematic',uuid:'406f485a7ca945feab9a731e9b6ca1de',displayName:'Six-axis controller Rev.A schematic',description:'Official API schematic generator; no PCB operations.',version:'1.0.0',publisher:'arm_6_axes_0',engines:{eda:'^3.2.0'},license:'UNLICENSED',entry:'./dist/index',activationEvents:{},headerMenus:{home:[{id:'arm-reva-home',title:'Arm Rev.A',menuItems:[{id:'arm-reva-generate',title:'Generate Rev.A in empty window',registerFn:'generateRevA'}]}]}};
await writeFile('extension.json',JSON.stringify(manifest,null,2)+'\n');
