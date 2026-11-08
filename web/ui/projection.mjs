/* One browser projection for local and hosted cREXX sessions. */
export class BrowserUI {
  constructor(root, route, {executeEffect} = {}) {
    this.root = root;
    this.route = route;
    this.executeEffect = executeEffect ?? (effect => this.fileEffect(effect));
    this.session = '';
    this.revision = 0;
    this.seenEffects = new Set();
    this.files = new Map();
    this.closed = false;
  }

  async start() {
    await this.route.start(frame => this.receive(frame));
  }

  async send(type, fields = {}) {
    if (this.closed || !this.session) throw new Error('UI session is unavailable');
    return this.route.send({version: 1, session: this.session, base: this.revision, type, ...fields});
  }

  receive(frame) {
    if (!frame || frame.version !== 1 || typeof frame.session !== 'string' ||
        !Number.isSafeInteger(frame.revision) || frame.revision < 0)
      throw new Error('Invalid UI frame');
    if (this.session && frame.session !== this.session) {
      this.revision = 0;
      this.seenEffects.clear();
      this.files.clear();
    }
    this.session = frame.session;
    if (frame.type === 'view') {
      if (frame.revision < this.revision) return;
      this.revision = frame.revision;
      this.project(frame.view);
    } else if (frame.type === 'effect') {
      const message = frame.message;
      if (!message || message.version !== 0 || message.direction !== 'effect' ||
          message.session_id !== this.session || typeof message.request_id !== 'string')
        throw new Error('Invalid effect frame');
      if (this.seenEffects.has(message.request_id)) return;
      this.seenEffects.add(message.request_id);
      void this.executeEffect(message).catch(error => this.complete(message, 'ui.effect.failed',
          {code: 'browser_effect', message: String(error)}));
    } else if (frame.type === 'closed') {
      this.closed = true;
      this.files.clear();
      this.seenEffects.clear();
      this.root.replaceChildren();
    } else if (frame.type === 'refusal') {
      this.root.dispatchEvent(new CustomEvent('ui-refusal', {detail: frame.result}));
    } else throw new Error('Unknown UI frame');
  }

  project(view) {
    if (!view || typeof view.title !== 'string' || !Array.isArray(view.nodes) || view.nodes.length > 64)
      throw new Error('Invalid UI view');
    const ids = new Set();
    const fragment = document.createDocumentFragment();
    const heading = document.createElement('h1');
    heading.textContent = view.title;
    fragment.append(heading);
    const grid = document.createElement('div');
    grid.className = 'ui-grid';
    for (const node of view.nodes) {
      if (!node || typeof node.id !== 'string' || ids.has(node.id) ||
          !Number.isInteger(node.row) || !Number.isInteger(node.column) ||
          node.row < 0 || node.column < 0) throw new Error('Invalid UI node');
      ids.add(node.id);
      let element;
      if (node.kind === 'button') {
        element = document.createElement('button');
        element.type = 'button';
        element.addEventListener('click', () => void this.send('command', {command: node.action}));
      } else if (node.kind === 'line') element = document.createElement('hr');
      else if (node.kind === 'label') element = document.createElement('div');
      else throw new Error(`Unsupported browser control: ${node.kind}`);
      element.dataset.nodeId = node.id;
      element.textContent = node.kind === 'line' ? '' : String(node.text ?? '');
      element.style.gridRowStart = String(node.row + 1);
      element.style.gridColumnStart = String(node.column + 1);
      grid.append(element);
    }
    fragment.append(grid);
    this.root.replaceChildren(fragment);
  }

  async complete(request, name, payload) {
    if (this.closed || request.session_id !== this.session) return;
    return this.send('complete', {message: {
      version: 0, direction: 'event', name, target: request.target,
      payload, request_id: request.request_id, session_id: request.session_id, sequence: 0,
    }});
  }

  async fileEffect(request) {
    if (request.name === 'ui.resource.choose') {
      const input = document.createElement('input');
      input.type = 'file';
      input.accept = 'text/plain,.txt';
      const file = await new Promise(resolve => {
        input.addEventListener('change', () => resolve(input.files?.[0] ?? null), {once: true});
        input.addEventListener('cancel', () => resolve(null), {once: true});
        input.click();
      });
      if (!file) return this.complete(request, 'ui.effect.cancelled', {reason: 'chooser_cancelled'});
      const id = crypto.randomUUID();
      this.files.set(id, file);
      return this.complete(request, 'ui.resource.selected', {resource: {
        id, scope: 'session', display_name: file.name, readable: true, writable: false,
      }});
    }
    if (request.name === 'io.text.read') {
      const resource = request.payload?.resource;
      const file = this.files.get(resource?.id);
      if (!file) return this.complete(request, 'ui.effect.failed',
          {code: 'invalid_grant', message: 'Browser text grant is unavailable'});
      if (file.size > 131072) return this.complete(request, 'ui.effect.failed',
          {code: 'resource_limit', message: 'Browser text file exceeds 128 KiB'});
      const text = await file.text();
      const records = text.replace(/\r\n?/g, '\n').split('\n');
      if (records.at(-1) === '') records.pop();
      return this.complete(request, 'io.text.read.completed', {resource, records});
    }
    return this.complete(request, 'ui.effect.failed',
        {code: 'unsupported_effect', message: `No browser executor for ${request.name}`});
  }
}
