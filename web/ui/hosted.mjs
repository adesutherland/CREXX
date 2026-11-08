/* Authenticated hosted relay. Reconnect fetches a full authoritative view. */
class ProjectionFailure extends Error {}
export class HostedRoute {
  constructor(token, base = '', app = 'text') {
    this.token = token;
    this.base = base;
    this.app = app;
    this.cursor = 0;
    this.session = '';
    this.stopped = false;
  }
  async request(path, init = {}) {
    const response = await fetch(this.base + path, {...init, headers: {
      Authorization: `Bearer ${this.token}`,
      ...(init.body ? {'Content-Type': 'application/json'} : {}),
    }});
    if (!response.ok) throw new Error(`Hosted UI HTTP ${response.status}`);
    return response.json();
  }
  async start(onFrame) {
    this.onFrame = onFrame;
    const created = await this.request('/api/sessions', {method: 'POST', body: JSON.stringify({app: this.app})});
    this.session = created.session;
    this.cursor = 0;
    this.stopped = false;
    try { await this.pollUntilView(); }
    catch (error) { await this.stop(); throw error; }
    this.loop();
  }
  async pollUntilView() {
    for (let attempt = 0; attempt < 100; attempt++) {
      const frames = await this.poll();
      if (frames.some(frame => frame.type === 'view')) return;
      await new Promise(resolve => setTimeout(resolve, 50));
    }
    throw new Error('Hosted UI did not produce a view');
  }
  async poll() {
    const result = await this.request(`/api/sessions/${encodeURIComponent(this.session)}?after=${this.cursor}`);
    const frames = result.frames.map(entry => entry.frame);
    for (const entry of result.frames) {
      this.cursor = Math.max(this.cursor, entry.sequence);
      try { this.onFrame(entry.frame); }
      catch (error) { throw new ProjectionFailure(String(error)); }
      if (entry.frame.type === 'closed') this.stopped = true;
    }
    return frames;
  }
  async loop() {
    let failures = 0;
    while (!this.stopped) {
      try { await this.poll(); failures = 0; }
      catch (error) {
        if (this.stopped) break;
        if (error instanceof ProjectionFailure) {
          try { this.onFrame({version: 1, type: 'refusal', session: this.session,
            revision: 0, result: {code: 'invalid_frame', operation: 'browser', message: String(error)}}); }
          finally { await this.stop(); }
          break;
        }
        failures++;
        if (failures >= 3) {
          try { this.onFrame({version: 1, type: 'refusal', session: this.session,
            revision: 0, result: {code: 'transport', operation: 'browser', message: String(error)}}); }
          finally { await this.stop(); }
          break;
        }
      }
      await new Promise(resolve => setTimeout(resolve, 100));
    }
  }
  async send(input) {
    return this.request(`/api/sessions/${encodeURIComponent(this.session)}/input`,
      {method: 'POST', body: JSON.stringify(input)});
  }
  async reconnect() {
    this.cursor = 0;
    await this.pollUntilView();
  }
  async stop() {
    if (this.stopped && !this.session) return;
    this.stopped = true;
    const session = this.session;
    this.session = '';
    if (session) {
      try { await this.request(`/api/sessions/${encodeURIComponent(session)}`, {method: 'DELETE'}); }
      catch { /* the server lease also expires an abandoned session */ }
    }
  }
}
