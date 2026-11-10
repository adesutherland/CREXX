/* Small authenticated relay for the same cREXX RXBIN process used by Wasm. */
import {createServer} from 'node:http';
import {spawn} from 'node:child_process';
import {randomUUID, timingSafeEqual} from 'node:crypto';
import {readFile} from 'node:fs/promises';
import {join, resolve} from 'node:path';
import {fileURLToPath, pathToFileURL} from 'node:url';
import {StringDecoder} from 'node:string_decoder';
import {CORE_MODULES, preflightPackage, siteAssets} from './package-registry.mjs';

const token = process.env.CREXX_UI_TOKEN;
if (!token || token.length < 16) throw new Error('Set CREXX_UI_TOKEN (at least 16 characters)');
const build = resolve(process.env.CREXX_UI_BUILD ?? 'cmake-build-ui');
const vm = join(build, 'bin/rxvm');
const common = ['ui_web', 'ui_scene', 'ui_contract', 'ui_catalog', 'ui', 'library'];
const applications = {
  text: ['text_inspector_web', 'text_inspector'],
  graph: ['graph_web', 'scene_editors'],
  map: ['map_web', 'scene_editors'],
  process: ['process_web', 'process_designer'],
  probe: ['web_service_probe'],
};
const image = name => join(build, name.startsWith('text_') || name === 'web_service_probe' ?
  'examples/ui/text-inspector' : ['graph_web', 'map_web', 'scene_editors'].includes(name) ?
  'examples/ui/scene' : ['process_web', 'process_designer'].includes(name) ?
  'examples/ui/process-designer' : 'bin', `${name}.rxbin`);
const sessions = new Map();
const site = resolve(fileURLToPath(new URL('.', import.meta.url)));
const siteManifest = JSON.parse(await readFile(join(site, 'manifest.json'), 'utf8'));

function stopServices(session) {
  for (const controller of session.services.values()) controller.abort();
  session.services.clear();
}

function completeService(session, request, outcome) {
  if (session.ended || !sessions.has(session.id) || !session.latest) return;
  if (!outcome || typeof outcome.name !== 'string' || !outcome.payload ||
      typeof outcome.payload !== 'object')
    throw new Error('Invalid hosted application service outcome');
  const message = {version: 0, direction: 'event', name: outcome.name,
    target: request.target, payload: outcome.payload, request_id: request.request_id,
    session_id: session.id, sequence: 0};
  session.process.stdin.write(JSON.stringify({version: 1, type: 'complete',
    session: session.id, base: session.latest.revision, message}) + '\n');
}

function authorized(request) {
  const header = request.headers.authorization ?? '';
  const candidate = header.startsWith('Bearer ') ? header.slice(7) : '';
  const a = Buffer.from(candidate), b = Buffer.from(token);
  return a.length === b.length && timingSafeEqual(a, b);
}
function reply(response, code, body) {
  const text = JSON.stringify(body);
  response.writeHead(code, {'Content-Type': 'application/json', 'Cache-Control': 'no-store'});
  response.end(text);
}
async function body(request) {
  let text = '';
  for await (const part of request) {
    text += part;
    if (text.length > 262144) throw new Error('Input exceeds limit');
  }
  return JSON.parse(text);
}
async function createSession(app) {
  const pkg = siteManifest.packages?.[app];
  if (!pkg && !Object.hasOwn(applications, app)) throw new Error('Unknown UI application');
  if (sessions.size >= 16) throw new Error('Hosted UI session limit reached');
  let images;
  let service = null;
  if (pkg) {
    const checked = preflightPackage(pkg, 'hosted');
    for (const path of siteAssets(checked)) {
      if (!siteManifest.assets?.[path] || !(await readFile(join(site, path))).length)
        throw new Error(`Missing required UI asset: ${path}`);
    }
    const records = [checked.selected.entry, ...[...pkg.modules].reverse()];
    images = [...records.map(record => join(build, record.path)),
      ...CORE_MODULES.hosted.map(name => join(build, 'bin', `${name}.rxbin`))];
    for (const path of images) {
      if (!(await readFile(path)).length) throw new Error(`Missing required UI asset: ${path}`);
    }
    if (checked.selected.service) {
      service = await import(pathToFileURL(join(site, checked.selected.service.target)).href);
      if (typeof service.supports !== 'function' || typeof service.run !== 'function')
        throw new Error('Hosted application service must export supports and run');
    }
  } else images = [...applications[app], ...common].map(image);
  const id = `hosted-${randomUUID()}`;
  const process = spawn(vm, images.concat('-a', id),
    {stdio: ['pipe', 'pipe', 'pipe']});
  const session = {id, process, frames: [], sequence: 0, latest: null, services: new Map(),
    ended: false, lastTouched: Date.now()};
  sessions.set(id, session);
  const decoder = new StringDecoder('utf8');
  let pending = '';
  process.stdout.on('data', bytes => {
    pending += decoder.write(bytes);
    if (pending.length > 524288) { process.kill(); return; }
    let end;
    while ((end = pending.indexOf('\n')) >= 0) {
      const line = pending.slice(0, end);
      pending = pending.slice(end + 1);
      try {
        const frame = JSON.parse(line);
        if (frame.session !== id || frame.version !== 1) throw new Error('Invalid host frame');
        if (frame.type === 'view') session.latest = frame;
        if (frame.type === 'effect' && service?.supports(frame.message)) {
          const request = frame.message;
          if (request?.session_id !== id || typeof request?.request_id !== 'string' ||
              session.services.has(request.request_id) || !session.latest)
            throw new Error('Invalid hosted service request');
          const controller = new AbortController();
          controller.effectName = request.name;
          session.services.set(request.request_id, controller);
          void Promise.resolve().then(() => service.run(request, {signal: controller.signal,
            progress: outcome => {
              if (controller.signal.aborted || outcome?.name !== 'ui.effect.progress') return;
              completeService(session, request, outcome);
            }}))
            .then(outcome => {
              if (!controller.signal.aborted) completeService(session, request, outcome);
            })
            .catch(() => {
              if (!controller.signal.aborted) completeService(session, request, {
                name: 'ui.effect.failed', payload: {
                  code: 'service_failed', message: 'Application service failed',
                },
              });
            })
            .finally(() => session.services.delete(request.request_id));
          continue;
        }
        session.frames.push({sequence: ++session.sequence, frame});
        if (session.frames.length > 256) session.frames.shift();
      } catch { process.kill(); }
    }
  });
  process.stderr.on('data', bytes => { session.diagnostic = String(bytes).slice(0, 1024); });
  process.on('error', error => { session.diagnostic = String(error); session.ended = true; });
  process.on('close', (code, signal) => {
    session.ended = true;
    stopServices(session);
    if (session.frames.at(-1)?.frame.type !== 'closed') {
      const frame = {version: 1, type: 'refusal', session: id,
        revision: session.latest?.revision ?? 0,
        result: {code: 'host_lost', operation: 'browser',
          message: `Native UI session ended (${signal ?? code})`}};
      session.frames.push({sequence: ++session.sequence, frame});
    }
    setTimeout(() => sessions.delete(id), 60000).unref();
  });
  return session;
}

const contentTypes = {'.html': 'text/html', '.mjs': 'text/javascript', '.css': 'text/css', '.json': 'application/json',
  '.wasm': 'application/wasm', '.rxbin': 'application/octet-stream'};
const server = createServer(async (request, response) => {
  try {
    const url = new URL(request.url, 'http://localhost');
    if (!url.pathname.startsWith('/api/')) {
      const path = url.pathname === '/' ? 'index.html' : url.pathname.slice(1);
      if (!/^(index\.html|manifest\.json|[a-z-]+\.mjs|apps\/[a-z][a-z0-9_-]*\.mjs|styles\/[a-z][a-z0-9_-]*\.css|vm\/crexx_ui_vm\.(mjs|wasm)|modules\/[a-z][a-z0-9_]*\.rxbin)$/.test(path))
        return reply(response, 404, {error: 'Not found'});
      const bytes = await readFile(join(site, path));
      const ext = path.slice(path.lastIndexOf('.'));
      response.writeHead(200, {'Content-Type': contentTypes[ext], 'Cache-Control': 'no-store'});
      return response.end(bytes);
    }
    if (!authorized(request)) return reply(response, 401, {error: 'Unauthorized'});
    if (url.pathname === '/api/sessions' && request.method === 'POST') {
      const selection = await body(request);
      return reply(response, 201, {session: (await createSession(selection.app ?? 'text')).id});
    }
    const match = /^\/api\/sessions\/([a-z0-9-]+)(\/input)?$/.exec(url.pathname);
    const session = match && sessions.get(match[1]);
    if (!session) return reply(response, 404, {error: 'Unknown session'});
    if (request.method === 'DELETE' && !match[2]) {
      sessions.delete(session.id);
      stopServices(session);
      if (!session.ended) session.process.kill();
      return reply(response, 200, {closed: true});
    }
    if (request.method === 'GET' && !match[2]) {
      session.lastTouched = Date.now();
      const after = Number(url.searchParams.get('after') ?? 0);
      if (!Number.isSafeInteger(after) || after < 0) return reply(response, 400, {error: 'Invalid cursor'});
      const old = session.frames.length && after < session.frames[0].sequence - 1;
      const frames = old ? [{sequence: session.sequence, frame: session.latest}].filter(x => x.frame) :
        session.frames.filter(item => item.sequence > after);
      return reply(response, 200, {frames, ended: session.ended});
    }
    if (request.method === 'POST' && match[2]) {
      if (session.ended) return reply(response, 410, {error: 'Session ended'});
      session.lastTouched = Date.now();
      const input = await body(request);
      if (input.version !== 1 || input.session !== session.id ||
          !Number.isSafeInteger(input.base) || input.base < 0)
        return reply(response, 400, {error: 'Invalid input'});
      if (input.type === 'cancel' && input.base === session.latest?.revision &&
          (typeof input.request_id === 'string' || typeof input.effect === 'string')) {
        if (input.request_id) session.services.get(input.request_id)?.abort();
        else for (const controller of session.services.values())
          if (controller.effectName === input.effect) controller.abort();
      }
      session.process.stdin.write(JSON.stringify(input) + '\n');
      return reply(response, 202, {accepted: true});
    }
    return reply(response, 405, {error: 'Method not allowed'});
  } catch (error) { return reply(response, 400, {error: String(error)}); }
});
server.listen(Number(process.env.CREXX_UI_PORT ?? 8787), '127.0.0.1');
setInterval(() => {
  const now = Date.now();
  for (const session of sessions.values()) {
    if (session.ended || now - session.lastTouched <= 30000) continue;
    sessions.delete(session.id);
    stopServices(session);
    session.process.kill();
  }
}, 5000).unref();
function shutdown() {
  for (const session of sessions.values()) {
    stopServices(session);
    session.process.kill();
  }
  server.close();
}
process.on('SIGINT', shutdown);
process.on('SIGTERM', shutdown);
