/* Application-owned HTTP executor; the cREXX session still owns UI state. */
export function ragQueryHttp() {
  return async (effect, complete) => {
    if (effect.name !== 'rag.query') return false;
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), 11000);
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
          typeof result.library_id !== 'string' || !result.library_id ||
          result.library_id.length > 128 ||
          !Number.isSafeInteger(result.generation) || result.generation < 0 ||
          typeof result.citation !== 'string' || result.citation.length > 512 ||
          typeof result.excerpt !== 'string' || result.excerpt.length > 2048 ||
          Boolean(result.citation) !== Boolean(result.excerpt) ||
          (result.citation && !result.citation.startsWith(`crexx-rag:${result.library_id}:`)))
        throw new Error('Invalid RAG HTTP response');
      await complete(effect, 'rag.query.completed', {
        library_id: result.library_id, generation: result.generation,
        citation: result.citation, excerpt: result.excerpt,
      });
    } catch {
      await complete(effect, 'ui.effect.failed', {
        code: 'service_failed', message: 'Read-only query service unavailable',
      });
    } finally { clearTimeout(timer); }
    return true;
  };
}
