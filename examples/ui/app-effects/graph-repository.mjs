/* Graph application storage policy. UI hosts only pass effect envelopes through. */
const key = 'crexx.ui.graph.document.v1';

export function graphRepository(storage) {
  return async (request, complete) => {
    if (!['graph.repository.save', 'graph.repository.load'].includes(request.name)) return false;
    try {
      const current = storage.getItem(key);
      const document = current === null ? null : JSON.parse(current);
      if (document && (!Number.isSafeInteger(document.revision) ||
          document.revision < 1 || typeof document.snapshot !== 'string'))
        throw new Error('Invalid stored graph document');
      if (request.name === 'graph.repository.load') {
        if (!document) return complete(request, 'ui.effect.failed',
          {code: 'not_found', message: 'No saved graph'});
        return complete(request, 'graph.repository.loaded', document);
      }
      const expected = request.payload?.expected_revision;
      const snapshot = request.payload?.snapshot;
      if (!Number.isSafeInteger(expected) || expected < 0 || expected > 1000000 ||
          typeof snapshot !== 'string' || snapshot.length > 16384)
        throw new Error('Invalid graph save request');
      const parsed = JSON.parse(snapshot);
      if (parsed?.format !== 1 || !parsed.model || !Array.isArray(parsed.undo) ||
          !Array.isArray(parsed.redo)) throw new Error('Invalid graph snapshot');
      if ((document?.revision ?? 0) !== expected)
        return complete(request, 'ui.effect.failed',
          {code: 'conflict', message: 'Graph revision changed'});
      const revision = expected + 1;
      storage.setItem(key, JSON.stringify({revision, snapshot}));
      return complete(request, 'graph.repository.saved', {revision});
    } catch (error) {
      return complete(request, 'ui.effect.failed',
        {code: 'graph_repository', message: String(error)});
    }
  };
}
