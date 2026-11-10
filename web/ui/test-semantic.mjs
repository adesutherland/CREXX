/* Bounded semantic documents, safe links and adapter capability preflight. */
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import {safeHttpUrl, documentBlocks} from './projection.mjs';
import {preflightPackage} from './package-registry.mjs';
assert.equal(safeHttpUrl('https://example.org/source.pdf'), 'https://example.org/source.pdf');
assert.equal(new URL(safeHttpUrl('/api/rag/v1/original/asset')).pathname, '/api/rag/v1/original/asset');
for (const unsafe of ['javascript:alert(1)', 'data:text/html,test', '//example.org/file',
  'https://user:password@example.org', 'https://example.org/\nfile', 'file:///private/source'])
  assert.throws(() => safeHttpUrl(unsafe));
const document = {blocks: [{kind: 'paragraph', runs: [
  {text: '<script>literal text</script>'}, {text: '[1]', action: 'fixture.source.one'},
  {text: 'Original', href: 'https://example.org/source'},
]}]};
assert.equal(documentBlocks(JSON.stringify(document)).length, 1);
for (const invalid of [
  {blocks: [{kind: 'html', runs: [{text: '<b>unsafe</b>'}]}]},
  {blocks: [{kind: 'paragraph', runs: [{text: 'unsafe', href: 'javascript:alert(1)'}]}]},
  {blocks: [{kind: 'paragraph', runs: [{text: 'ambiguous', href: 'https://example.org', action: 'x'}]}]},
  {blocks: Array.from({length: 65}, () => document.blocks[0])},
]) assert.throws(() => documentBlocks(JSON.stringify(invalid)));
const pkg = JSON.parse(await readFile(new URL('../../examples/ui/rag-query/rag-answer.package.json', import.meta.url), 'utf8'));
assert.equal(preflightPackage(pkg, 'local').selected.executor.capabilities.length, 3);
assert.throws(() => preflightPackage(pkg, 'local', ['form.label']), /Missing required UI operation/);
const missing = structuredClone(pkg); delete missing.routes.local.executor.capabilities;
assert.throws(() => preflightPackage(missing, 'local'), /rag.answer/);
const bad = structuredClone(pkg); bad.routes.local.executor.capabilities.push('rag.answer');
assert.throws(() => preflightPackage(bad, 'local'), /Invalid UI adapter capabilities/);
console.log('PASS: semantic document bounds, link safety and multi-capability preflight');
