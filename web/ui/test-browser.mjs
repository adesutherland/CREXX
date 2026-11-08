/* Run against a packaged site served by hosted-server.mjs. */
import assert from 'node:assert/strict';
const {chromium} = await import(process.env.CREXX_PLAYWRIGHT ?? 'playwright');
const browser = await chromium.launch({headless: true,
  executablePath: process.env.CREXX_CHROME || undefined});
const origin = process.env.CREXX_UI_ORIGIN ?? 'http://127.0.0.1:8787';
const token = process.env.CREXX_UI_TOKEN;
if (!token) throw new Error('Set CREXX_UI_TOKEN for hosted browser checks');
try {
  const unauthenticated = await fetch(origin + '/api/sessions',
    {method: 'POST', body: JSON.stringify({app: 'text'})});
  assert.equal(unauthenticated.status, 401);
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
        const session = route.session;
        await route.stop();
        const stopped = await fetch(`/api/sessions/${encodeURIComponent(session)}`,
          {headers: {Authorization: `Bearer ${token}`}});
        root.remove();
        return {before, after, projected, stopped: stopped.status};
      }, token);
      assert.equal(result.before, result.after);
      assert.equal(result.projected, true);
      assert.equal(result.stopped, 404);
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
      const restored = await scene.evaluate(element => {
        const context = element.getContext('2d');
        const original = context.fillRect.bind(context);
        let draws = 0;
        context.fillRect = (...args) => { draws++; return original(...args); };
        const lost = new Event('contextlost', {cancelable: true});
        element.dispatchEvent(lost);
        element.dispatchEvent(new Event('contextrestored'));
        context.fillRect = original;
        return lost.defaultPrevented && draws > 0;
      });
      assert.equal(restored, true);
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
  for (const hosted of [false, true]) {
    const page = await browser.newPage();
    await page.goto(origin);
    const result = await page.evaluate(async ({hosted, token}) => {
      const {BrowserUI} = await import('/projection.mjs');
      const {LocalRoute} = await import('/local.mjs');
      const {HostedRoute} = await import('/hosted.mjs');
      const root = document.createElement('div'); document.body.append(root);
      const route = hosted ? new HostedRoute(token, '', 'probe') : new LocalRoute(undefined, 'probe');
      const requests = [];
      let ui;
      ui = new BrowserUI(root, route, {executeEffect: async request => {
        requests.push(request);
        await ui.complete(request, 'probe.completed', {value: 'bounded answer'});
      }});
      const wait = async predicate => {
        for (let attempt = 0; attempt < 100; attempt++) {
          if (predicate()) return;
          await new Promise(resolve => setTimeout(resolve, 50));
        }
        throw new Error('Service probe timed out');
      };
      await ui.start();
      await ui.send('command', {command: 'probe.run'});
      await wait(() => root.querySelector('[data-node-id="status"]')?.textContent === 'Result: bounded answer');
      const evidence = {requests: requests.length, name: requests[0]?.name,
        key: requests[0]?.payload?.key, target: requests[0]?.target,
        result: root.querySelector('[data-node-id="status"]')?.textContent};
      await ui.send('command', {command: 'probe.unsupported'});
      await wait(() => root.querySelector('[data-node-id="status"]')?.textContent === 'Failed: unsupported_capability');
      evidence.refusal = root.querySelector('[data-node-id="status"]')?.textContent;
      evidence.requests = requests.length;
      await ui.send('close');
      await wait(() => ui.closed);
      await route.stop(); root.remove();
      return evidence;
    }, {hosted, token});
    assert.deepEqual(result, {requests: 1, name: 'probe.fetch', key: 'alpha',
      target: 'root', result: 'Result: bounded answer', refusal: 'Failed: unsupported_capability'});
    await page.close();
  }
  const page = await browser.newPage();
  await page.goto(origin);
  assert.equal(await page.evaluate(async () => {
    const {BrowserUI} = await import('/projection.mjs');
    const root = document.createElement('div');
    const ui = new BrowserUI(root, {stop() {}});
    ui.project({title: 'valid', nodes: [{id: 'status', kind: 'label', text: 'intact', row: 0, column: 0}]});
    const before = root.innerHTML;
    try { ui.project({title: 'invalid', nodes: [{id: 'bad', kind: 'unavailable', row: 0, column: 0}]}); }
    catch { return root.innerHTML === before; }
    return false;
  }), true);
  assert.deepEqual(await page.evaluate(async () => {
    const {BrowserUI} = await import('/projection.mjs');
    const ui = new BrowserUI(document.createElement('div'), {stop() {}});
    let outcome;
    ui.complete = async (_request, name, payload) => { outcome = {name, code: payload.code}; };
    await ui.fileEffect({name: 'io.text.read', payload: {resource: {
      id: 'forged', display_name: 'sample.txt', scope: 'session'}}});
    return outcome;
  }), {name: 'ui.effect.failed', code: 'invalid_grant'});
  await page.close();
  console.log('PASS: local and hosted browser forms, graph, map, service result, restart and reconnect');
} finally { await browser.close(); }
