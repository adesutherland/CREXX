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
  for (const app of ['graph', 'map']) for (const hosted of [false, true]) {
    const page = await browser.newPage();
    if (hosted) page.on('dialog', dialog => dialog.accept(token));
    await page.goto(`${origin}/?app=${app}${hosted ? '&hosted=1' : ''}`);
    const scene = page.locator(`[data-node-id="${app}"]`);
    await scene.waitFor({timeout: 30000});
    if (app === 'graph') {
      assert.equal(await scene.evaluate(element => element.tagName.toLowerCase()), 'svg');
      assert.equal(await scene.locator('rect').count(), 2);
      const firstUpdate = performance.now();
      await page.locator('[data-node-id="create"]').click();
      await page.waitForFunction(() => document.querySelector('[data-node-id="graph"]')?.querySelectorAll('rect').length === 3);
      console.log(`OBSERVE: ${hosted ? 'hosted' : 'local'} graph first update ${Math.round(performance.now() - firstUpdate)} ms end-to-end`);
      await page.locator('[data-node-id="connect"]').click();
      await page.waitForFunction(() => document.querySelector('[data-node-id="graph"]')?.querySelectorAll('line').length === 2);
      await scene.click({position: {x: 40, y: 40}});
      await page.getByText('Selected: node-a', {exact: false}).waitFor();
      await page.locator('[data-node-id="graph"]').press('ArrowRight');
      await page.getByText('A: 10240', {exact: false}).waitFor();
      await page.locator('[data-node-id="zoom"]').click();
      await page.getByText('Scale: 128', {exact: false}).waitFor();
      await page.locator('[data-node-id="pan"]').click();
      await page.getByText('Pan: 2560', {exact: false}).waitFor();
      await page.locator('[data-node-id="delete"]').click();
      await page.waitForFunction(() => document.querySelector('[data-node-id="graph"]')?.querySelectorAll('rect').length === 2);
      await page.locator('[data-node-id="undo"]').click();
      await page.waitForFunction(() => document.querySelector('[data-node-id="graph"]')?.querySelectorAll('rect').length === 3);
      await page.locator('[data-node-id="redo"]').click();
      await page.waitForFunction(() => document.querySelector('[data-node-id="graph"]')?.querySelectorAll('rect').length === 2);
    } else {
      assert.equal(await scene.evaluate(element => element.tagName.toLowerCase()), 'canvas');
      await page.getByText('Turn: 1 Unit x: 4096', {exact: false}).waitFor();
      const firstUpdate = performance.now();
      await page.locator('[data-node-id="turn"]').click();
      await page.getByText('Turn: 2 Unit x: 20480', {exact: false}).waitFor();
      console.log(`OBSERVE: ${hosted ? 'hosted' : 'local'} map first update ${Math.round(performance.now() - firstUpdate)} ms end-to-end`);
      await page.locator('[data-node-id="zoom"]').click();
      await page.getByText('Scale: 128', {exact: false}).waitFor();
      await page.locator('[data-node-id="pan"]').click();
      await page.getByText('Pan: 4096', {exact: false}).waitFor();
      await page.locator('[data-node-id="map"]').click({position: {x: 140, y: 50}});
      await page.getByText('Selected: unit-blue', {exact: false}).waitFor();
      await page.locator('[data-node-id="map"]').press('ArrowRight');
      await page.getByText('Turn: 3 Unit x: 36864', {exact: false}).waitFor();
    }
    await page.locator('[data-node-id="quit"]').click();
    await page.waitForFunction(() => document.querySelector('#app').children.length === 0);
    await page.close();
  }
  console.log('PASS: local and hosted browser forms, graph, map, restart and reconnect');
} finally { await browser.close(); }
