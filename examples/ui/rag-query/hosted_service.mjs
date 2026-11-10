/* Application-owned hosted adapter for the read-only cREXX-RAG query surface. */
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';

const execute = promisify(execFile);

export function supports(effect) {
  return ['rag.query', 'rag.answer', 'rag.source'].includes(effect?.name);
}

async function httpService(effect, {signal, progress} = {}) {
  const configured = process.env.CREXX_UI_RAG_HTTP_ORIGIN;
  if (!configured) throw new Error('Hosted answer service is not configured');
  const origin = new URL(configured);
  if (!['http:', 'https:'].includes(origin.protocol) || origin.username || origin.password)
    throw new Error('Invalid hosted answer origin');
  const corpus = process.env.CREXX_UI_RAG_CORPUS;
  const payload = effect.payload;
  if (!corpus || payload?.corpus !== corpus ||
      (effect.name === 'rag.answer' && (typeof payload.question !== 'string' ||
        !payload.question.trim() || payload.question.length > 1024)) ||
      (effect.name === 'rag.source' && (typeof payload.citation !== 'string' ||
        !payload.citation.startsWith('crexx-rag:') || payload.citation.length > 512)))
    throw new Error('Invalid hosted answer request');
  const post = async (path, body, request_id = effect.request_id) => {
    const response = await fetch(new URL(path, origin), {method: 'POST', signal,
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({version: 1, ...body, request_id})});
    if (!response.ok) throw new Error('Hosted RAG HTTP request failed');
    const text = await response.text();
    if (text.length > 262144) throw new Error('Hosted RAG response exceeds limit');
    const result = JSON.parse(text);
    if (result.version !== 1 || result.request_id !== request_id)
      throw new Error('Hosted RAG response correlation failed');
    return result;
  };
  if (effect.name === 'rag.source') {
    const source = await post('/api/rag/v1/source', payload);
    const source_json = source.source_json ?? JSON.stringify(source);
    const context = JSON.parse(source_json);
    if (source_json.length > 49152 || context.schema !== 'crexx-rag.source-context/1' ||
        context.citation !== payload.citation) throw new Error('Invalid captured source response');
    /* This older bearer-token UI relay has no original-file navigation route.
     * The direct same-origin HTTP assembly keeps its approved original URL. */
    if (typeof context.original_url === 'string' && context.original_url.startsWith('/'))
      context.original_url = '';
    return {name: 'rag.source.completed', payload: {source_json: JSON.stringify(context)}};
  }
  let completed = 0;
  const report = async message => {
    if (progress && !signal?.aborted && message)
      await progress({name: 'ui.effect.progress', payload: {completed: ++completed, message: message.slice(0, 1024)}});
  };
  const prepare = await post('/api/rag/v1/answer/prepare', payload);
  if (prepare.schema !== 'crexx-rag.answer-preparation/1' || prepare.question !== payload.question ||
      !['ready', 'insufficient_evidence', 'scope_refused'].includes(prepare.status) ||
      typeof prepare.library_id !== 'string' || !Number.isSafeInteger(prepare.generation) ||
      !Array.isArray(prepare.activities) || prepare.activities.length > 16 ||
      !Array.isArray(prepare.research?.searches) || prepare.research.searches.length > 2 ||
      prepare.research.searches.some(text => typeof text !== 'string' || text.length > 256) ||
      !Array.isArray(prepare.research?.reads) || prepare.research.reads.length > 4 ||
      prepare.research.reads.some(citation => typeof citation !== 'string' || citation.length > 512 ||
        !citation.startsWith(`crexx-rag:${prepare.library_id}:`)))
    throw new Error('Invalid hosted answer preparation');
  for (const activity of prepare.activities)
    if (typeof activity?.message === 'string' && activity.message.length <= 512)
      await report(activity.message);
  if (prepare.status === 'ready') {
    for (const [index, citation] of prepare.research.reads.entries()) {
      const raw = await post('/api/rag/v1/source', {corpus, citation}, `${effect.request_id}-read-${index + 1}`);
      const source = raw.source_json ? JSON.parse(raw.source_json) : raw;
      if (source.schema !== 'crexx-rag.source-context/1' || source.citation !== citation ||
          source.library_id !== prepare.library_id || source.generation !== prepare.generation ||
          typeof source.title !== 'string' || typeof source.excerpt !== 'string')
        throw new Error('Invalid prepared source read');
      await report(`Read captured source: ${source.title}\n${source.excerpt}`);
    }
  }
  const final = await post('/api/rag/v1/answer', {...payload, prepared: {
    library_id: prepare.library_id, generation: prepare.generation,
    searches: prepare.research.searches, reads: prepare.research.reads,
  }});
  const answer_json = final.answer_json ?? JSON.stringify(final);
  const answer = JSON.parse(answer_json);
  if (answer_json.length > 49152 || answer.schema !== 'crexx-rag.answer/1' ||
      !['answered', 'insufficient_evidence', 'scope_refused'].includes(answer.status) ||
      answer.library_id !== prepare.library_id || answer.generation !== prepare.generation ||
      answer.question !== payload.question || typeof answer.answer !== 'string' || answer.answer.length > 12000 ||
      !Array.isArray(answer.sources) || answer.sources.length > 8)
    throw new Error('Invalid hosted cited answer');
  for (const activity of (Array.isArray(answer.activities) ? answer.activities.slice(0, 16) : []))
    if (typeof activity?.message === 'string' && activity.message.length <= 512)
      await report(activity.message);
  for (const source of answer.sources)
    if (typeof source.original_url === 'string' && source.original_url.startsWith('/'))
      source.original_url = '';
  return {name: 'rag.answer.completed', payload: {answer_json: JSON.stringify(answer)}};
}

export async function run(effect, context = {}) {
  const {signal} = context;
  if (['rag.answer', 'rag.source'].includes(effect.name)) return httpService(effect, context);
  const corpus = process.env.CREXX_UI_RAG_CORPUS;
  const binary = process.env.CREXX_UI_RAG_BINARY;
  const library = process.env.CREXX_UI_RAG_LIBRARY;
  const config = process.env.CREXX_UI_RAG_CONFIG;
  if (!binary || !library || !config || !corpus)
    throw new Error('RAG service is not configured');
  const request = effect.payload;
  if (request?.corpus !== corpus || request?.mode !== 'compare' ||
      typeof request?.question !== 'string' || !request.question.trim() ||
      request.question.length > 256 || /[\r\n]/.test(request.question) ||
      !Number.isSafeInteger(request.hops) || request.hops < 0 || request.hops > 4)
    throw new Error('Invalid read-only RAG query');

  const {stdout} = await execute(binary, [
    '--progress', 'off', '--format', 'json', '--access', 'read',
    '--library', library, '--config-file', config,
    'query', 'compare', request.question,
    '--limit', '3', '--hops', String(request.hops),
  ], {signal, timeout: 20000, maxBuffer: 524288,
    env: {PATH: process.env.PATH, HOME: process.env.HOME, NO_COLOR: '1'}});
  const result = JSON.parse(stdout);
  const fields = result?.records?.[0]?.fields;
  if (result?.operation !== 'query.compare' || result?.status !== 'ok' ||
      result?.exit_code !== 0 || fields?.provider_calls !== 1 ||
      fields?.query_gaps_recorded !== 0 ||
      typeof fields?.comparison_json !== 'string' ||
      fields.comparison_json.length > 16384)
    throw new Error('RAG read-only query contract failed');
  const comparison = JSON.parse(fields.comparison_json);
  const libraryId = comparison.library_id;
  if (comparison.schema !== 'crexx-rag.query-comparison/1' ||
      typeof libraryId !== 'string' || !libraryId || libraryId.length > 128 ||
      !Number.isSafeInteger(comparison.generation) || comparison.generation < 0 ||
      comparison.graph_hops !== request.hops ||
      typeof comparison.model !== 'string' || !comparison.model ||
      comparison.model.length > 128 ||
      typeof comparison.embedding_profile_id !== 'string' ||
      !comparison.embedding_profile_id ||
      !Array.isArray(comparison.routes) || comparison.routes.length !== 3)
    throw new Error('RAG comparison identity is invalid');
  for (const [index, route] of comparison.routes.entries()) {
    if (route?.route !== ['keyword', 'embedding', 'graph'][index] ||
        !Number.isSafeInteger(route.candidate_count) || route.candidate_count < 0 ||
        !Array.isArray(route.passages) || route.passages.length > 3)
      throw new Error('RAG comparison route is invalid');
    if (index === 2) {
      const graph = route.graph;
      if (!graph || !Number.isSafeInteger(graph.anchor_count) ||
          graph.anchor_count < 0 || !Array.isArray(graph.anchors) ||
          graph.anchors.length > 8 || graph.anchors.length > graph.anchor_count ||
          !graph.anchors.every(id => typeof id === 'string' && id.length > 0 && id.length <= 128) ||
          !['both', 'outbound', 'inbound'].includes(graph.direction) ||
          !Number.isSafeInteger(graph.claim_count) || graph.claim_count < 0 ||
          !Array.isArray(graph.claims) || graph.claims.length > 4 ||
          graph.claims.length > graph.claim_count ||
          (graph.anchor_count === 0 && graph.claim_count !== 0))
        throw new Error('RAG graph trace is invalid');
      for (const claim of graph.claims) {
        const chain = claim?.path?.claim_ids;
        if (!claim || typeof claim.claim_id !== 'string' || !claim.claim_id ||
            typeof claim.subject !== 'string' || claim.subject.length > 100 ||
            typeof claim.relationship !== 'string' || claim.relationship.length > 100 ||
            typeof claim.object !== 'string' || claim.object.length > 100 ||
            typeof claim.stored_direction !== 'string' ||
            !Number.isSafeInteger(claim.hop) || claim.hop < 1 ||
            claim.hop > request.hops || !Array.isArray(chain) ||
            chain.length !== claim.hop || chain.at(-1) !== claim.claim_id ||
            !chain.every(id => typeof id === 'string' && id.length > 0))
          throw new Error('RAG graph path is invalid');
      }
    }
    for (const passage of route.passages) {
      let validUrl = passage.source_url === '';
      if (!validUrl && typeof passage.source_url === 'string' &&
          passage.source_url.length <= 2048 &&
          !/[\u0000-\u001f\u007f]/.test(passage.source_url)) {
        try {
          const parsed = new URL(passage.source_url);
          validUrl = ['http:', 'https:'].includes(parsed.protocol) &&
            Boolean(parsed.hostname) && !parsed.username && !parsed.password;
        } catch { /* Reject malformed source locators below. */ }
      }
      if (typeof passage.title !== 'string' || passage.title.length > 256 ||
          typeof passage.excerpt !== 'string' || passage.excerpt.length > 900 ||
          typeof passage.citation !== 'string' ||
          !passage.citation.startsWith(`crexx-rag:${libraryId}:`) ||
          !validUrl || !Number.isSafeInteger(passage.span_start) ||
          !Number.isSafeInteger(passage.span_end) ||
          passage.span_start < 0 || passage.span_end < passage.span_start)
        throw new Error('RAG cited passage is invalid');
    }
  }
  return {name: 'rag.query.completed', payload: {
    comparison_json: fields.comparison_json,
  }};
}
