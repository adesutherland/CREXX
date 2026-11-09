/* A deterministic RAG process checks the hosted adapter's trust boundary. */
import assert from 'node:assert/strict';
import {mkdtemp, writeFile, chmod, rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {run} from './hosted_service.mjs';

const scratch = await mkdtemp(join(tmpdir(), 'crexx-ui-rag-service-'));
const binary = join(scratch, 'rag-fixture');
await writeFile(binary, `#!/usr/bin/env node
const question = process.argv[process.argv.indexOf('inspect') + 1];
const publish = () => {
  const evidence = {library_id: 'library-test', active_generation: 2,
    passages: [{citation: {id: question === 'bad citation' ? 'wrong-library:source' :
      'crexx-rag:library-test:source'}, text: 'Cited source text'}]};
  const fields = {provider_calls: question === 'provider' ? 1 : 0,
    query_gaps_recorded: 0, evidence_json: JSON.stringify(evidence)};
  console.log(JSON.stringify({operation: 'query.inspect', status: 'ok', exit_code: 0,
    records: [{fields}]}));
};
if (question === 'slow') setTimeout(publish, 1000);
else publish();
`);
await chmod(binary, 0o755);
Object.assign(process.env, {CREXX_UI_RAG_CORPUS: 'tutorial', CREXX_UI_RAG_BINARY: binary,
  CREXX_UI_RAG_LIBRARY: join(scratch, 'library'), CREXX_UI_RAG_CONFIG: join(scratch, 'config')});
const request = question => ({name: 'rag.query', payload: {
  corpus: 'tutorial', mode: 'lexical', question,
}});
try {
  const result = await run(request('valid'));
  assert.deepEqual(result, {name: 'rag.query.completed', payload: {
    library_id: 'library-test', generation: 2,
    citation: 'crexx-rag:library-test:source', excerpt: 'Cited source text',
  }});
  await assert.rejects(run({...request('valid'), payload: {
    ...request('valid').payload, corpus: 'other',
  }}), /Invalid read-only RAG query/);
  await assert.rejects(run(request('provider')), /read-only query contract failed/);
  await assert.rejects(run(request('bad citation')), /cited result is invalid/);
  const abort = new AbortController();
  const slow = run(request('slow'), {signal: abort.signal});
  abort.abort();
  await assert.rejects(slow, /AbortError|aborted/i);
} finally { await rm(scratch, {recursive: true, force: true}); }
console.log('PASS: hosted RAG adapter validates citation, provider isolation and abort');
