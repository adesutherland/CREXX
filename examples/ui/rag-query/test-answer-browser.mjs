/* Browser workflow with the real Level G owner and deterministic HTTP fixtures.
 * Serve a site packaged with rag-answer.package.json; no model/provider calls. */
import assert from 'node:assert/strict';
const {chromium} = await import(process.env.CREXX_PLAYWRIGHT ?? 'playwright');
const browser = await chromium.launch({headless: true,
  executablePath: process.env.CREXX_CHROME || undefined});
const origin = process.env.CREXX_UI_ORIGIN ?? 'http://127.0.0.1:8788';
const page = await browser.newPage({viewport: {width: 1320, height: 950}});
let finalCount = 0;
let releaseFinal;
let finalGate = new Promise(resolve => { releaseFinal = resolve; });
const question = 'What does the evidence say?\nExplain its limitations.';
const source = {title: 'Fixture research source', excerpt: 'Captured supporting evidence.',
  source_url: 'https://example.org/source.pdf', citation: 'crexx-rag:fixture:source:one', span_start: 20, span_end: 45};
try {
  await page.route('**/api/rag/v1/**', async route => {
    const input = route.request().postDataJSON();
    const path = new URL(route.request().url()).pathname;
    const base = {version: 1, request_id: input.request_id, library_id: 'fixture', generation: 7};
    let result;
    if (path.endsWith('/prepare')) result = {...base, schema: 'crexx-rag.answer-preparation/1',
      corpus: input.corpus, question: input.question, status: 'ready',
      activities: [{stage: 'search', message: 'Initial evidence search completed.', route: 'keyword', candidate_count: 1, sources: [source]}],
      usage: {provider_calls: 1, input_tokens: 700, output_tokens: 140, cost_microunits: 87},
      research: {searches: [], reads: [source.citation]}};
    else if (path.endsWith('/source')) result = {...base, ...source, schema: 'crexx-rag.source-context/1',
      context_text: 'Wider captured context.', context_start: 0, context_end: 80,
      original_url: '/api/rag/v1/original/fixture-asset'};
    else if (path.endsWith('/answer')) {
      finalCount++; await finalGate;
      result = {...base, schema: 'crexx-rag.answer/1', status: 'answered', question: input.question,
        provider: 'fixture', model: input.model_profile === 'fast' ? 'gpt-6-luna' : 'gpt-6.1-sol',
        answer: '## Findings\n**A supported statement [1]**.\n- <script>literal text</script>', sources: [source],
        usage: {provider_calls: 3, input_tokens: 2810, output_tokens: 412, cost_microunits: 487}};
    } else if (path.endsWith('/query')) result = {version: 1, request_id: input.request_id,
      comparison_json: JSON.stringify({schema: 'crexx-rag.query-comparison/1', library_id: 'fixture', generation: 7,
        model: 'fixture', embedding_profile_id: 'fixture', graph_hops: input.hops,
        routes: ['keyword', 'embedding', 'graph'].map(name => ({route: name, candidate_count: 1,
          passages: [source], ...(name === 'graph' ? {graph: {anchor_count: 0, direction: 'both', claims: []}} : {})}))})};
    else throw new Error(`Unexpected fixture route: ${path}`);
    await route.fulfill({json: result});
  });
  await page.goto(`${origin}/?app=rag_demo`);
  assert.equal(await page.locator('[data-node-id="search-panel"]').count(), 0);
  assert.match(await page.locator('[data-node-id="model-thorough"]').innerText(), /selected/);
  await page.locator('[data-node-id="model-fast"]').click();
  await page.locator('[data-node-id="model-fast"]').getByText('selected', {exact: false}).waitFor();
  const composer = page.getByRole('textbox', {name: 'Your question', exact: true});
  await composer.waitFor({timeout: 30000});
  await composer.fill(question);
  await page.locator('[data-node-id="ask"]').click();
  await page.locator('[data-node-id="stop"]').waitFor();
  assert.equal(await page.locator('[data-node-id="model-fast"]').isDisabled(), true);
  await page.locator('[data-node-id="stage-3"]').waitFor();
  assert.match(await page.locator('[data-node-id="answer-status"]').getAttribute('data-tone'), /waiting|busy|done/);
  await page.locator('[data-node-id="stage-3"]').click();
  await page.locator('[data-node-id="stage-detail"]').getByText('Captured supporting evidence.', {exact: false}).waitFor();
  assert.equal(await composer.inputValue(), question, 'Clicking Ask commits the current multiline draft');
  releaseFinal();
  await page.locator('[data-node-id="answer"] strong').getByText('A supported statement').waitFor();
  assert.equal(finalCount, 1);
  assert.ok((await page.locator('[data-node-id="answer"]').innerText()).length < 256,
    'Short answers must not be padded to the document limit');
  assert.equal(await page.locator('[data-node-id="answer-status"]').getAttribute('data-tone'), 'answer');
  assert.equal(await page.locator('[data-node-id="answer"] h4').innerText(), 'Findings');
  assert.equal(await page.locator('[data-node-id="answer"] script').count(), 0);
  assert.equal(await page.locator('[data-node-id="answer"] button').count(), 1);
  assert.match(await page.locator('[data-node-id="answer-usage"]').innerText(), /Full workflow: 4 model calls/);
  assert.equal(await page.locator('[data-node-id="answer"] strong button').count(), 1,
    'Citations inside emphasis remain actionable');
  assert.match(await page.locator('[data-node-id="answer"] li').innerText(), /<script>literal text<\/script>/);
  if (process.env.CREXX_SCREENSHOT) await page.screenshot({path: process.env.CREXX_SCREENSHOT, fullPage: true});
  await page.locator('[data-node-id="source-1"]').click();
  await page.locator('[data-node-id="source-view"]').waitFor();
  assert.match(await page.locator('[data-node-id="source-excerpt"]').innerText(), /supporting evidence/);
  assert.equal(await page.locator('[data-node-id="source-original"]').getAttribute('rel'), 'noopener noreferrer');
  await page.locator('[data-node-id="source-context"]').click();
  await page.locator('[data-node-id="source-context-text"]').getByText('Wider captured context.', {exact: true}).waitFor();
  assert.equal(await page.locator('[data-node-id="source-captured-original"]').getAttribute('href'),
    `${origin}/api/rag/v1/original/fixture-asset`);
  await page.locator('[data-node-id="source-back"]').click();
  await page.waitForFunction(() => document.activeElement?.dataset.nodeId === 'source-1');
  await page.locator('[data-node-id="explore"]').click();
  await page.getByRole('textbox', {name: 'Corpus search question'}).fill('A precise corpus query');
  await page.locator('[data-node-id="search"]').click();
  await page.locator('[data-node-id="keyword-result"]').getByText('Captured supporting evidence.', {exact: false}).waitFor();
  assert.equal(finalCount, 1, 'Raw search does not regenerate the answer');
  await page.setViewportSize({width: 390, height: 844});
  assert.equal(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), true);
  await composer.focus(); await composer.evaluate(element => element.setSelectionRange(3, 8));
  await page.locator('[data-node-id="drawer-toggle"]').click();
  assert.equal(await composer.inputValue(), question);
  finalGate = new Promise(resolve => { releaseFinal = resolve; });
  await page.locator('[data-node-id="ask"]').click();
  await page.locator('[data-node-id="stop"]').waitFor();
  await page.locator('[data-node-id="stop"]').click();
  await page.locator('[data-node-id="answer-status"]').getByText('cancelled', {exact: false}).waitFor();
  releaseFinal();
  assert.match(await page.locator('[data-node-id="answer"]').innerText(), /supported statement/);
  await page.locator('[data-node-id="quit"]').click();
  await page.waitForFunction(() => document.querySelector('#app').children.length === 0);
  console.log('PASS: multiline submit, actual activity, safe citations, source context, independent search, mobile layout and Stop');
} finally { releaseFinal?.(); await page.close(); await browser.close(); }
