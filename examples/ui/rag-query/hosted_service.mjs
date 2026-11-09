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
  if (request?.corpus !== corpus || request?.mode !== 'lexical' ||
      typeof request?.question !== 'string' || !request.question.trim() ||
      request.question.length > 256 || /[\r\n]/.test(request.question))
    throw new Error('Invalid read-only RAG query');

  const {stdout} = await execute(binary, [
    '--progress', 'off', '--format', 'json', '--access', 'read',
    '--library', library, '--config-file', config,
    'query', 'inspect', request.question, '--mode', 'lexical',
    '--limit', '1', '--hops', '0',
  ], {signal, timeout: 10000, maxBuffer: 524288,
    env: {PATH: process.env.PATH, HOME: process.env.HOME, NO_COLOR: '1'}});
  const result = JSON.parse(stdout);
  const fields = result?.records?.[0]?.fields;
  if (result?.operation !== 'query.inspect' || result?.status !== 'ok' ||
      result?.exit_code !== 0 || fields?.provider_calls !== 0 ||
      fields?.query_gaps_recorded !== 0 || typeof fields?.evidence_json !== 'string')
    throw new Error('RAG read-only query contract failed');
  const evidence = JSON.parse(fields.evidence_json);
  const passage = evidence.passages?.[0];
  const libraryId = evidence.library_id;
  const generation = evidence.active_generation;
  const citation = passage?.citation?.id ?? '';
  const excerpt = passage?.text ?? '';
  if (typeof libraryId !== 'string' || !libraryId || libraryId.length > 128 ||
      !Number.isSafeInteger(generation) || generation < 0 ||
      typeof citation !== 'string' || citation.length > 512 ||
      typeof excerpt !== 'string' || excerpt.length > 2048 ||
      Boolean(citation) !== Boolean(excerpt) ||
      (citation && !citation.startsWith(`crexx-rag:${libraryId}:`)))
    throw new Error('RAG cited result is invalid or exceeds the UI bound');
  return {name: 'rag.query.completed', payload: {
    library_id: libraryId, generation, citation, excerpt,
  }};
}
