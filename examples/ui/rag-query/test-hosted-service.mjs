/* A deterministic RAG process checks the hosted comparison adapter boundary. */
import assert from 'node:assert/strict';
import {mkdtemp, writeFile, chmod, rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {run} from './hosted_service.mjs';

const scratch = await mkdtemp(join(tmpdir(), 'crexx-ui-rag-service-'));
const binary = join(scratch, 'rag-fixture');
await writeFile(binary, `#!/usr/bin/env node
const question = process.argv[process.argv.indexOf('compare') + 1];
const hops = Number(process.argv[process.argv.indexOf('--hops') + 1]);
const publish = () => {
  const passage = {rank: 1, title: 'Source document', excerpt: 'Cited source text',
    source_url: question === 'bad URL' ? 'javascript:alert(1)' : 'https://example.org/source.pdf',
    citation: question === 'bad citation' ? 'wrong:source' : 'crexx-rag:library-test:source',
    span_start: 42, span_end: 59};
  const comparison = {schema: 'crexx-rag.query-comparison/1', library_id: 'library-test',
    generation: 2, embedding_profile_id: 'embedding-test', model: 'bge-test',
    graph_hops: hops, routes: ['keyword', 'embedding', 'graph'].map(route =>
      ({route, candidate_count: 1, passages: [passage],
        ...(route === 'graph' ? {graph: {anchor_count: 1, anchors: ['concept-1'],
          direction: 'both', claim_count: 1, claims: [{claim_id: 'claim-1', hop: 1,
            subject: 'Heart failure', relationship: 'treated by', object: 'Therapy',
            stored_direction: 'forward', path: {claim_ids:
              question === 'bad graph' ? [] : ['claim-1']}}]}} : {})}))};
  const fields = {provider_calls: question === 'provider' ? 0 : 1,
    query_gaps_recorded: 0, comparison_json: JSON.stringify(comparison)};
  console.log(JSON.stringify({operation: 'query.compare', status: 'ok', exit_code: 0,
    records: [{fields}]}));
};
if (question === 'slow') setTimeout(publish, 1000);
else publish();
`);
await chmod(binary, 0o755);
Object.assign(process.env, {CREXX_UI_RAG_CORPUS: 'tutorial', CREXX_UI_RAG_BINARY: binary,
  CREXX_UI_RAG_LIBRARY: join(scratch, 'library'), CREXX_UI_RAG_CONFIG: join(scratch, 'config')});
const request = question => ({name: 'rag.query', payload: {
  corpus: 'tutorial', mode: 'compare', question, hops: 2,
}});
try {
  const result = await run(request('valid'));
  assert.equal(result.name, 'rag.query.completed');
  const comparison = JSON.parse(result.payload.comparison_json);
  assert.deepEqual(comparison.routes.map(route => route.route),
    ['keyword', 'embedding', 'graph']);
  assert.equal(comparison.graph_hops, 2);
  await assert.rejects(run({...request('valid'), payload: {
    ...request('valid').payload, corpus: 'other',
  }}), /Invalid read-only RAG query/);
  await assert.rejects(run(request('provider')), /read-only query contract failed/);
  await assert.rejects(run(request('bad citation')), /cited passage is invalid/);
  await assert.rejects(run(request('bad URL')), /cited passage is invalid/);
  await assert.rejects(run(request('bad graph')), /graph path is invalid/);
  const abort = new AbortController();
  const slow = run(request('slow'), {signal: abort.signal});
  abort.abort();
  await assert.rejects(slow, /AbortError|aborted/i);
} finally { await rm(scratch, {recursive: true, force: true}); }
console.log('PASS: hosted RAG adapter validates three routes, citations and abort');
