/* Browser effect to stateless HTTP API, with correlated failure handling. */
import assert from 'node:assert/strict';
import {ragQueryHttp} from './query-http.mjs';

const effect = {name: 'rag.query', request_id: 'local-test/1', payload: {
  corpus: 'heart-failure', mode: 'compare', hops: 2,
  question: 'What does NICE QS9 say?',
}};
const comparison_json = JSON.stringify({schema: 'crexx-rag.query-comparison/1',
  library_id: 'library-test', generation: 2308, model: 'bge-test',
  embedding_profile_id: 'embedding-test', graph_hops: 2,
  routes: ['keyword', 'embedding', 'graph'].map(route => ({route,
    candidate_count: 1, passages: []}))});
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
  assert.equal(request.hops, 2);
  return Response.json({version: 1, request_id: effect.request_id, comparison_json});
};
assert.equal(await handler(effect, complete), true);
assert.deepEqual(completions.shift(), {name: 'rag.query.completed', payload: {
  comparison_json,
}});
globalThis.fetch = async () => Response.json({version: 1,
  request_id: 'another-request', comparison_json});
await handler(effect, complete);
assert.deepEqual(completions.shift(), {name: 'ui.effect.failed', payload: {
  code: 'service_failed', message: 'Read-only query service unavailable',
}});
globalThis.fetch = async () => new Response('', {status: 503});
await handler(effect, complete);
assert.equal(completions.shift().name, 'ui.effect.failed');
globalThis.fetch = async () => Response.json({version: 1,
  request_id: effect.request_id, comparison_json: '{}'});
await handler(effect, complete);
assert.equal(completions.shift().name, 'ui.effect.failed');
console.log('PASS: browser RAG HTTP executor correlates comparison and failures');
