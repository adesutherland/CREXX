/* Application-owned hosted adapter for the read-only cREXX-RAG query surface. */
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';

const execute = promisify(execFile);

export function supports(effect) {
  return effect?.name === 'rag.query';
}

export async function run(effect, {signal} = {}) {
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
