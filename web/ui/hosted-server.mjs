/* Small authenticated relay for the same cREXX RXBIN process used by Wasm. */
import {createServer} from 'node:http';
import {spawn} from 'node:child_process';
import {randomUUID, timingSafeEqual} from 'node:crypto';
import {readFile} from 'node:fs/promises';
import {join, resolve} from 'node:path';
import {fileURLToPath} from 'node:url';
import {StringDecoder} from 'node:string_decoder';

const token = process.env.CREXX_UI_TOKEN;
if (!token || token.length < 16) throw new Error('Set CREXX_UI_TOKEN (at least 16 characters)');
const build = resolve(process.env.CREXX_UI_BUILD ?? 'cmake-build-ui');
const vm = join(build, 'bin/rxvm');
const common = ['ui_web', 'ui_scene', 'ui_contract', 'ui_catalog', 'ui', 'library'];
const applications = {
  text: ['text_inspector_web', 'text_inspector'],
  graph: ['graph_web', 'scene_editors'],
  map: ['map_web', 'scene_editors'],
  probe: ['web_service_probe'],
};
const image = name => join(build, name.startsWith('text_') || name === 'web_service_probe' ?
  'examples/ui/text-inspector' : ['graph_web', 'map_web', 'scene_editors'].includes(name) ?
  'examples/ui/scene' : 'bin', `${name}.rxbin`);
const sessions = new Map();
const site = resolve(fileURLToPath(new URL('.', import.meta.url)));

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
function createSession(app) {
  if (!Object.hasOwn(applications, app)) throw new Error('Unknown UI application');
  if (sessions.size >= 16) throw new Error('Hosted UI session limit reached');
  const id = `hosted-${randomUUID()}`;
  const process = spawn(vm, [...applications[app], ...common].map(image).concat('-a', id),
    {stdio: ['pipe', 'pipe', 'pipe']});
  const session = {id, process, frames: [], sequence: 0, latest: null,
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
        session.frames.push({sequence: ++session.sequence, frame});
        if (session.frames.length > 256) session.frames.shift();
      } catch { process.kill(); }
    }
  });
  process.stderr.on('data', bytes => { session.diagnostic = String(bytes).slice(0, 1024); });
  process.on('error', error => { session.diagnostic = String(error); session.ended = true; });
  process.on('close', (code, signal) => {
    session.ended = true;
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

const contentTypes = {'.html': 'text/html', '.mjs': 'text/javascript',
  '.wasm': 'application/wasm', '.rxbin': 'application/octet-stream'};
const server = createServer(async (request, response) => {
  try {
    const url = new URL(request.url, 'http://localhost');
    if (!url.pathname.startsWith('/api/')) {
      const path = url.pathname === '/' ? 'index.html' : url.pathname.slice(1);
      if (!/^(index\.html|[a-z-]+\.mjs|vm\/crexx_ui_vm\.(mjs|wasm)|modules\/[a-z_]+\.rxbin)$/.test(path))
        return reply(response, 404, {error: 'Not found'});
      const bytes = await readFile(join(site, path));
      const ext = path.slice(path.lastIndexOf('.'));
      response.writeHead(200, {'Content-Type': contentTypes[ext], 'Cache-Control': 'no-store'});
      return response.end(bytes);
    }
    if (!authorized(request)) return reply(response, 401, {error: 'Unauthorized'});
    if (url.pathname === '/api/sessions' && request.method === 'POST') {
      const selection = await body(request);
      return reply(response, 201, {session: createSession(selection.app ?? 'text').id});
    }
    const match = /^\/api\/sessions\/([a-z0-9-]+)(\/input)?$/.exec(url.pathname);
    const session = match && sessions.get(match[1]);
    if (!session) return reply(response, 404, {error: 'Unknown session'});
    if (request.method === 'DELETE' && !match[2]) {
      sessions.delete(session.id);
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
    session.process.kill();
  }
}, 5000).unref();
function shutdown() {
  for (const session of sessions.values()) session.process.kill();
  server.close();
}
process.on('SIGINT', shutdown);
process.on('SIGTERM', shutdown);
