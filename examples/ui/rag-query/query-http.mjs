/* Application-owned HTTP executor. The cREXX session owns all visible state. */
function safeLink(value, {original = false} = {}) {
  if (value === '') return true;
  if (typeof value !== 'string' || value.length > 2048 || /[\u0000-\u0020\u007f]/.test(value)) return false;
  if (original && /^\/api\/rag\/v1\/original\/[A-Za-z0-9_-]+$/.test(value)) return true;
  try {
    const url = new URL(value);
    return ['http:', 'https:'].includes(url.protocol) && Boolean(url.hostname) && !url.username && !url.password;
  } catch { return false; }
}

export function validateAnswer(value, request) {
  if (!value || value.schema !== 'crexx-rag.answer/1' ||
      !['answered', 'insufficient_evidence', 'scope_refused'].includes(value.status) ||
      typeof value.library_id !== 'string' || !value.library_id || value.library_id.length > 128 ||
      !Number.isSafeInteger(value.generation) || value.generation < 0 ||
      typeof value.provider !== 'string' || value.provider.length > 128 ||
      typeof value.model !== 'string' || value.model.length > 256 ||
      typeof value.answer !== 'string' || value.answer.length > 12000 ||
      value.question !== request.question || !Array.isArray(value.sources) || value.sources.length > 8)
    throw new Error('Invalid cited answer response');
  for (const source of value.sources) {
    if (!source || typeof source.title !== 'string' || source.title.length > 256 ||
        typeof source.excerpt !== 'string' || source.excerpt.length > 900 ||
        typeof source.citation !== 'string' || source.citation.length > 512 ||
        !source.citation.startsWith(`crexx-rag:${value.library_id}:`) ||
        !safeLink(source.source_url) ||
        !Number.isSafeInteger(source.span_start) || source.span_start < 0 ||
        !Number.isSafeInteger(source.span_end) || source.span_end < source.span_start ||
        (source.original_url !== undefined && !safeLink(source.original_url, {original: true})))
      throw new Error('Invalid answer evidence');
  }
  for (const match of value.answer.matchAll(/\[(\d+)\]/g)) {
    const number = Number(match[1]);
    if (number < 1 || number > value.sources.length) throw new Error('Unknown answer citation');
  }
  if (value.status === 'answered' && !value.sources.length)
    throw new Error('Uncited answer refused');
  if (JSON.stringify(value).length > 49152) throw new Error('Answer exceeds limit');
  return value;
}

export function validateSource(value, citation) {
  if (!value || value.schema !== 'crexx-rag.source-context/1' || value.citation !== citation ||
      typeof value.title !== 'string' || value.title.length > 256 ||
      typeof value.excerpt !== 'string' || typeof value.context_text !== 'string' ||
      value.context_text.length > 32768 || !safeLink(value.source_url) ||
      !safeLink(value.original_url ?? '', {original: true}) ||
      !Number.isSafeInteger(value.span_start) || !Number.isSafeInteger(value.span_end) ||
      !Number.isSafeInteger(value.context_start) || !Number.isSafeInteger(value.context_end) ||
      value.context_start < 0 || value.span_start < value.context_start ||
      value.span_end < value.span_start || value.context_end < value.span_end ||
      JSON.stringify(value).length > 49152)
    throw new Error('Invalid source context');
  return value;
}

function activityText(record) {
  if (!record || typeof record.message !== 'string' || record.message.length > 512 ||
      !['search', 'scope', 'read', 'answer', 'validation'].includes(record.stage)) return '';
  let message = record.message;
  if (typeof record.route === 'string') message += ` · ${record.route}`;
  if (Number.isSafeInteger(record.candidate_count)) message += ` · ${record.candidate_count} candidates`;
  for (const source of (Array.isArray(record.sources) ? record.sources.slice(0, 4) : []))
    if (typeof source.title === 'string') message += `\n${source.title.slice(0, 256)}`;
  return message.slice(0, 1024);
}

export function ragQueryHttp() {
  return async (effect, complete, {signal} = {}) => {
    if (!['rag.query', 'rag.answer', 'rag.source'].includes(effect.name)) return false;
    const controller = new AbortController();
    const abort = () => controller.abort();
    signal?.addEventListener('abort', abort, {once: true});
    if (signal?.aborted) controller.abort();
    const timer = setTimeout(abort, effect.name === 'rag.answer' ? 120000 : 22000);
    const post = async (path, payload, requestId = effect.request_id) => {
      const response = await fetch(path, {
        method: 'POST', headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({version: 1, ...payload, request_id: requestId}), signal: controller.signal,
      });
      if (!response.ok) throw new Error(`RAG HTTP ${response.status}`);
      const result = await response.json();
      if (result.version !== 1 || result.request_id !== requestId) throw new Error('Invalid RAG response envelope');
      return result;
    };
    try {
      if (effect.name === 'rag.query') {
        const result = await post('/api/rag/v1/query', effect.payload);
        if (typeof result.comparison_json !== 'string' || result.comparison_json.length > 16384)
          throw new Error('Invalid RAG HTTP response');
        const comparison = JSON.parse(result.comparison_json);
        if (comparison.schema !== 'crexx-rag.query-comparison/1' ||
            comparison.graph_hops !== effect.payload.hops ||
            !Array.isArray(comparison.routes) || comparison.routes.length !== 3 ||
            comparison.routes.some((route, index) => route?.route !== ['keyword', 'embedding', 'graph'][index]))
          throw new Error('Invalid RAG comparison response');
        await complete(effect, 'rag.query.completed', {comparison_json: result.comparison_json});
      } else if (effect.name === 'rag.source') {
        const result = await post('/api/rag/v1/source', effect.payload);
        const context = validateSource(result.source_json ? JSON.parse(result.source_json) : result, effect.payload.citation);
        await complete(effect, 'rag.source.completed', {source_json: JSON.stringify(context)});
      } else {
        let completed = 0;
        const progress = async stage => {
          const message = JSON.stringify({schema: 'rag-ui.stage/1', ...stage});
          if (message.length > 8192) throw new Error('Research stage exceeds UI limit');
          if (!controller.signal.aborted)
            await complete(effect, 'ui.effect.progress', {completed: ++completed,
              message});
        };
        if (!['thorough', 'fast'].includes(effect.payload.model_profile))
          throw new Error('Invalid answer model profile');
        const model = effect.payload.model_profile === 'fast' ? 'GPT-6 Luna' : 'GPT-6.1 Sol';
        const seen = new Set();
        const publicActivity = async (activity, phase) => {
          const output = activityText(activity);
          if (!output || seen.has(output)) return;
          seen.add(output);
          let id = `${phase}-${seen.size}`;
          let title = 'Corpus research';
          if (activity.stage === 'scope') {
            id = phase === 'prepare' ? 'scope' : 'snapshot';
            title = phase === 'prepare' ? 'Scope and search plan' : 'Snapshot revalidation';
          }
          else if (activity.stage === 'search') title = phase === 'prepare' ? 'Initial corpus search' : 'Follow-up corpus search';
          else if (activity.stage === 'read') title = 'Captured source read';
          else if (activity.stage === 'answer') { id = 'answer'; title = 'Answer and citation checks'; }
          else if (activity.stage === 'validation') title = 'Evidence validation';
          const input = activity.stage === 'scope' ? effect.payload.question :
            activity.stage === 'search' ? (activity.message.includes(': ') ? activity.message.split(': ').slice(1).join(': ') : effect.payload.question) :
              activity.stage === 'read' ? (activity.sources?.[0]?.citation ?? 'Selected corpus citation') : 'Selected corpus evidence';
          await progress({id, state: 'done', title, summary: activity.message.slice(0, 220),
            input: String(input).slice(0, 1024),
            output: output.slice(0, 1800)});
        };
        await progress({id: 'scope', state: 'waiting', title: 'Scope and search plan',
          summary: `Waiting for ${model} to assess this question and plan corpus searches`,
          input: effect.payload.question, output: 'No result yet.'});
        const prepared = await post('/api/rag/v1/answer/prepare', effect.payload);
        if (prepared.schema !== 'crexx-rag.answer-preparation/1' ||
            !['ready', 'insufficient_evidence', 'scope_refused'].includes(prepared.status) ||
            prepared.question !== effect.payload.question ||
            typeof prepared.library_id !== 'string' || !Number.isSafeInteger(prepared.generation) ||
            !Array.isArray(prepared.activities) || prepared.activities.length > 16)
          throw new Error('Invalid answer preparation');
        for (const activity of prepared.activities) await publicActivity(activity, 'prepare');
        const research = prepared.research ?? {searches: [], reads: []};
        if (!Array.isArray(research.searches) || research.searches.length > 2 ||
            research.searches.some(text => typeof text !== 'string' || text.length > 256) ||
            !Array.isArray(research.reads) || research.reads.length > 4 ||
            research.reads.some(citation => typeof citation !== 'string' || citation.length > 512 ||
              !citation.startsWith(`crexx-rag:${prepared.library_id}:`)))
          throw new Error('Invalid preparation selectors');
        if (prepared.status === 'ready') await progress({id: 'scope', state: 'done', title: 'Scope and search plan',
          summary: 'Question accepted; bounded follow-up searches and reads selected',
          input: effect.payload.question,
          output: `Follow-up searches: ${research.searches.length ? research.searches.join('; ') : 'none'}\nCaptured reads: ${research.reads.length ? research.reads.join('; ') : 'none'}`});
        if (prepared.status === 'ready') {
          for (const [index, citation] of research.reads.entries()) {
            const id = `read-${index + 1}`;
            await progress({id, state: 'busy', title: 'Read captured source',
              summary: 'Reading the selected passage from the immutable corpus', input: citation, output: 'No result yet.'});
            const read = await post('/api/rag/v1/source', {corpus: effect.payload.corpus, citation},
              `${effect.request_id}-read-${index + 1}`);
            const source = validateSource(read.source_json ? JSON.parse(read.source_json) : read, citation);
            if (source.library_id !== prepared.library_id || source.generation !== prepared.generation)
              throw new Error('Source snapshot changed during preparation');
            await progress({id, state: 'done', title: 'Read captured source',
              summary: `Read ${source.title}`, input: citation, output: source.excerpt.slice(0, 1800)});
          }
        }
        await progress({id: 'model', state: 'waiting', title: 'Model evidence review',
          summary: `Waiting for ${model} to review evidence and draft a cited answer`,
          input: `Question: ${effect.payload.question}\nCorpus: ${effect.payload.corpus}`, output: 'No result yet.'});
        const final = await post('/api/rag/v1/answer', {...effect.payload, prepared: {
          library_id: prepared.library_id, generation: prepared.generation,
          searches: research.searches, reads: research.reads,
        }});
        const answer = validateAnswer(final.answer_json ? JSON.parse(final.answer_json) : final, effect.payload);
        if (answer.library_id !== prepared.library_id || answer.generation !== prepared.generation)
          throw new Error('Answer snapshot changed during preparation');
        await progress({id: 'model', state: 'done', title: 'Model evidence review',
          summary: `${answer.model} returned a draft for citation checks`,
          input: `Question: ${effect.payload.question}\nCorpus: ${effect.payload.corpus}`,
          output: `Status: ${answer.status}\nSources cited: ${answer.sources.length}`});
        for (const activity of (Array.isArray(answer.activities) ? answer.activities.slice(0, 16) : []))
          await publicActivity(activity, 'answer');
        await progress({id: 'answer', state: 'answer', title: 'Answer and citation checks',
          summary: answer.status === 'answered' ? `Validated reply from ${answer.model}` :
            `Completed with ${answer.status.replaceAll('_', ' ')}`,
          input: `Question: ${effect.payload.question}\nCorpus snapshot: ${answer.library_id} / ${answer.generation}`,
          output: answer.answer.slice(0, 1800)});
        await complete(effect, 'rag.answer.completed', {answer_json: JSON.stringify(answer)});
      }
    } catch (error) {
      if (effect.name === 'rag.answer' && !signal?.aborted)
        await complete(effect, 'ui.effect.progress', {completed: 1, message: JSON.stringify({schema: 'rag-ui.stage/1',
          id: 'error', state: 'error', title: 'Request failed', summary: 'The answer service could not complete this request',
          input: 'The submitted question', output: String(error?.message ?? error).slice(0, 900)})});
      if (!signal?.aborted) await complete(effect, 'ui.effect.failed', {
        code: controller.signal.aborted ? 'service_timeout' : 'service_failed',
        message: effect.name === 'rag.query' ? 'Read-only query service unavailable' :
          effect.name === 'rag.source' ? 'Captured source service unavailable' : 'Answer service unavailable. No automatic paid retry was made.',
      });
    } finally {
      clearTimeout(timer);
      signal?.removeEventListener('abort', abort);
    }
    return true;
  };
}
