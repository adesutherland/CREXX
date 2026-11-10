/* Focused owner-loop proof for local and hosted RAG effects and lifecycle. */
import assert from 'node:assert/strict';
import {spawn} from 'node:child_process';
import {join, resolve} from 'node:path';

const build = resolve(process.argv[2] ?? 'cmake-build-ui');
const modules = ['ui_web', 'ui_scene', 'ui_contract', 'ui_catalog', 'ui', 'library'];
const comparisonJson = (generation, excerpt) => JSON.stringify({
  schema: 'crexx-rag.query-comparison/1', library_id: 'library-test', generation,
  embedding_profile_id: 'embedding-test', model: 'bge-test', graph_hops: 2,
  routes: ['keyword', 'embedding', 'graph'].map(route => ({route,
    candidate_count: 1, passages: [{rank: 1, title: 'Source document',
      excerpt, source_url: 'https://example.org/source.pdf',
      citation: 'crexx-rag:library-test:source', span_start: 42, span_end: 59}],
    ...(route === 'graph' ? {graph: {anchor_count: 1, anchors: ['concept-1'],
      direction: 'both', claim_count: 1, claims: [{claim_id: 'claim-1', hop: 1,
        subject: 'Heart failure', relationship: 'treated by', object: 'Therapy',
        stored_direction: 'forward', path: {claim_ids: ['claim-1']}}]}} : {})})),
});

async function route(kind) {
  const id = `rag-${kind}`;
  const images = [`rag_query_web_${kind}`, 'rag_query_app']
    .map(name => join(build, 'examples/ui/rag-query', `${name}.rxbin`))
    .concat(modules.map(name => join(build, 'bin', `${name}.rxbin`)));
  const child = spawn(join(build, 'bin/rxvm'), [...images, '-a', id],
    {stdio: ['pipe', 'pipe', 'pipe']});
  const frames = [];
  let pending = '';
  let wake;
  let diagnostics = '';
  child.stderr.on('data', data => { diagnostics += String(data); });
  child.stdout.on('data', data => {
    pending += String(data);
    let end;
    while ((end = pending.indexOf('\n')) >= 0) {
      const frame = JSON.parse(pending.slice(0, end));
      pending = pending.slice(end + 1);
      if (wake) { const finish = wake; wake = null; finish(frame); }
      else frames.push(frame);
    }
  });
  const next = () => frames.length ? Promise.resolve(frames.shift()) :
    new Promise((resolveNext, reject) => {
      const timer = setTimeout(() => reject(new Error(`Timed out on ${kind}: ${diagnostics}`)), 5000);
      wake = frame => { clearTimeout(timer); resolveNext(frame); };
    });
  const send = (type, base, fields = {}) => child.stdin.write(JSON.stringify({
    version: 1, session: id, base, type, ...fields,
  }) + '\n');
  const label = (view, node) => view.view.nodes.find(item => item.id === node)?.text;
  const completion = (request, name, payload) => ({version: 0, direction: 'event',
    name, target: request.target, payload, request_id: request.request_id,
    session_id: request.session_id, sequence: 0});
  try {
    let view = await next();
    assert.equal(view.type, 'view');
    assert.match(label(view, 'status'), /Ready/);
    if (kind === 'local') {
      send('command', view.revision, {command: 'rag.search'});
      view = await next();
      assert.equal(view.type, 'view');
      const effect = (await next()).message;
      assert.equal(effect.name, 'rag.query');
      assert.equal(effect.payload.corpus, 'heart-failure');
      assert.equal(effect.payload.mode, 'compare');
      assert.equal(effect.payload.hops, 2);
      assert.equal(effect.payload.question, 'heart failure treatment pathway');
      send('complete', view.revision, {message: completion(effect, 'rag.query.completed', {
        comparison_json: comparisonJson(2308, 'Cited source text'),
      })});
      view = await next();
      assert.equal(label(view, 'status'), 'Three read-only routes compared');
      assert.match(label(view, 'identity'), /Generation: 2308/);
      assert.match(label(view, 'keyword-result'), /Cited source text/);
      assert.match(label(view, 'graph-result'), /https:\/\/example.org/);
      assert.match(label(view, 'graph-result'), /Heart failure.*hop 1/);
    } else {
      send('value', view.revision, {node: 'question', value: '   '});
      view = await next();
      assert.match(label(view, 'status'), /1 to 256/);
      send('command', view.revision, {command: 'rag.search'});
      view = await next();
      let effect = (await next()).message;
      assert.equal(effect.name, 'rag.query');
      assert.deepEqual(effect.payload, {corpus: 'heart-failure',
        question: 'heart failure treatment pathway', mode: 'compare', hops: 2});
      send('cancel', view.revision, {request_id: effect.request_id});
      view = await next();
      assert.equal(label(view, 'status'), 'Query cancelled');
      send('complete', view.revision, {message: completion(effect, 'rag.query.completed', {
        comparison_json: comparisonJson(1, 'stale'),
      })});
      assert.equal((await next()).result.code, 'invalid_input');
      send('command', view.revision, {command: 'rag.search'});
      view = await next();
      effect = (await next()).message;
      const success = completion(effect, 'rag.query.completed', {
        comparison_json: comparisonJson(2, 'BillingService depends on CustomerDatabase.'),
      });
      send('complete', view.revision, {message: success});
      view = await next();
      assert.equal(label(view, 'status'), 'Three read-only routes compared');
      assert.match(label(view, 'identity'), /Library: library-test · Generation: 2/);
      assert.match(label(view, 'keyword-result'), /BillingService depends on CustomerDatabase/);
      send('complete', view.revision, {message: success});
      assert.equal((await next()).result.code, 'invalid_input');
      send('command', view.revision, {command: 'rag.search'});
      view = await next();
      effect = (await next()).message;
      send('value', view.revision, {node: 'hops', value: '3'});
      view = await next();
      assert.equal(label(view, 'graph-result'), '');
      send('complete', view.revision, {message: completion(effect, 'rag.query.completed', {
        comparison_json: comparisonJson(3, 'old input'),
      })});
      view = await next();
      assert.match(label(view, 'status'), /changed; search again/);
      assert.equal(label(view, 'keyword-result'), '');
      send('command', view.revision, {command: 'rag.search'});
      view = await next();
      effect = (await next()).message;
      send('complete', view.revision, {message: completion(effect, 'ui.effect.failed',
        {code: 'service_failed', message: 'Application service failed'})});
      view = await next();
      assert.equal(label(view, 'status'), 'Query failed: service_failed');
    }
    send('command', view.revision, {command: 'rag.quit'});
    assert.equal((await next()).type, 'closed');
    child.stdin.end();
    assert.equal(await new Promise(resolveExit => child.on('exit', resolveExit)), 0, diagnostics);
  } finally { if (child.exitCode === null) child.kill(); }
}

await route('hosted');
await route('local');
console.log('PASS: local and hosted RAG query effects, citation, cancellation and failure');
