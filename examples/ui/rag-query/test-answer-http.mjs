/* Real-stage HTTP orchestration and final answer/source contract validation. */
import assert from 'node:assert/strict';
import {ragQueryHttp, validateAnswer, validateSource} from './query-http.mjs';
const question = 'Explain the evidence.\nState its limits.';
const effect = {name: 'rag.answer', request_id: 'answer-http/1', payload: {corpus: 'heart-failure', question, model_profile: 'thorough'}};
const source = {title: 'Source title', excerpt: 'Captured supporting text.',
  citation: 'crexx-rag:test-library:source', source_url: 'https://example.org/source.pdf', span_start: 20, span_end: 45};
const answer = {schema: 'crexx-rag.answer/1', version: 1, request_id: effect.request_id,
  library_id: 'test-library', generation: 7, provider: 'fixture', model: 'fixture', question,
  status: 'answered', answer: 'A supported finding with its evidence listed separately.', sources: [source]};
const context = {schema: 'crexx-rag.source-context/1', version: 1, library_id: 'test-library', generation: 7,
  ...source, context_text: 'Wider source context with supporting text.', context_start: 0, context_end: 60,
  original_url: '/api/rag/v1/original/approved-asset'};
const prepared = {schema: 'crexx-rag.answer-preparation/1', version: 1, request_id: effect.request_id,
  corpus: 'heart-failure', question, library_id: 'test-library', generation: 7, status: 'ready',
  activities: [{stage: 'search', message: 'Initial search completed.', route: 'keyword', candidate_count: 1, sources: [source]},
    {stage: 'scope', message: 'Corpus scope assessed.', route: 'scope', candidate_count: 1, sources: []}],
  research: {searches: ['treatment evidence'], reads: [source.citation]}};
const handler = ragQueryHttp();
const completed = [], calls = [];
const complete = async (_request, name, payload) => completed.push({name, payload});
globalThis.fetch = async (path, options) => {
  const input = JSON.parse(options.body); calls.push({path, input});
  if (path.endsWith('/prepare')) return Response.json(prepared);
  if (path.endsWith('/source')) return Response.json({...context, request_id: input.request_id});
  assert.deepEqual(input.prepared, {library_id: 'test-library', generation: 7,
    searches: ['treatment evidence'], reads: [source.citation]});
  return Response.json(answer);
};
await handler(effect, complete);
assert.deepEqual(calls.map(call => call.path), ['/api/rag/v1/answer/prepare', '/api/rag/v1/source', '/api/rag/v1/answer']);
assert.ok(completed.filter(item => item.name === 'ui.effect.progress').length >= 5);
assert.ok(completed.some(item => item.name === 'ui.effect.progress' &&
  JSON.parse(item.payload.message).summary === 'Read Source title'));
assert.equal(calls[0].input.model_profile, 'thorough');
assert.equal(calls[2].input.model_profile, 'thorough');
assert.equal(completed.at(-1).name, 'rag.answer.completed');
assert.equal(JSON.parse(completed.at(-1).payload.answer_json).answer, answer.answer,
  'Validated sources support marker-free answer prose');
assert.equal(JSON.parse(completed.at(-1).payload.answer_json).sources[0].citation, source.citation);
assert.throws(() => validateAnswer({...answer, answer: 'Unknown reference [2].'}, effect.payload));
assert.throws(() => validateAnswer({...answer, sources: []}, effect.payload));
assert.throws(() => validateAnswer({...answer, sources: [{...source, source_url: 'file:///private/document'}]}, effect.payload));
assert.throws(() => validateAnswer({...answer, question: 'Another question'}, effect.payload));
assert.throws(() => validateSource({...context, original_url: '//evil.example/file'}, source.citation));
assert.throws(() => validateSource({...context, context_end: 21}, source.citation));
completed.length = 0;
let paidCalls = 0;
globalThis.fetch = async (path, options) => {
  if (path.endsWith('/prepare')) return Response.json(prepared);
  if (path.endsWith('/source')) return Response.json({...context, request_id: JSON.parse(options.body).request_id});
  paidCalls++; return new Response('', {status: 503});
};
await handler(effect, complete);
assert.equal(paidCalls, 1);
assert.ok(completed.some(item => item.name === 'ui.effect.progress' &&
  JSON.parse(item.payload.message).state === 'error'), 'Failure is a distinct red research stage');
assert.equal(completed.at(-1).name, 'ui.effect.failed');
completed.length = 0;
const controller = new AbortController();
globalThis.fetch = async (_path, {signal}) => new Promise((resolve, reject) => {
  if (signal.aborted) { reject(new DOMException('Stopped', 'AbortError')); return; }
  signal.addEventListener('abort', () => reject(new DOMException('Stopped', 'AbortError')), {once: true});
});
const running = handler(effect, complete, {signal: controller.signal});
controller.abort(); await running;
assert.equal(completed.some(item => item.name === 'rag.answer.completed' || item.name === 'ui.effect.failed'), false,
  'Physical abort must not publish a late completion or failure');
console.log('PASS: actual prepare/read progress, final citations, explicit retry boundary and physical cancellation');
