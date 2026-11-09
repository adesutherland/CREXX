/* Focused real-browser route check for two independently packaged applications. */
import assert from 'node:assert/strict';
const {chromium} = await import(process.env.CREXX_PLAYWRIGHT ?? 'playwright');
const browser = await chromium.launch({headless: true,
  executablePath: process.env.CREXX_CHROME || undefined});
const origin = process.env.CREXX_UI_ORIGIN ?? 'http://127.0.0.1:8787';
const token = process.env.CREXX_UI_TOKEN;
if (!token) throw new Error('Set CREXX_UI_TOKEN');
try {
  for (const hosted of [false, true]) {
    for (const app of ['graph_app', 'map_app']) {
      const context = await browser.newContext();
      const page = await context.newPage();
      if (hosted) page.on('dialog', dialog => dialog.accept(token));
      await page.goto(`${origin}/?app=${app}${hosted ? '&hosted=1' : ''}`);
      await page.locator('[data-node-id="state"]').waitFor({timeout: 30000});
      if (app === 'graph_app') {
        await page.locator('[data-node-id="create"]').click();
        await page.waitForFunction(() => document.querySelector('[data-node-id="state"]')?.textContent.includes(' C: 1 '));
        await page.locator('[data-node-id="save"]').click();
        try { await page.getByText('Graph saved (revision 1)').waitFor(); }
        catch (error) { console.error(await page.locator('#app').innerText()); throw error; }
        await page.locator('[data-node-id="delete"]').click();
        await page.waitForFunction(() => document.querySelector('[data-node-id="state"]')?.textContent.includes(' C: 0 '));
        await page.locator('[data-node-id="load"]').click();
        try { await page.getByText('Graph loaded (revision 1)').waitFor({timeout: 5000}); }
        catch (error) { console.error(await page.locator('#app').innerText()); throw error; }
        assert.match(await page.locator('[data-node-id="state"]').textContent(), /C: 1/);
        await page.locator('[data-node-id="undo"]').click();
        await page.waitForFunction(() => document.querySelector('[data-node-id="state"]')?.textContent.includes(' C: 0 '));
      } else {
        await page.locator('[data-node-id="turn"]').click();
        await page.getByText('Turn saved (revision 1)').waitFor();
        await page.locator('[data-node-id="restore"]').click();
        await page.getByText('Turn loaded (revision 1)').waitFor();
        assert.match(await page.locator('[data-node-id="state"]').textContent(), /Turn: 2 Unit x: 20480/);
      }
      await page.locator('[data-node-id="quit"]').click();
      await page.locator('[data-node-id="state"]').waitFor({state: 'detached'});
      await context.close();
    }
  }
  const unavailable = await browser.newContext();
  await unavailable.addInitScript(() => {
    Storage.prototype.getItem = () => { throw new Error('storage unavailable'); };
  });
  const fallback = await unavailable.newPage();
  await fallback.goto(`${origin}/?app=graph_app`);
  await fallback.locator('[data-node-id="save"]').waitFor();
  await fallback.locator('[data-node-id="save"]').click();
  await fallback.getByText('Graph repository failed: unsupported_capability').waitFor();
  await fallback.locator('[data-node-id="quit"]').click();
  await unavailable.close();
  console.log('PASS: graph and map packages on local and hosted browser routes');
} finally { await browser.close(); }
