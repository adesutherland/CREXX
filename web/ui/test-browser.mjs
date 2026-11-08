/* Run against a packaged site served by hosted-server.mjs. */
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
    await page.goto(origin + (hosted ? '/?hosted=1' : '/'));
    await page.locator('[data-node-id="open"]').waitFor({timeout: 30000});
    const chooser = page.waitForEvent('filechooser');
    await page.locator('[data-node-id="open"]').click();
    await (await chooser).setFiles({name: 'sample.txt', mimeType: 'text/plain',
      buffer: Buffer.from('alpha beta\ngamma delta epsilon\n')});
    await page.getByText('Words: 5').waitFor({timeout: 10000});
    assert.equal(await page.locator('[data-node-id="status"]').textContent(), 'Status: Count complete');
    if (!hosted) {
      const result = await page.evaluate(async () => {
        const {LocalRoute} = await import('/local.mjs');
        const {BrowserUI} = await import('/projection.mjs');
        const root = document.createElement('div'); document.body.append(root);
        const route = new LocalRoute();
        const ui = new BrowserUI(root, route);
        await ui.start();
        const before = ui.session;
        await route.restart(frame => ui.receive(frame));
        const after = ui.session;
        route.stop(); root.remove();
        return {before, after};
      });
      assert.notEqual(result.before, result.after);
    } else {
      const result = await page.evaluate(async token => {
        const {HostedRoute} = await import('/hosted.mjs');
        const {BrowserUI} = await import('/projection.mjs');
        const root = document.createElement('div'); document.body.append(root);
        const route = new HostedRoute(token);
        const ui = new BrowserUI(root, route, {executeEffect: async () => {}});
        await ui.start();
        const before = ui.session;
        await route.reconnect();
        const after = ui.session;
        const projected = !!root.querySelector('[data-node-id="open"]');
        route.stop(); root.remove();
        return {before, after, projected};
      }, token);
      assert.equal(result.before, result.after);
      assert.equal(result.projected, true);
    }
    await page.locator('[data-node-id="quit"]').click();
    await page.waitForFunction(() => document.querySelector('#app').children.length === 0,
      {timeout: 10000});
    await page.close();
  }
  console.log('PASS: local and hosted browser UI, file effects, restart and reconnect');
} finally { await browser.close(); }
