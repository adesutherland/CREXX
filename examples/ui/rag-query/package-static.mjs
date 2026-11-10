/* Assemble a local-review browser bundle from the RAG-owned serving export. */
import {cp, mkdir, readFile, readdir, writeFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {DatabaseSync} from 'node:sqlite';
import {join, resolve} from 'node:path';
import {fileURLToPath} from 'node:url';

if (process.argv.length !== 9) {
  console.error('Usage: node package-static.mjs UI_SITE RAG_WASM_BUILD RAG_RXBIN SERVING_DIR PROFILE_TSV CONFIG OUTPUT_DIR');
  process.exit(2);
}
const [site, wasm, reader, serving, profile, config, output] = process.argv.slice(2).map(path => resolve(path));
if (site === output || output.startsWith(`${site}/`)) throw new Error('Output must be separate from UI site');
let existing = [];
try { existing = await readdir(output); }
catch (error) { if (error.code !== 'ENOENT') throw error; }
if (existing.length) throw new Error('Output directory must be empty');
const vendor = fileURLToPath(new URL('./node_modules/@wllama/wllama/', import.meta.url));
const serve = JSON.parse(await readFile(join(serving, 'serving.json'), 'utf8'));
const corpus = JSON.parse(await readFile(join(serving, 'manifest.json'), 'utf8'));
if (serve.schema !== 'crexx-rag.serving-snapshot/1' ||
    corpus.library_id !== serve.library_id || corpus.published_generation !== serve.generation ||
    corpus.sidecars?.length !== 1 || corpus.sidecars[0].dimension !== 384)
  throw new Error('Serving identity or vector sidecar is invalid');
const db = new DatabaseSync(join(serving, 'library.sqlite'), {readOnly: true});
const snapshot = db.prepare('SELECT library_id,published_generation,active_profile_hash FROM library_meta WHERE singleton=1').get();
const embedding = db.prepare("SELECT p.embedding_profile_id,p.provider_id,p.model,p.dimension,p.input_envelope_fingerprint,v.sidecar_name,v.sidecar_checksum FROM embedding_profiles p JOIN vector_generations v ON v.embedding_profile_id=p.embedding_profile_id WHERE v.state='published' AND v.sidecar_name=?").get(corpus.sidecars[0].name);
db.close();
if (!snapshot || !embedding || snapshot.library_id !== serve.library_id ||
    snapshot.published_generation !== serve.generation ||
    snapshot.active_profile_hash !== corpus.profile_hash ||
    embedding.embedding_profile_id !== corpus.sidecars[0].embedding_profile_id ||
    embedding.dimension !== 384 || embedding.model !== 'bge-small-en-v1.5' ||
    embedding.sidecar_checksum !== corpus.sidecars[0].sha256)
  throw new Error('SQLite, profile, model or sidecar identities disagree');
const settings = await readFile(config, 'utf8');
function setting(name) {
  const matches = [...settings.matchAll(new RegExp(`^\\s*${name.replaceAll('.', '\\.') }\\s*=\\s*([^#\\r\\n]+)`, 'gm'))];
  if (matches.length !== 1) throw new Error(`Missing or repeated setting: ${name}`);
  return matches[0][1].trim();
}
function integer(name) {
  const raw = setting(name);
  if (!/^(0|[1-9][0-9]*)$/.test(raw)) throw new Error(`Invalid setting: ${name}`);
  return Number(raw);
}
const retrieval = {
  lexical_candidates: integer('retrieval.lexical_candidates'),
  vector_candidates: integer('retrieval.vector_candidates'),
  vector_scan_limit: integer('retrieval.vector_scan_limit'),
  claim_limit: integer('retrieval.claim_limit'),
  lead_limit: integer('retrieval.lead_limit'),
  rrf_k: integer('retrieval.rrf_k'),
  graph_direction: setting('retrieval.graph_direction'),
  graph_boost_millionths: integer('retrieval.graph_boost_millionths'),
  graph_decay_millionths: integer('retrieval.graph_decay_millionths'),
  directness_bonus_millionths: integer('retrieval.directness_bonus_millionths'),
  maximum_sidecar_bytes: integer('vector.maximum_sidecar_bytes'),
};
const files = [
  [join(serving, 'library.sqlite'), 'corpus/library.sqlite'],
  [join(serving, 'manifest.json'), 'corpus/manifest.json'],
  [join(serving, 'serving.json'), 'corpus/serving.json'],
  [join(serving, corpus.sidecars[0].name), `corpus/${corpus.sidecars[0].name}`],
  [join(serving, 'embedding.gguf'), 'corpus/embedding.gguf'],
  [profile, 'corpus/profile.tsv'],
  [join(wasm, 'crexx_rag_query_vm.mjs'), 'vm/crexx_rag_query_vm.mjs'],
  [join(wasm, 'crexx_rag_query_vm.wasm'), 'vm/crexx_rag_query_vm.wasm'],
  [reader, 'modules/crexxrag-wasm-reader.rxbin'],
  [join(vendor, 'esm/index.js'), 'apps/vendor/wllama/index.js'],
  [join(vendor, 'esm/wasm/wllama.wasm'), 'apps/vendor/wllama/wllama.wasm'],
  [join(vendor, 'LICENCE'), 'apps/vendor/wllama/LICENCE'],
  [new URL('./rag-embed-worker.mjs', import.meta.url), 'apps/rag-embed-worker.mjs'],
  [new URL('./rag-vm-worker.mjs', import.meta.url), 'apps/rag-vm-worker.mjs'],
  [new URL('./rag-bundle.mjs', import.meta.url), 'apps/rag-bundle.mjs'],
];
await cp(site, output, {recursive: true, force: true});
const uiManifest = JSON.parse(await readFile(join(output, 'manifest.json'), 'utf8'));
const assets = {};
for (const [source, target] of files) {
  const bytes = await readFile(source);
  if (!bytes.length) throw new Error(`Empty static asset: ${target}`);
  await mkdir(join(output, target, '..'), {recursive: true});
  await writeFile(join(output, target), bytes);
  const sha256 = createHash('sha256').update(bytes).digest('hex');
  assets[target] = {sha256, bytes: bytes.length};
  uiManifest.assets[target] = sha256;
}
if (assets['corpus/library.sqlite'].sha256 !== serve.sqlite_sha256 ||
    assets['corpus/embedding.gguf'].sha256 !== serve.model_sha256 ||
    assets[`corpus/${corpus.sidecars[0].name}`].sha256 !== corpus.sidecars[0].sha256)
  throw new Error('Export checksum does not match copied assets');
const descriptor = {
  schema: 'crexx-rag.browser-bundle/1',
  snapshot: {library_id: snapshot.library_id, generation: snapshot.published_generation,
    profile_hash: snapshot.active_profile_hash},
  embedding: {profile_id: embedding.embedding_profile_id, provider_id: embedding.provider_id,
    model: embedding.model, input_envelope_fingerprint: embedding.input_envelope_fingerprint,
    dimension: embedding.dimension, codec: 'f32le-v1'},
  retrieval, sidecar: corpus.sidecars[0].name, assets,
};
const descriptorBytes = Buffer.from(JSON.stringify(descriptor));
await writeFile(join(output, 'apps/rag-bundle.json'), descriptorBytes);
uiManifest.assets['apps/rag-bundle.json'] = createHash('sha256').update(descriptorBytes).digest('hex');
await writeFile(join(output, 'manifest.json'), `${JSON.stringify(uiManifest, null, 2)}\n`);
console.log(`Packed local RAG bundle (${Object.keys(assets).length} assets) in ${output}`);
