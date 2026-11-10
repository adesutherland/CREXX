/* Application-owned HTTP executor; the cREXX session still owns UI state. */
export function ragQueryHttp() {
  return async (effect, complete) => {
    if (effect.name !== 'rag.query') return false;
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), 22000);
    try {
      const response = await fetch('/api/rag/v1/query', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({version: 1, ...effect.payload, request_id: effect.request_id}),
        signal: controller.signal,
      });
      if (!response.ok) throw new Error(`RAG HTTP ${response.status}`);
      const result = await response.json();
      if (result.version !== 1 || result.request_id !== effect.request_id ||
          typeof result.comparison_json !== 'string' ||
          result.comparison_json.length > 16384)
        throw new Error('Invalid RAG HTTP response');
      const comparison = JSON.parse(result.comparison_json);
      if (comparison.schema !== 'crexx-rag.query-comparison/1' ||
          comparison.graph_hops !== effect.payload.hops ||
          !Array.isArray(comparison.routes) || comparison.routes.length !== 3 ||
          comparison.routes.some((route, index) =>
            route?.route !== ['keyword', 'embedding', 'graph'][index]))
        throw new Error('Invalid RAG comparison response');
      await complete(effect, 'rag.query.completed', {
        comparison_json: result.comparison_json,
      });
    } catch {
      await complete(effect, 'ui.effect.failed', {
        code: 'service_failed', message: 'Read-only query service unavailable',
      });
    } finally { clearTimeout(timer); }
    return true;
  };
}
