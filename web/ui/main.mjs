import {BrowserUI} from './projection.mjs';
import {LocalRoute} from './local.mjs';
import {HostedRoute} from './hosted.mjs';
import {preflightBrowserPackage} from './package-registry.mjs';

const root = document.getElementById('app');
const hosted = new URL(location.href).searchParams.get('hosted') === '1';
const app = new URL(location.href).searchParams.get('app') ?? 'text';
root.addEventListener('ui-refusal', ({detail}) => {
  const message = document.createElement('p');
  message.setAttribute('role', 'alert');
  message.textContent = `${detail.code}: ${detail.operation}: ${detail.message}`;
  root.append(message);
});
try {
  const manifest = await (await fetch('./manifest.json')).json();
  const packaged = Object.hasOwn(manifest.packages ?? {}, app);
  if (!packaged && !['text', 'graph', 'map', 'process'].includes(app))
    throw new Error('Unknown UI application');
  const checked = packaged ? await preflightBrowserPackage(manifest, app, hosted ? 'hosted' : 'local') : null;
  if (checked?.pkg.browserStyle) {
    const style = document.createElement('link');
    style.rel = 'stylesheet';
    style.href = new URL(`./${checked.pkg.browserStyle.target}`, import.meta.url).href;
    const loaded = new Promise((resolve, reject) => {
      style.onload = resolve;
      style.onerror = () => reject(new Error('Application stylesheet failed to load'));
    });
    document.head.append(style);
    await loaded;
  }
  const token = hosted ? prompt('Hosted UI access token') : null;
  const route = hosted ? new HostedRoute(token, '', app) : new LocalRoute(undefined, app);
  let ui;
  if (checked?.selected.executor) {
    const executor = checked.selected.executor;
    const module = await import(`./${executor.target}`);
    let storage;
    try { storage = localStorage; storage.getItem('crexx.ui.storage.probe'); }
    catch { storage = null; }
    const handler = (executor.requiresStorage === false || storage) ? module[executor.export](storage) :
      async (request, complete) => complete(request, 'ui.effect.failed',
        {code: 'unsupported_capability', message: executor.capability});
    ui = new BrowserUI(root, route, {executeEffect: async request => {
      if (await handler(request, ui.complete.bind(ui)) === false) await ui.fileEffect(request);
    }});
  } else ui = new BrowserUI(root, route);
  await ui.start();
}
catch (error) { root.textContent = `cREXX UI failed: ${error}`; }
