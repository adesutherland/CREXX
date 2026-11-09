import {BrowserUI} from './projection.mjs';
import {LocalRoute} from './local.mjs';
import {HostedRoute} from './hosted.mjs';

const root = document.getElementById('app');
const hosted = new URL(location.href).searchParams.get('hosted') === '1';
const app = new URL(location.href).searchParams.get('app') ?? 'text';
if (!['text', 'graph', 'map', 'process'].includes(app)) throw new Error('Unknown UI application');
const token = hosted ? prompt('Hosted UI access token') : null;
const route = hosted ? new HostedRoute(token, '', app) : new LocalRoute(undefined, app);
const ui = new BrowserUI(root, route);
root.addEventListener('ui-refusal', ({detail}) => {
  const message = document.createElement('p');
  message.setAttribute('role', 'alert');
  message.textContent = `${detail.code}: ${detail.operation}: ${detail.message}`;
  root.append(message);
});
try { await ui.start(); }
catch (error) { root.textContent = `cREXX UI failed: ${error}`; }
