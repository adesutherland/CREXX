/* Application executor: two disposable Workers serve one UI effect at a time. */
function runWorker(url, payload, signal, timeoutMs, transfers = []) {
  return new Promise((resolve, reject) => {
    const worker = new Worker(url, {type: 'module'});
    let settled = false;
    function finish(error, result) {
      if (settled) return;
      settled = true;
      clearTimeout(timer);
      signal.removeEventListener('abort', cancelled);
      worker.terminate();
      if (error) reject(error); else resolve(result);
    }
    const cancelled = () => finish(new Error('RAG query cancelled'));
    const timer = setTimeout(() => finish(new Error('RAG query timed out')), timeoutMs);
    signal.addEventListener('abort', cancelled, {once: true});
    worker.onmessage = ({data}) => {
      if (data?.type === 'result') finish(null, data.result);
      else if (data?.type === 'failure') finish(new Error(`${url}: ${data.message || 'RAG Worker failed'}`));
      else finish(new Error('Invalid RAG Worker reply'));
    };
    worker.onerror = event => finish(new Error(event.message || 'RAG Worker error'));
    if (signal.aborted) cancelled();
    else worker.postMessage(payload, transfers);
  });
}

export function ragQueryLocal() {
  let active;
  return async (effect, complete) => {
    if (effect.name !== 'rag.query') return false;
    active?.abort();
    const controller = new AbortController();
    active = controller;
    try {
      const vector = await runWorker(new URL('./rag-embed-worker.mjs', import.meta.url),
        {type: 'embed', question: effect.payload.question}, controller.signal, 120000);
      if (!(vector instanceof ArrayBuffer) || vector.byteLength !== 384 * 4)
        throw new Error('Invalid BGE vector');
      const result = await runWorker(new URL('./rag-vm-worker.mjs', import.meta.url),
        {type: 'query', effect, vector}, controller.signal, 120000, [vector]);
      if (controller.signal.aborted) return true;
      if (result?.version !== 1 || result.request_id !== effect.request_id ||
          typeof result.comparison_json !== 'string' || result.comparison_json.length > 16384)
        throw new Error('Invalid RAG comparison reply');
      const comparison = JSON.parse(result.comparison_json);
      if (comparison.schema !== 'crexx-rag.query-comparison/1' ||
          comparison.graph_hops !== effect.payload.hops ||
          !Array.isArray(comparison.routes) || comparison.routes.length !== 3 ||
          comparison.routes.some((route, index) =>
            route?.route !== ['keyword', 'embedding', 'graph'][index]))
        throw new Error('Invalid RAG comparison routes');
      await complete(effect, 'rag.query.completed', {comparison_json: result.comparison_json});
    } catch (error) {
      console.error('Browser corpus query failed', error);
      if (!controller.signal.aborted)
        await complete(effect, 'ui.effect.failed',
          {code: 'service_failed', message: 'Browser corpus reader unavailable'});
    } finally {
      if (active === controller) active = null;
    }
    return true;
  };
}
