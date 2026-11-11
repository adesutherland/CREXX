/* Independent answer/source, progress, draft and cancellation owner expectations. */
import assert from 'node:assert/strict';
import {spawn} from 'node:child_process';
import {join, resolve} from 'node:path';
const build = resolve(process.argv[2] ?? 'cmake-build-ui');
const id = 'answer-contract';
const child = spawn(join(build, 'bin/rxvm'), [
  ...['rag_query_web_server', 'rag_query_app'].map(name => join(build, 'examples/ui/rag-query', `${name}.rxbin`)),
  ...['ui_web', 'ui_scene', 'ui_contract', 'ui_catalog', 'ui', 'library'].map(name => join(build, 'bin', `${name}.rxbin`)),
  '-a', id,
], {stdio: ['pipe', 'pipe', 'pipe']});
const queued = [];
let partial = '', diagnostics = '', wake;
child.stderr.on('data', data => { diagnostics += data; });
child.stdout.on('data', data => {
  partial += data;
  let end;
  while ((end = partial.indexOf('\n')) >= 0) {
    const frame = JSON.parse(partial.slice(0, end)); partial = partial.slice(end + 1);
    if (wake) { const deliver = wake; wake = null; deliver(frame); } else queued.push(frame);
  }
});
const next = () => queued.length ? Promise.resolve(queued.shift()) : new Promise((resolveNext, reject) => {
  const timer = setTimeout(() => reject(new Error(`Answer owner timed out: ${diagnostics}`)), 5000);
  wake = frame => { clearTimeout(timer); resolveNext(frame); };
});
const send = (type, base, body = {}) => child.stdin.write(JSON.stringify({version: 1, session: id, base, type, ...body}) + '\n');
const node = (frame, name) => frame.view.nodes.find(item => item.id === name);
const outcome = (request, name, payload) => ({version: 0, direction: 'event', name,
  target: request.target, session_id: request.session_id, request_id: request.request_id, sequence: 0, payload});
const question = 'What does the corpus say about treatment?\nExplain its limitations.';
const source = {title: 'Independent source', excerpt: 'Documented treatment evidence.',
  citation: 'crexx-rag:test-library:source:one', source_url: 'https://example.org/evidence.pdf', span_start: 20, span_end: 50};
const answer = {schema: 'crexx-rag.answer/1', status: 'answered', library_id: 'test-library', generation: 7,
  provider: 'fixture', model: 'fixture', question, answer: 'The corpus supports this bounded statement [1].', sources: [source]};
try {
  let view = await next();
  assert.equal(node(view, 'ask-question').kind, 'textarea');
  assert.equal(node(view, 'search-panel'), undefined, 'Raw search initially closed');
  assert.match(node(view, 'model-thorough').text, /selected/);
  send('command', view.revision, {command: 'rag.model.fast'}); view = await next();
  assert.match(node(view, 'model-fast').text, /selected/);
  send('command', view.revision, {command: 'rag.model.thorough'}); view = await next();
  send('value', view.revision, {node: 'ask-question', value: question, input_id: 'question-edit'});
  view = await next();
  assert.equal(view.input_id, 'question-edit'); assert.equal(view.accepted, 1);
  send('command', view.revision, {command: 'rag.ask'});
  view = await next();
  const request = (await next()).message;
  assert.equal(request.name, 'rag.answer'); assert.deepEqual(request.payload, {corpus: 'heart-failure', question, model_profile: 'thorough'});
  const oldRevision = view.revision;
  assert.ok(node(view, 'live-drawer')); assert.ok(node(view, 'stop'));
  const stage = {schema: 'rag-ui.stage/1', id: 'read-1', state: 'done', title: 'Source read',
    summary: 'Read Independent source', input: source.citation, output: source.excerpt};
  send('complete', oldRevision, {message: outcome(request, 'ui.effect.progress', {completed: 1, message: JSON.stringify(stage)})});
  view = await next();
  assert.match(node(view, 'stage-1').text, /Independent source/);
  send('command', view.revision, {command: 'rag.stage.one'}); view = await next();
  assert.match(node(view, 'stage-detail').value, /Documented treatment evidence/);
  send('complete', oldRevision, {message: outcome(request, 'rag.answer.completed', {answer_json: JSON.stringify(answer)})});
  view = await next();
  assert.match(node(view, 'answer-status').text, /Answer complete/);
  assert.match(node(view, 'answer-status').properties, /"tone":"answer"/);
  assert.equal(node(view, 'stop'), undefined);
  const answerView = JSON.parse(node(view, 'answer').value);
  assert.equal(answerView.blocks[0].runs.find(run => run.action)?.action, 'rag.source.one');
  send('complete', view.revision, {message: outcome(request, 'rag.answer.completed', {answer_json: JSON.stringify(answer)})});
  assert.equal((await next()).result.code, 'invalid_input');
  send('command', view.revision, {command: 'rag.source.one'}); view = await next();
  assert.equal(node(view, 'source-view').kind, 'group');
  assert.match(node(view, 'source-excerpt').value, /Documented treatment/);
  assert.equal(node(view, 'source-original').kind, 'link');
  send('command', view.revision, {command: 'rag.context'}); view = await next();
  const read = (await next()).message;
  assert.equal(read.name, 'rag.source'); assert.equal(read.payload.citation, source.citation);
  send('complete', view.revision, {message: outcome(read, 'rag.source.completed', {source_json: JSON.stringify({
    schema: 'crexx-rag.source-context/1', library_id: 'test-library', generation: 7,
    ...source, context_text: 'Wider documented context.', context_start: 0, context_end: 100,
    original_url: '/api/rag/v1/original/approved-asset',
  })})}); view = await next();
  assert.match(node(view, 'source-context-text').value, /Wider documented/);
  assert.equal(node(view, 'source-captured-original').value, '/api/rag/v1/original/approved-asset');
  send('command', view.revision, {command: 'rag.ask'}); view = await next();
  const cancelled = (await next()).message;
  send('cancel', view.revision, {effect: 'rag.answer'}); view = await next();
  assert.match(node(view, 'answer-status').text, /cancelled/);
  assert.match(node(view, 'answer').value, /bounded statement/);
  send('complete', view.revision, {message: outcome(cancelled, 'rag.answer.completed', {answer_json: JSON.stringify(answer)})});
  assert.equal((await next()).result.code, 'invalid_input');
  send('command', view.revision, {command: 'rag.ask'}); view = await next();
  const changed = (await next()).message;
  send('value', view.revision, {node: 'ask-question', value: 'A different question'}); view = await next();
  send('complete', oldRevision, {message: outcome(changed, 'rag.answer.completed', {answer_json: JSON.stringify(answer)})});
  view = await next();
  assert.match(node(view, 'answer-status').text, /question changed/i);
  assert.match(node(view, 'answer').value, /bounded statement/);
  send('command', view.revision - 1, {command: 'rag.ask', input_id: 'stale-command'}); view = await next();
  assert.equal(view.input_id, 'stale-command'); assert.equal(view.accepted, 0);
  send('command', view.revision, {command: 'rag.ask'}); view = await next();
  const failed = (await next()).message;
  const failureStage = {schema: 'rag-ui.stage/1', id: 'error', state: 'error', title: 'Request failed',
    summary: 'The answer service could not complete this request', input: 'A different question', output: 'RAG HTTP 503'};
  send('complete', view.revision, {message: outcome(failed, 'ui.effect.progress', {completed: 1, message: JSON.stringify(failureStage)})});
  view = await next();
  assert.match(node(view, 'stage-1').text, /Request failed/);
  send('complete', view.revision, {message: outcome(failed, 'ui.effect.failed', {code: 'service_failed', message: 'Answer service unavailable'})});
  view = await next();
  assert.match(node(view, 'answer-status').properties, /"tone":"error"/);
  send('command', view.revision, {command: 'rag.quit'});
  assert.equal((await next()).type, 'closed'); child.stdin.end();
  assert.equal(await new Promise(resolveExit => child.on('exit', resolveExit)), 0, diagnostics);
  console.log('PASS: answer/source ownership, real progress, stale completions, input acknowledgement and cancellation');
} finally { if (child.exitCode === null) child.kill(); }
