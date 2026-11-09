/* Assemble a complete browser bundle from one native RXBIN build and Wasm VM. */
import {copyFile, mkdir, readFile, writeFile} from 'node:fs/promises';
import {resolve, join, dirname} from 'node:path';
import {fileURLToPath} from 'node:url';
import {createHash} from 'node:crypto';
import {preflightPackage} from './package-registry.mjs';

if (process.argv.length < 5) {
  console.error('Usage: node package-site.mjs NATIVE_BUILD WASM_BUILD OUTPUT_DIR [PACKAGE_JSON ...]');
  process.exit(2);
}
const native = resolve(process.argv[2]);
const wasm = resolve(process.argv[3]);
const output = resolve(process.argv[4]);
const source = fileURLToPath(new URL('.', import.meta.url));
const names = ['library', 'ui', 'ui_catalog', 'ui_contract', 'ui_scene', 'ui_web',
  'text_inspector', 'text_inspector_web', 'web_service_probe',
  'scene_editors', 'graph_web', 'map_web', 'process_designer', 'process_web'];
const assets = [];
function addAsset(sourcePath, target) {
  const prior = assets.find(asset => asset.target === target);
  if (prior && prior.source !== sourcePath)
    throw new Error(`Conflicting UI asset: ${target}`);
  if (!prior) assets.push({source: sourcePath, target});
}
for (const name of names) assets.push({source: name.startsWith('text_') || name === 'web_service_probe' ?
  join(native, 'examples/ui/text-inspector', `${name}.rxbin`) :
  ['scene_editors', 'graph_web', 'map_web'].includes(name) ?
  join(native, 'examples/ui/scene', `${name}.rxbin`) :
  ['process_designer', 'process_web'].includes(name) ?
  join(native, 'examples/ui/process-designer', `${name}.rxbin`) :
  join(native, 'bin', `${name}.rxbin`), target: `modules/${name}.rxbin`});
for (const name of ['crexx_ui_vm.mjs', 'crexx_ui_vm.wasm'])
  assets.push({source: join(wasm, name), target: `vm/${name}`});
for (const name of ['index.html', 'main.mjs', 'projection.mjs', 'package-registry.mjs', 'local.mjs',
  'local-worker.mjs', 'hosted.mjs', 'hosted-server.mjs', 'scene.mjs'])
  assets.push({source: join(source, name), target: name});
const packages = {};
for (const filename of process.argv.slice(5)) {
  const declaration = resolve(filename);
  const pkg = JSON.parse(await readFile(declaration, 'utf8'));
  if (Object.hasOwn(packages, pkg.id)) throw new Error(`Duplicate UI package: ${pkg.id}`);
  for (const route of Object.keys(pkg.routes ?? {})) preflightPackage(pkg, route);
  packages[pkg.id] = pkg;
  for (const record of pkg.modules) {
    const target = `modules/${record.name}.rxbin`;
    addAsset(join(native, record.path), target);
  }
  for (const route of ['local', 'hosted']) {
    const selected = pkg.routes[route];
    if (!selected) continue;
    const target = `modules/${selected.entry.name}.rxbin`;
    addAsset(join(native, selected.entry.path), target);
    if (selected.executor)
      addAsset(join(dirname(declaration), selected.executor.path), selected.executor.target);
  }
}
const manifest = {contract: 1, packages, assets: {}};
for (const asset of assets) {
  const bytes = await readFile(asset.source);
  if (!bytes.length) throw new Error(`Empty UI asset: ${asset.source}`);
  await mkdir(join(output, asset.target, '..'), {recursive: true});
  await copyFile(asset.source, join(output, asset.target));
  manifest.assets[asset.target] = createHash('sha256').update(bytes).digest('hex');
}
await writeFile(join(output, 'manifest.json'), JSON.stringify(manifest, null, 2) + '\n');
console.log(`Packed ${assets.length} UI assets in ${output}`);
