/* Browser API glue validates the application-owned local package before use. */
const hexadecimal = bytes => [...bytes].map(byte => byte.toString(16).padStart(2, '0')).join('');

export async function asset(bundle, path) {
  const expected = bundle.assets?.[path];
  if (!expected || !/^[0-9a-f]{64}$/.test(expected.sha256) ||
      !Number.isSafeInteger(expected.bytes) || expected.bytes < 1)
    throw new Error(`Unlisted RAG asset: ${path}`);
  const url = new URL(`../${path}`, import.meta.url);
  url.searchParams.set('sha256', expected.sha256);
  let response = await fetch(url, {cache: 'force-cache'});
  let bytes = response.ok ? await response.arrayBuffer() : null;
  const matches = async value => value?.byteLength === expected.bytes &&
    hexadecimal(new Uint8Array(await crypto.subtle.digest('SHA-256', value))) === expected.sha256;
  if (!await matches(bytes)) {
    response = await fetch(url, {cache: 'reload'});
    bytes = response.ok ? await response.arrayBuffer() : null;
  }
  if (!response.ok || !bytes) throw new Error(`Missing RAG asset: ${path}`);
  if (!await matches(bytes)) throw new Error(`RAG asset checksum mismatch: ${path}`);
  return bytes;
}

export async function loadBundle() {
  const [manifestResponse, bundleResponse] = await Promise.all([
    fetch(new URL('../manifest.json', import.meta.url), {cache: 'reload'}),
    fetch(new URL('./rag-bundle.json', import.meta.url), {cache: 'reload'}),
  ]);
  if (!manifestResponse.ok || !bundleResponse.ok) throw new Error('RAG bundle descriptor unavailable');
  const manifest = await manifestResponse.json();
  const raw = await bundleResponse.arrayBuffer();
  const hash = hexadecimal(new Uint8Array(await crypto.subtle.digest('SHA-256', raw)));
  if (manifest.assets?.['apps/rag-bundle.json'] !== hash) throw new Error('RAG descriptor mismatch');
  const bundle = JSON.parse(new TextDecoder('utf-8', {fatal: true}).decode(raw));
  if (bundle.schema !== 'crexx-rag.browser-bundle/1' ||
      bundle.embedding?.model !== 'bge-small-en-v1.5' ||
      bundle.embedding?.dimension !== 384 || bundle.embedding?.codec !== 'f32le-v1' ||
      !/^[a-z0-9._-]+\.rxvec$/.test(bundle.sidecar ?? ''))
    throw new Error('Incompatible RAG bundle');
  return bundle;
}
