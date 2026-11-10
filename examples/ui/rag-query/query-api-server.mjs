/* Loopback development host for a stateless, same-origin RAG query API. */
import {createServer} from 'node:http';
import {readFile} from 'node:fs/promises';
import {extname, join, resolve} from 'node:path';
import {Readable} from 'node:stream';
import {pipeline} from 'node:stream/promises';
import {run} from './hosted_service.mjs';

if (!process.argv[2]) throw new Error('Pass the packaged browser site directory');
const site = resolve(process.argv[2]);
const port = Number(process.env.CREXX_UI_PORT ?? 8788);
if (!Number.isSafeInteger(port) || port < 1 || port > 65535)
  throw new Error('Invalid UI API port');
const origin = new URL(process.env.CREXX_UI_PUBLIC_ORIGIN ?? `http://127.0.0.1:${port}`);
const manifest = JSON.parse(await readFile(join(site, 'manifest.json'), 'utf8'));
const publicAssets = new Set(Object.keys(manifest.assets ?? {})
  .filter(path => !path.startsWith('services/')));
publicAssets.add('manifest.json');
const contentTypes = {'.html': 'text/html', '.mjs': 'text/javascript', '.css': 'text/css',
  '.json': 'application/json', '.wasm': 'application/wasm',
  '.rxbin': 'application/octet-stream'};
let inFlight = 0;

function json(response, status, value) {
  response.writeHead(status, {'Content-Type': 'application/json',
    'Cache-Control': 'no-store'});
  response.end(JSON.stringify(value));
}

async function parseBody(request) {
  if (!/^application\/json(?:\s*;|$)/i.test(request.headers['content-type'] ?? ''))
    throw new Error('Expected JSON request');
  const chunks = [];
  let size = 0;
  for await (const part of request) {
    size += part.length;
    if (size > 8192) throw new Error('Query request exceeds limit');
    chunks.push(part);
  }
  const value = JSON.parse(Buffer.concat(chunks).toString('utf8'));
  if (value?.version !== 1 || typeof value.request_id !== 'string' ||
      !/^[A-Za-z0-9/_-]{1,128}$/.test(value.request_id))
    throw new Error('Invalid query request envelope');
  return value;
}

const server = createServer(async (request, response) => {
  try {
    const url = new URL(request.url, origin);
    const answerRoute = ['/api/rag/v1/answer/prepare', '/api/rag/v1/answer', '/api/rag/v1/source'].includes(url.pathname);
    const originalRoute = /^\/api\/rag\/v1\/original\/[A-Za-z0-9_-]+$/.test(url.pathname);
    if (answerRoute || originalRoute) {
      if (request.headers.host !== origin.host ||
          (request.headers.origin && request.headers.origin !== origin.origin))
        return json(response, 403, {error: 'Origin refused'});
      if ((answerRoute && request.method !== 'POST') ||
          (originalRoute && !['GET', 'HEAD'].includes(request.method)))
        return json(response, 405, {error: 'Method not allowed'});
      if (inFlight >= 8) return json(response, 503, {error: 'Service capacity reached'});
      const configured = process.env.CREXX_UI_RAG_HTTP_ORIGIN;
      if (!configured) return json(response, 503, {error: 'Server answer service is not configured'});
      const upstream = new URL(configured);
      if (!['http:', 'https:'].includes(upstream.protocol) || upstream.username || upstream.password)
        throw new Error('Invalid backend origin');
      const input = answerRoute ? await parseBody(request) : null;
      const controller = new AbortController();
      const timer = setTimeout(() => controller.abort(), 120000);
      response.once('close', () => { if (!response.writableEnded) controller.abort(); });
      inFlight++;
      try {
        const remote = await fetch(new URL(url.pathname, upstream), {
          method: request.method, signal: controller.signal,
          ...(input ? {headers: {'Content-Type': 'application/json'}, body: JSON.stringify(input)} : {}),
        });
        response.writeHead(remote.status, {
          'Content-Type': remote.headers.get('content-type') ?? 'application/octet-stream',
          'Cache-Control': 'no-store', 'X-Content-Type-Options': 'nosniff',
          ...(originalRoute ? {'Content-Disposition': remote.headers.get('content-disposition') ?? 'inline'} : {}),
        });
        if (request.method === 'HEAD' || !remote.body) response.end();
        else await pipeline(Readable.fromWeb(remote.body), response);
      } catch {
        if (!controller.signal.aborted && !response.headersSent)
          json(response, 502, {error: 'Server answer service unavailable'});
        else response.destroy();
      } finally { clearTimeout(timer); inFlight--; }
      return;
    }
    if (url.pathname === '/api/rag/v1/query') {
      if (request.method !== 'POST') return json(response, 405, {error: 'Method not allowed'});
      if (request.headers.host !== origin.host ||
          (request.headers.origin && request.headers.origin !== origin.origin))
        return json(response, 403, {error: 'Origin refused'});
      if (inFlight >= 8) return json(response, 503, {error: 'Query capacity reached'});
      const input = await parseBody(request);
      const controller = new AbortController();
      response.once('close', () => { if (!response.writableEnded) controller.abort(); });
      inFlight++;
      try {
        const result = await run({name: 'rag.query', payload: {
          corpus: input.corpus, question: input.question, mode: input.mode,
          hops: input.hops,
        }}, {signal: controller.signal});
        if (result.name !== 'rag.query.completed')
          throw new Error('Unexpected query result');
        if (!controller.signal.aborted)
          json(response, 200, {version: 1, request_id: input.request_id, ...result.payload});
      } catch {
        if (!controller.signal.aborted)
          json(response, 502, {error: 'Read-only query failed'});
      } finally { inFlight--; }
      return;
    }
    if (request.method !== 'GET' && request.method !== 'HEAD')
      return json(response, 405, {error: 'Method not allowed'});
    const path = url.pathname === '/' ? 'index.html' : url.pathname.slice(1);
    if (!publicAssets.has(path) || path.includes('..'))
      return json(response, 404, {error: 'Not found'});
    const bytes = await readFile(join(site, path));
    response.writeHead(200, {'Content-Type': contentTypes[extname(path)] ??
      'application/octet-stream', 'Cache-Control': 'no-store'});
    response.end(request.method === 'HEAD' ? undefined : bytes);
  } catch {
    if (!response.writableEnded) json(response, 400, {error: 'Invalid request'});
  }
});

server.listen(port, '127.0.0.1');
process.on('SIGINT', () => server.close());
process.on('SIGTERM', () => server.close());
