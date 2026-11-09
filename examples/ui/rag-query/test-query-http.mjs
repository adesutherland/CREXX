/* Browser effect to stateless HTTP API, with correlated failure handling. */
import assert from 'node:assert/strict';
import {ragQueryHttp} from './query-http.mjs';

const effect = {name: 'rag.query', request_id: 'local-test/1', payload: {
  corpus: 'heart-failure', mode: 'lexical', question: 'What does NICE QS9 say?',
}};
const completions = [];
const complete = async (_effect, name, payload) => completions.push({name, payload});
const handler = ragQueryHttp();
assert.equal(await handler({name: 'other.effect'}, complete), false);
globalThis.fetch = async (path, options) => {
  assert.equal(path, '/api/rag/v1/query');
  assert.equal(options.method, 'POST');
  assert.equal(options.headers['Content-Type'], 'application/json');
  const request = JSON.parse(options.body);
  assert.equal(request.version, 1);
  assert.equal(request.request_id, effect.request_id);
  assert.equal(request.corpus, 'heart-failure');
  return Response.json({version: 1, request_id: effect.request_id,
    library_id: 'library-test', generation: 2308,
    citation: 'crexx-rag:library-test:source', excerpt: 'Cited source text'});
};
assert.equal(await handler(effect, complete), true);
assert.deepEqual(completions.shift(), {name: 'rag.query.completed', payload: {
  library_id: 'library-test', generation: 2308,
  citation: 'crexx-rag:library-test:source', excerpt: 'Cited source text',
}});
globalThis.fetch = async () => Response.json({version: 1,
  request_id: 'another-request', library_id: 'library-test', generation: 2308,
  citation: 'crexx-rag:library-test:source', excerpt: 'Cited source text'});
await handler(effect, complete);
assert.deepEqual(completions.shift(), {name: 'ui.effect.failed', payload: {
  code: 'service_failed', message: 'Read-only query service unavailable',
}});
globalThis.fetch = async () => new Response('', {status: 503});
await handler(effect, complete);
assert.equal(completions.shift().name, 'ui.effect.failed');
console.log('PASS: browser RAG HTTP executor correlates cited results and failures');
