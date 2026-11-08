/* Assemble a complete browser bundle from one native RXBIN build and Wasm VM. */
import {copyFile, mkdir, readFile, writeFile} from 'node:fs/promises';
import {resolve, join} from 'node:path';
import {fileURLToPath} from 'node:url';
import {createHash} from 'node:crypto';

if (process.argv.length !== 5) {
  console.error('Usage: node package-site.mjs NATIVE_BUILD WASM_BUILD OUTPUT_DIR');
  process.exit(2);
}
const native = resolve(process.argv[2]);
const wasm = resolve(process.argv[3]);
const output = resolve(process.argv[4]);
const source = fileURLToPath(new URL('.', import.meta.url));
const names = ['library', 'ui', 'ui_catalog', 'ui_contract', 'ui_scene', 'ui_web',
  'text_inspector', 'text_inspector_web', 'scene_editors', 'graph_web', 'map_web'];
const assets = [];
for (const name of names) assets.push({source: name.startsWith('text_') ?
  join(native, 'examples/ui/text-inspector', `${name}.rxbin`) :
  ['scene_editors', 'graph_web', 'map_web'].includes(name) ?
  join(native, 'examples/ui/scene', `${name}.rxbin`) :
  join(native, 'bin', `${name}.rxbin`), target: `modules/${name}.rxbin`});
for (const name of ['crexx_ui_vm.mjs', 'crexx_ui_vm.wasm'])
  assets.push({source: join(wasm, name), target: `vm/${name}`});
for (const name of ['index.html', 'main.mjs', 'projection.mjs', 'local.mjs',
  'local-worker.mjs', 'hosted.mjs', 'hosted-server.mjs', 'scene.mjs'])
  assets.push({source: join(source, name), target: name});
const manifest = {contract: 1, assets: {}};
for (const asset of assets) {
  const bytes = await readFile(asset.source);
  if (!bytes.length) throw new Error(`Empty UI asset: ${asset.source}`);
  await mkdir(join(output, asset.target, '..'), {recursive: true});
  await copyFile(asset.source, join(output, asset.target));
  manifest.assets[asset.target] = createHash('sha256').update(bytes).digest('hex');
}
await writeFile(join(output, 'manifest.json'), JSON.stringify(manifest, null, 2) + '\n');
console.log(`Packed ${assets.length} UI assets in ${output}`);
