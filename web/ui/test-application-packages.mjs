import assert from 'node:assert/strict';
import {readFile, mkdtemp, writeFile, copyFile, mkdir, rm} from 'node:fs/promises';
import {spawnSync} from 'node:child_process';
import {tmpdir} from 'node:os';
import {join, resolve} from 'node:path';
import {fileURLToPath} from 'node:url';
import {preflightPackage, preflightBrowserPackage, siteAssets} from './package-registry.mjs';

const graph = JSON.parse(await readFile(new URL('../../examples/ui/app-effects/graph.package.json', import.meta.url)));
const map = JSON.parse(await readFile(new URL('../../examples/ui/app-effects/map.package.json', import.meta.url)));
const rag = JSON.parse(await readFile(new URL('../../examples/ui/rag-query/rag-query.package.json', import.meta.url)));
for (const pkg of [graph, map]) {
  for (const route of ['gtk', 'ansi', 'local', 'hosted']) {
    const checked = preflightPackage(pkg, route);
    assert.equal(checked.modules.length, 3);
    assert.ok(checked.modules.at(-1).name.endsWith(route === 'hosted' || route === 'local' ? '_web' : `_${route}`));
    if (route === 'ansi') assert.ok(checked.selected.optionalOperations.includes('form.scene'));
  }
  assert.throws(() => preflightPackage({...pkg, contract: 2}, 'local'), /Incompatible/);
  const broken = structuredClone(pkg);
  broken.routes.local.requiredOperations.push('scene.unknown');
  assert.throws(() => preflightPackage(broken, 'local'), /Missing required UI operation/);
  const unavailable = structuredClone(pkg);
  unavailable.routes.ansi.requiredCapabilities.push(unavailable.routes.local.executor.capability);
  assert.throws(() => preflightPackage(unavailable, 'ansi'), /Missing required UI capability/);
  unavailable.routes.ansi.executor = unavailable.routes.local.executor;
  assert.throws(() => preflightPackage(unavailable, 'ansi'), /Unsupported executor/);
  const invalidAsset = structuredClone(pkg);
  invalidAsset.routes.local.executor.target = '../escape.mjs';
  assert.throws(() => preflightPackage(invalidAsset, 'local'), /Invalid UI executor/);
  const checked = preflightPackage(pkg, 'local');
  const assets = Object.fromEntries(siteAssets(checked).map(path => [path, 'hash']));
  const manifest = {packages: {[pkg.id]: pkg}, assets};
  assert.equal((await preflightBrowserPackage(manifest, pkg.id, 'local',
    async () => ({ok: true}))).pkg.id, pkg.id);
  delete assets[`modules/${checked.selected.entry.name}.rxbin`];
  await assert.rejects(preflightBrowserPackage(manifest, pkg.id, 'local',
    async () => ({ok: true})), /Missing required UI asset/);
  assets[`modules/${checked.selected.entry.name}.rxbin`] = 'hash';
  await assert.rejects(preflightBrowserPackage(manifest, pkg.id, 'local',
    async () => ({ok: false})), /Missing required UI asset/);
  const hostedAssets = Object.fromEntries(siteAssets(preflightPackage(pkg, 'hosted'))
    .map(path => [path, 'hash']));
  delete hostedAssets[checked.selected.executor.target];
  await assert.rejects(preflightBrowserPackage({packages: {[pkg.id]: pkg},
    assets: hostedAssets}, pkg.id, 'hosted', async () => ({ok: true})),
  /Missing required UI asset/);
}
const localRag = preflightPackage(rag, 'local');
const hostedRag = preflightPackage(rag, 'hosted');
assert.ok(siteAssets(localRag).includes('styles/rag-query.css'));
assert.ok(siteAssets(hostedRag).includes('styles/rag-query.css'));
assert.equal(hostedRag.selected.service.capability, 'rag.query');
assert.equal(localRag.selected.service, undefined);
assert.equal(localRag.selected.executor.capability, 'rag.query');
assert.equal(localRag.selected.executor.requiresStorage, false);
const badService = structuredClone(rag);
badService.routes.hosted.service.target = '../rag-query.mjs';
assert.throws(() => preflightPackage(badService, 'hosted'), /Invalid hosted UI service/);
const badStyle = structuredClone(rag);
badStyle.browserStyle.target = '../rag-query.css';
assert.throws(() => preflightPackage(badStyle, 'local'), /Invalid UI browser style/);
const ragAssets = Object.fromEntries(siteAssets(hostedRag).map(path => [path, 'hash']));
await preflightBrowserPackage({packages: {rag_demo: rag}, assets: ragAssets},
  'rag_demo', 'hosted', async () => ({ok: true}));
delete ragAssets[hostedRag.selected.service.target];
await assert.rejects(preflightBrowserPackage({packages: {rag_demo: rag}, assets: ragAssets},
  'rag_demo', 'hosted', async () => ({ok: true})), /Missing required UI asset/);
const scratch = await mkdtemp(join(tmpdir(), 'crexx-ui-package-test-'));
try {
  for (const pkg of [graph, map]) {
    for (const route of ['gtk', 'ansi']) {
      const broken = structuredClone(pkg);
      broken.routes[route].entry.path = `examples/ui/app-effects/missing/${broken.routes[route].entry.name}.rxbin`;
      const filename = join(scratch, `${pkg.id}-${route}.json`);
      await writeFile(filename, JSON.stringify(broken));
      const result = spawnSync(process.execPath,
        [fileURLToPath(new URL('./run-package.mjs', import.meta.url)),
          filename, resolve(process.argv[2] ?? 'cmake-build-ui'), route, '--check'],
        {encoding: 'utf8'});
      assert.notEqual(result.status, 0, `${pkg.id} ${route} missing entry must fail`);
      assert.equal(result.stdout, '');
    }
  }
  const external = join(scratch, 'external-team');
  await mkdir(external);
  await writeFile(join(external, 'graph.package.json'), JSON.stringify(graph));
  await copyFile(new URL('../../examples/ui/app-effects/graph-repository.mjs', import.meta.url),
    join(external, 'graph-repository.mjs'));
  const bundle = join(scratch, 'site');
  const packed = spawnSync(process.execPath,
    [fileURLToPath(new URL('./package-site.mjs', import.meta.url)),
      resolve(process.argv[2] ?? 'cmake-build-ui'),
      resolve(process.argv[3] ?? 'cmake-build-ui-wasm'), bundle,
      join(external, 'graph.package.json'),
      fileURLToPath(new URL('../../examples/ui/rag-query/rag-query.package.json', import.meta.url))],
    {encoding: 'utf8'});
  assert.equal(packed.status, 0, packed.stderr);
  const manifest = JSON.parse(await readFile(join(bundle, 'manifest.json'), 'utf8'));
  assert.ok(manifest.packages.graph_app);
  assert.ok(manifest.assets['apps/graph-repository.mjs']);
  assert.ok(manifest.packages.rag_demo);
  assert.ok(manifest.assets['services/rag-query.mjs']);
  assert.ok(manifest.assets['apps/rag-query-http.mjs']);
  assert.ok(manifest.assets['styles/rag-query.css']);
} finally { await rm(scratch, {recursive: true, force: true}); }
console.log('PASS: independent UI packages and preflight refusals');
