/* Real local-Wasm and hosted-native process form/graph interactions. */
import assert from 'node:assert/strict';
const {chromium} = await import(process.env.CREXX_PLAYWRIGHT ?? 'playwright');
const browser = await chromium.launch({headless: true,
  executablePath: process.env.CREXX_CHROME || undefined});
const origin = process.env.CREXX_UI_ORIGIN ?? 'http://127.0.0.1:8787';
const token = process.env.CREXX_UI_TOKEN;
if (!token) throw new Error('Set CREXX_UI_TOKEN for hosted browser checks');

try {
  for (const hosted of [false, true]) {
    const page = await browser.newPage();
    if (hosted) page.on('dialog', dialog => dialog.accept(token));
    await page.goto(`${origin}/?app=process${hosted ? '&hosted=1' : ''}`);
    const scene = page.locator('[data-node-id="process"]');
    const name = page.locator('[data-node-id="step.name"]');
    await name.waitFor({timeout: 30000});
    assert.equal(await name.inputValue(), 'Request');
    assert.equal(await scene.locator('rect').count(), 3);
    assert.equal(await scene.locator('line').count(), 2);

    const staleName = await name.elementHandle();
    await scene.locator('[data-item-id="step-2"]').click();
    await page.locator('[data-node-id="selected"]').getByText('step-2', {exact: false}).waitFor();
    await staleName.evaluate(element => {
      element.value = 'Stale edit';
      element.dispatchEvent(new Event('change', {bubbles: true}));
    });
    const beforeResync = await name.elementHandle();
    await beforeResync.waitForElementState('hidden', {timeout: 10000});
    assert.equal(await name.inputValue(), 'Review');

    const beforeInvalid = await name.elementHandle();
    await beforeInvalid.evaluate(element => {
      element.value = 'x'.repeat(1025);
      element.dispatchEvent(new Event('change', {bubbles: true}));
    });
    await page.getByRole('alert').getByText('invalid_input', {exact: false}).waitFor();
    await scene.locator('[data-item-id="step-2"]').click();
    await beforeInvalid.waitForElementState('hidden', {timeout: 10000});
    assert.equal(await name.inputValue(), 'Review');

    await name.fill('Check request');
    await name.press('Tab');
    await page.waitForFunction(() => document.querySelector('[data-node-id="process"]')?.textContent.includes('Check request'));
    assert.equal(await name.inputValue(), 'Check request');
    const owner = page.locator('[data-node-id="step.owner"]');
    const beforeOwner = await owner.elementHandle();
    await owner.fill('Team lead');
    await owner.press('Tab');
    await beforeOwner.waitForElementState('hidden', {timeout: 10000});
    assert.equal(await owner.inputValue(), 'Team lead');

    await page.locator('[data-node-id="add"]').click();
    await page.waitForFunction(() => document.querySelector('[data-node-id="process"]')?.querySelectorAll('rect').length === 4);
    assert.equal(await name.inputValue(), 'New step');
    await page.locator('[data-node-id="earlier"]').click();
    await page.locator('[data-node-id="selected"]').getByText('Step 2 of 4', {exact: false}).waitFor();
    await page.locator('[data-node-id="undo"]').click();
    await page.locator('[data-node-id="selected"]').getByText('Step 3 of 4', {exact: false}).waitFor();
    assert.equal(await scene.locator('rect').count(), 4);
    await page.locator('[data-node-id="redo"]').click();
    await page.locator('[data-node-id="selected"]').getByText('Step 2 of 4', {exact: false}).waitFor();
    await page.locator('[data-node-id="remove"]').click();
    await page.waitForFunction(() => document.querySelector('[data-node-id="process"]')?.querySelectorAll('rect').length === 3);
    await scene.press('ArrowLeft');
    await page.locator('[data-node-id="selected"]').getByText('step-1', {exact: false}).waitFor();

    await page.locator('[data-node-id="quit"]').click();
    await page.waitForFunction(() => document.querySelector('#app').children.length === 0);
    await page.close();
  }
  console.log('PASS: process designer graph and editable form on both browser routes');
} finally { await browser.close(); }
