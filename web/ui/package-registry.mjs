/* Trusted package declarations and route preflight; no application names here. */
export const HOST_OPERATIONS = Object.freeze({
  gtk: ['form.label', 'form.statusbar', 'form.line', 'form.input', 'form.checkbox',
    'form.progress', 'form.button', 'form.choice', 'form.combo', 'form.integer',
    'form.scene', 'scene.rect', 'scene.edge', 'scene.tile', 'scene.unit',
    'input.pointer', 'input.keyboard'],
  ansi: ['form.label', 'form.statusbar', 'form.line', 'form.input', 'form.button',
    'form.choice', 'form.integer', 'input.pointer', 'input.keyboard'],
  local: ['form.label', 'form.line', 'form.input', 'form.button', 'form.scene',
    'scene.rect', 'scene.edge', 'scene.tile', 'scene.unit', 'input.pointer', 'input.keyboard'],
  hosted: ['form.label', 'form.line', 'form.input', 'form.button', 'form.scene',
    'scene.rect', 'scene.edge', 'scene.tile', 'scene.unit', 'input.pointer', 'input.keyboard'],
});

export const HOST_CAPABILITIES = Object.freeze({
  gtk: ['ui.resource.choose', 'io.text.read', 'io.text.write', 'ui.dialog',
    'ui.widgets', 'ui.collections', 'ui.pointer', 'ui.keyboard',
    'form.scene', 'scene.rect', 'scene.edge', 'scene.tile', 'scene.unit'],
  ansi: ['ui.resource.choose', 'io.text.read', 'io.text.write', 'ui.dialog',
    'ui.viewport', 'ui.keyboard', 'ui.text', 'ui.pointer', 'ui.scroll',
    'ui.widgets', 'ui.collections'],
  local: ['ui.resource.choose', 'io.text.read', 'form.scene', 'scene.rect',
    'scene.edge', 'scene.tile', 'scene.unit', 'ui.pointer', 'ui.keyboard'],
  hosted: ['ui.resource.choose', 'io.text.read', 'form.scene', 'scene.rect',
    'scene.edge', 'scene.tile', 'scene.unit', 'ui.pointer', 'ui.keyboard'],
});

export const CORE_MODULES = Object.freeze({
  gtk: ['ui_gtk', 'ui_local_resources', 'ui_scene', 'ui_contract', 'ui_catalog', 'ui', 'library'],
  ansi: ['ui_ansi', 'ui_dialogs', 'ui_terminal_view', 'ui_local_resources',
    'ui_scene', 'ui_contract', 'ui_catalog', 'ui', 'library'],
  local: ['library', 'ui', 'ui_catalog', 'ui_contract', 'ui_scene', 'ui_web'],
  hosted: ['ui_web', 'ui_scene', 'ui_contract', 'ui_catalog', 'ui', 'library'],
});

const safePath = path => typeof path === 'string' &&
  /^[a-zA-Z0-9_./-]+$/.test(path) &&
  path.split('/').every(segment => segment && segment !== '.' && segment !== '..');
const moduleRecord = record => record && /^[a-z][a-z0-9_]*$/.test(record.name) &&
  safePath(record.path) && record.path.endsWith(`/${record.name}.rxbin`);

export function preflightPackage(pkg, route, operations = HOST_OPERATIONS[route]) {
  if (!pkg || pkg.format !== 1 || pkg.contract !== 1 ||
      !/^[a-z][a-z0-9_]*$/.test(pkg.id ?? '') || !Array.isArray(pkg.modules) ||
      !pkg.modules.length || pkg.modules.some(record => !moduleRecord(record)))
    throw new Error('Incompatible UI package declaration');
  const selected = pkg.routes?.[route];
  if (!selected || !moduleRecord(selected.entry) ||
      !Array.isArray(selected.requiredOperations) ||
      !Array.isArray(selected.optionalOperations) ||
      !Array.isArray(selected.requiredCapabilities))
    throw new Error(`Missing or invalid ${route} route`);
  if (!Array.isArray(operations)) throw new Error(`Unknown UI route: ${route}`);
  for (const operation of selected.requiredOperations) {
    if (typeof operation !== 'string' || !operations.includes(operation))
      throw new Error(`Missing required UI operation: ${operation}`);
  }
  if (selected.optionalOperations.some(operation => typeof operation !== 'string'))
    throw new Error('Invalid optional UI operation');
  const modules = [...pkg.modules, selected.entry];
  if (new Set(modules.map(record => record.name)).size !== modules.length)
    throw new Error('Duplicate UI module');
  if (modules.some(record => CORE_MODULES[route].includes(record.name)))
    throw new Error('Application package redeclares a host module');
  if (selected.executor && (!safePath(selected.executor.path) ||
      !selected.executor.path.endsWith('.mjs') ||
      !/^apps\/[a-z][a-z0-9_-]*\.mjs$/.test(selected.executor.target ?? '') ||
      !/^[a-zA-Z][a-zA-Z0-9_]*$/.test(selected.executor.export ?? '') ||
      !/^[a-z][a-z0-9_.]*$/.test(selected.executor.capability ?? '') ||
      (selected.executor.requiresStorage !== undefined &&
        typeof selected.executor.requiresStorage !== 'boolean')))
    throw new Error('Invalid UI executor declaration');
  if (selected.executor && !['local', 'hosted'].includes(route))
    throw new Error(`Unsupported executor on ${route} route`);
  if (selected.service && (route !== 'hosted' || !safePath(selected.service.path) ||
      !selected.service.path.endsWith('.mjs') ||
      !/^services\/[a-z][a-z0-9_-]*\.mjs$/.test(selected.service.target ?? '') ||
      !/^[a-z][a-z0-9_.]*$/.test(selected.service.capability ?? '')))
    throw new Error('Invalid hosted UI service declaration');
  const capabilities = [...HOST_CAPABILITIES[route],
    ...(selected.executor ? [selected.executor.capability] : []),
    ...(selected.service ? [selected.service.capability] : [])];
  for (const capability of selected.requiredCapabilities) {
    if (typeof capability !== 'string' || !capabilities.includes(capability))
      throw new Error(`Missing required UI capability: ${capability}`);
  }
  return {pkg, route, selected, modules};
}

export function siteAssets(preflight) {
  const assets = [...CORE_MODULES[preflight.route].map(name => `modules/${name}.rxbin`),
    ...preflight.modules.map(record => `modules/${record.name}.rxbin`)];
  if (preflight.route === 'local')
    assets.push('vm/crexx_ui_vm.mjs', 'vm/crexx_ui_vm.wasm');
  if (preflight.selected.executor) assets.push(preflight.selected.executor.target);
  if (preflight.selected.service) assets.push(preflight.selected.service.target);
  return assets;
}

export async function preflightBrowserPackage(manifest, id, route, fetchAsset = fetch) {
  const pkg = manifest?.packages?.[id];
  const checked = preflightPackage(pkg, route);
  for (const path of siteAssets(checked)) {
    if (!manifest.assets?.[path]) throw new Error(`Missing required UI asset: ${path}`);
    if (path.startsWith('services/')) continue;
    const response = await fetchAsset(new URL(path, import.meta.url), {method: 'HEAD'});
    if (!response.ok) throw new Error(`Missing required UI asset: ${path}`);
  }
  return checked;
}
