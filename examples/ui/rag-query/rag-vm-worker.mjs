/* One query VM owns a transient read-only corpus and runs RAG's Level G core. */
import {asset, loadBundle} from './rag-bundle.mjs';

self.onmessage = async ({data}) => {
  let vm;
  const errors = [];
  try {
    if (data?.type !== 'query' || !data.effect ||
        typeof data.effect.request_id !== 'string' ||
        !/^[A-Za-z0-9/_-]{1,128}$/.test(data.effect.request_id) ||
        !(data.vector instanceof ArrayBuffer) || data.vector.byteLength !== 384 * 4 ||
        data.effect.payload?.mode !== 'compare' ||
        !Number.isInteger(data.effect.payload?.hops) ||
        data.effect.payload.hops < 0 || data.effect.payload.hops > 4)
      throw new Error('Invalid RAG query start');
    const bundle = await loadBundle();
    const paths = [
      'corpus/library.sqlite', 'corpus/manifest.json', 'corpus/serving.json',
      `corpus/${bundle.sidecar}`, 'corpus/embedding.gguf', 'corpus/profile.tsv',
      'modules/crexxrag-wasm-reader.rxbin',
      'vm/crexx_rag_query_vm.mjs', 'vm/crexx_rag_query_vm.wasm',
    ];
    const loaded = await Promise.all(paths.map(async path => [path, await asset(bundle, path)]));
    const files = new Map(loaded);
    const {default: createVM} = await import('../vm/crexx_rag_query_vm.mjs');
    const output = [];
    vm = await createVM({noInitialRun: true,
      locateFile: name => new URL(`../vm/${name}`, import.meta.url).href,
      print: line => output.push(line), printErr: line => errors.push(line)});
    vm.FS.mkdir('/library');
    for (const path of paths.filter(path => path.startsWith('corpus/')))
      vm.FS.writeFile(`/library/${path.slice(7)}`, new Uint8Array(files.get(path)));
    vm.FS.writeFile('/profile.tsv', new Uint8Array(files.get('corpus/profile.tsv')));
    vm.FS.writeFile('/query-vector.f32', new Uint8Array(data.vector));
    if (vm._rag_vm_prepare_fts() !== 0)
      throw new Error(vm.ccall('rag_vm_error', 'string', [], []));
    if (vm._rag_vm_create() !== 0) throw new Error(vm.ccall('rag_vm_error', 'string', [], []));
    const reader = new Uint8Array(files.get('modules/crexxrag-wasm-reader.rxbin'));
    const pointer = vm._malloc(reader.byteLength);
    if (!pointer) throw new Error('Cannot allocate RAG RXBIN');
    try {
      vm.HEAPU8.set(reader, pointer);
      if (vm._rag_vm_load(pointer, reader.byteLength) !== 0)
        throw new Error(vm.ccall('rag_vm_error', 'string', [], []));
    } finally { vm._free(pointer); }
    const request = {
      schema: 'crexx-rag.wasm-query/1', request_id: data.effect.request_id,
      question: data.effect.payload.question, hops: data.effect.payload.hops, limit: 3,
      snapshot: bundle.snapshot,
      embedding: {...bundle.embedding, vector_file: '/query-vector.f32'},
      retrieval: bundle.retrieval,
    };
    const status = vm.ccall('rag_vm_run', 'number', ['string'], [JSON.stringify(request)]);
    if (status !== 0 || vm._rag_vm_program_rc() !== 0)
      throw new Error(output.join('\n') || errors.join('\n') ||
        vm.ccall('rag_vm_error', 'string', [], []));
    if (output.length !== 1) throw new Error('Unexpected RAG VM output');
    const result = JSON.parse(output[0]);
    if (result.provider_calls !== 0 || result.query_gaps_recorded !== 0)
      throw new Error('RAG reader performed a write or provider call');
    self.postMessage({type: 'result', result});
  } catch (error) {
    self.postMessage({type: 'failure', message: `${error?.stack || error}\n${errors.slice(-8).join(' | ')}`});
  } finally {
    try { vm?._rag_vm_destroy(); } catch {}
    self.close();
  }
};
