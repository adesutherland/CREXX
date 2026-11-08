import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {join, resolve} from 'node:path';

const root = resolve(process.argv[2] ?? 'cmake-build-ui-web-site');
const manifest = JSON.parse(await readFile(join(root, 'manifest.json'), 'utf8'));
assert.equal(manifest.contract, 1);
const required = ['index.html', 'main.mjs', 'projection.mjs', 'scene.mjs',
  'local.mjs', 'local-worker.mjs', 'hosted.mjs', 'hosted-server.mjs',
  'vm/crexx_ui_vm.mjs', 'vm/crexx_ui_vm.wasm',
  ...['library', 'ui', 'ui_catalog', 'ui_contract', 'ui_scene', 'ui_web',
    'text_inspector', 'text_inspector_web', 'web_service_probe',
    'scene_editors', 'graph_web', 'map_web'].map(name => `modules/${name}.rxbin`)];
assert.deepEqual(Object.keys(manifest.assets).sort(), required.sort());
for (const [name, expected] of Object.entries(manifest.assets)) {
  const bytes = await readFile(join(root, name));
  assert.ok(bytes.length > 0, `${name} is empty`);
  assert.equal(createHash('sha256').update(bytes).digest('hex'), expected, name);
  if (name.endsWith('.wasm')) assert.equal(bytes.subarray(0, 4).toString('hex'), '0061736d');
}
console.log('PASS: browser UI package closure and asset hashes');
