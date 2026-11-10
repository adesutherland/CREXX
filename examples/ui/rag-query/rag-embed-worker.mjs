/* CPU Wasm makes one BGE query vector; the RAG VM owns retrieval. */
import {asset, loadBundle} from './rag-bundle.mjs';

self.onmessage = async ({data}) => {
  try {
    if (data?.type !== 'embed' || typeof data.question !== 'string' ||
        !data.question.trim() || data.question.length > 256 || /[\r\n\0]/.test(data.question))
      throw new Error('Invalid BGE question');
    const bundle = await loadBundle();
    await Promise.all([
      asset(bundle, 'apps/vendor/wllama/index.js'),
      asset(bundle, 'apps/vendor/wllama/wllama.wasm'),
    ]);
    const model = await asset(bundle, 'corpus/embedding.gguf');
    const {Wllama} = await import('./vendor/wllama/index.js');
    const engine = new Wllama({default: new URL('./vendor/wllama/wllama.wasm', import.meta.url).href});
    await engine.loadModel([new Blob([model])], {
      embeddings: true, pooling_type: 'LLAMA_POOLING_TYPE_CLS',
      n_ctx: 512, n_batch: 512, n_ubatch: 512, n_gpu_layers: 0,
    });
    const result = await engine.createEmbedding({
      input: `Represent this sentence for searching relevant passages: ${data.question}`,
      embd_normalize: 2,
    });
    const vector = result.data?.[0]?.embedding;
    if (!Array.isArray(vector) || vector.length !== 384 ||
        vector.some(value => !Number.isFinite(value)))
      throw new Error('Invalid BGE vector');
    const bytes = new ArrayBuffer(384 * 4);
    const view = new DataView(bytes);
    vector.forEach((value, index) => view.setFloat32(index * 4, value, true));
    self.postMessage({type: 'result', result: bytes}, [bytes]);
  } catch (error) {
    self.postMessage({type: 'failure', message: error?.stack || String(error)});
  } finally { self.close(); }
};
