/* Tile-map application append-only turn journal. */
const key = 'crexx.ui.map.turns.v1';

export function mapRepository(storage) {
  return async (request, complete) => {
    if (!['map.journal.append', 'map.journal.load'].includes(request.name)) return false;
    try {
      const raw = storage.getItem(key);
      const journal = raw === null ? [] : JSON.parse(raw);
      if (!Array.isArray(journal) || journal.length > 100000 ||
          journal.some((entry, index) => entry?.revision !== index + 1 ||
            typeof entry.record !== 'string')) throw new Error('Invalid turn journal');
      if (request.name === 'map.journal.load') {
        if (!journal.length) return complete(request, 'ui.effect.failed',
          {code: 'not_found', message: 'No saved turn'});
        return complete(request, 'map.journal.loaded', journal.at(-1));
      }
      const expected = request.payload?.expected_revision;
      const record = request.payload?.record;
      if (!Number.isSafeInteger(expected) || expected < 0 || expected > 1000000 ||
          typeof record !== 'string' || record.length > 256)
        throw new Error('Invalid turn append request');
      const parsed = JSON.parse(record);
      if (!Number.isSafeInteger(parsed?.turn) || parsed.turn < 2 || parsed.turn > 100000 ||
          parsed.unit_x !== 4096 + ((parsed.turn - 1) % 4) * 16384)
        throw new Error('Invalid turn record');
      if (journal.length !== expected)
        return complete(request, 'ui.effect.failed',
          {code: 'conflict', message: 'Turn journal revision changed'});
      if (journal.length && parsed.turn !== JSON.parse(journal.at(-1).record).turn + 1)
        return complete(request, 'ui.effect.failed',
          {code: 'turn_sequence', message: 'Turn is not the next journal entry'});
      const revision = journal.length + 1;
      journal.push({revision, record});
      storage.setItem(key, JSON.stringify(journal));
      return complete(request, 'map.journal.appended', {revision});
    } catch (error) {
      return complete(request, 'ui.effect.failed',
        {code: 'map_repository', message: String(error)});
    }
  };
}
