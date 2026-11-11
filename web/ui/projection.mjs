/* One semantic browser projection for local and hosted cREXX sessions. */
import {SceneReplica, renderScene} from './scene.mjs';

export function safeHttpUrl(value) {
  if (typeof value !== 'string' || value.length > 2048 || /[\u0000-\u0020\u007f]/.test(value))
    throw new Error('Invalid external link');
  const relative = value.startsWith('/') && !value.startsWith('//');
  const url = relative ? new URL(value, globalThis.location?.origin ?? 'https://localhost') : new URL(value);
  if (!['http:', 'https:'].includes(url.protocol) || !url.hostname || url.username || url.password)
    throw new Error('Invalid external link');
  return url.href;
}

export function documentBlocks(value) {
  if (typeof value !== 'string' || value.length > 49152) throw new Error('Invalid document');
  const parsed = JSON.parse(value);
  if (!parsed || !Array.isArray(parsed.blocks) || parsed.blocks.length > 64)
    throw new Error('Invalid document blocks');
  let runs = 0;
  for (const block of parsed.blocks) {
    if (!block || !['paragraph', 'heading', 'list-item'].includes(block.kind) ||
        !Array.isArray(block.runs) || !block.runs.length || block.runs.length > 128)
      throw new Error('Invalid document block');
    for (const run of block.runs) {
      if (++runs > 512 || !run || typeof run.text !== 'string' || run.text.length > 16384 ||
          (run.action !== undefined && (typeof run.action !== 'string' || !/^[a-z][a-z0-9_.-]{0,127}$/.test(run.action))) ||
          (run.href !== undefined && typeof run.href !== 'string') ||
          (run.href !== undefined && run.action !== undefined))
        throw new Error('Invalid document run');
      if (run.href !== undefined) safeHttpUrl(run.href);
    }
  }
  return parsed.blocks;
}

function propertiesFor(node) {
  if (node.properties !== undefined && (typeof node.properties !== 'string' || node.properties.length > 4096))
    throw new Error('Invalid node properties');
  const p = JSON.parse(node.properties ?? '{}');
  if (!p || Array.isArray(p) || typeof p !== 'object') throw new Error('Invalid node properties');
  const allowed = ['parent', 'label', 'description', 'disabled', 'expanded', 'controls', 'role', 'level', 'maxLength', 'focus', 'cancel', 'tone', 'format'];
  if (Object.keys(p).some(key => !allowed.includes(key))) throw new Error('Unknown node property');
  for (const key of ['parent', 'label', 'description', 'controls', 'role', 'focus', 'cancel'])
    if (p[key] !== undefined && (typeof p[key] !== 'string' || p[key].length > 1024))
      throw new Error('Invalid text property');
  for (const key of ['disabled', 'expanded'])
    if (p[key] !== undefined && typeof p[key] !== 'boolean') throw new Error('Invalid state property');
  if (p.role !== undefined && !['region', 'complementary', 'log', 'status'].includes(p.role))
    throw new Error('Invalid semantic role');
  if (p.level !== undefined && (!Number.isInteger(p.level) || p.level < 2 || p.level > 6))
    throw new Error('Invalid heading level');
  if (p.maxLength !== undefined && (!Number.isInteger(p.maxLength) || p.maxLength < 1 || p.maxLength > 1024))
    throw new Error('Invalid text limit');
  if (p.tone !== undefined && !['neutral', 'busy', 'waiting', 'done', 'answer', 'error'].includes(p.tone))
    throw new Error('Invalid status tone');
  if (p.format !== undefined && p.format !== 'markdown') throw new Error('Invalid document format');
  return p;
}

/* Answer Markdown is presentation only. Every character is inserted as text;
 * citation buttons come solely from validated document actions. */
export function renderMarkdownDocument(blocks, container, activate) {
  const actions = new Map();
  const source = blocks.map(block => block.runs.map(run => {
    if (run.action && /^\[[1-8]\]$/.test(run.text)) actions.set(run.text, run.action);
    return run.text;
  }).join('')).join('\n');
  const inline = (parent, value) => {
    const pattern = /(\*\*[^*\n]+\*\*|\*[^*\n]+\*|`[^`\n]+`|\[[1-8]\])/g;
    let start = 0;
    for (const match of value.matchAll(pattern)) {
      if (match.index > start) parent.append(document.createTextNode(value.slice(start, match.index)));
      const token = match[0];
      let part;
      if (actions.has(token)) {
        part = document.createElement('button'); part.type = 'button'; part.className = 'ui-reference';
        part.dataset.reference = actions.get(token);
        part.addEventListener('click', () => activate(actions.get(token)));
        part.textContent = token;
      } else if (token.startsWith('**')) {
        part = document.createElement('strong'); inline(part, token.slice(2, -2));
      } else if (token.startsWith('*')) {
        part = document.createElement('em'); inline(part, token.slice(1, -1));
      } else if (token.startsWith('`')) {
        part = document.createElement('code'); part.textContent = token.slice(1, -1);
      } else part = document.createTextNode(token);
      parent.append(part); start = match.index + token.length;
    }
    if (start < value.length) parent.append(document.createTextNode(value.slice(start)));
  };
  let list = null;
  for (const line of source.split(/\r?\n/)) {
    if (!line.trim()) { list = null; continue; }
    const heading = /^(#{1,4})\s+(.+)$/.exec(line);
    const bullet = /^\s*[-*]\s+(.+)$/.exec(line);
    const numbered = /^\s*\d+[.)]\s+(.+)$/.exec(line);
    if (bullet || numbered) {
      const tag = bullet ? 'ul' : 'ol';
      if (!list || list.tagName.toLowerCase() !== tag) {
        list = document.createElement(tag); container.append(list);
      }
      const item = document.createElement('li'); inline(item, (bullet || numbered)[1]); list.append(item);
    } else {
      list = null;
      const paragraph = document.createElement(heading ? `h${Math.min(heading[1].length + 2, 6)}` : 'p');
      inline(paragraph, heading ? heading[2] : line); container.append(paragraph);
    }
  }
}

export class BrowserUI {
  constructor(root, route, {executeEffect} = {}) {
    this.root = root;
    this.route = route;
    this.executeEffect = executeEffect ?? ((effect, _context) => this.fileEffect(effect));
    this.session = '';
    this.revision = 0;
    this.seenEffects = new Set();
    this.activeEffects = new Map();
    this.cancelledEffects = new Set();
    this.files = new Map();
    this.drafts = new Map();
    this.closed = false;
    this.scene = new SceneReplica();
    this.inputQueue = Promise.resolve();
    this.waiters = [];
    this.inputSerial = 0;
  }

  async start() { await this.route.start(frame => this.receive(frame)); }

  async send(type, fields = {}) {
    if (this.closed || !this.session) throw new Error('UI session is unavailable');
    return this.route.send({version: 1, session: this.session, base: this.revision, type, ...fields});
  }

  /* Serialize committed values and commands. A blur immediately before a click
   * must commit the draft before invoking the command against its next view. */
  input(type, fields = {}) {
    this.inputQueue = this.inputQueue.then(async () => {
      for (let attempt = 0; attempt < 4; attempt++) {
        const input_id = `input-${++this.inputSerial}`;
        const next = new Promise((resolve, reject) => {
          const waiter = {input_id, resolve, reject};
          waiter.timer = setTimeout(() => {
            this.waiters = this.waiters.filter(item => item !== waiter);
            reject(new Error('UI input acknowledgement timed out'));
          }, 15000);
          this.waiters.push(waiter);
        });
        try {
          await this.send(type, {...fields, input_id});
          if (await next) return;
          if (fields.base !== undefined) return;
          /* The owner explicitly refused the stale base before admitting it.
           * Retrying this case cannot repeat an accepted paid operation. */
        } catch (error) { this.settleInputs(error); throw error; }
      }
      throw new Error('UI changed repeatedly; please try the action again');
    }).catch(error => this.root.dispatchEvent(new CustomEvent('ui-refusal', {detail: {
      code: 'input_failed', operation: type, message: String(error),
    }})));
    return this.inputQueue;
  }

  settleInputs(error) {
    const pending = this.waiters;
    this.waiters = [];
    for (const waiter of pending) {
      clearTimeout(waiter.timer);
      if (error) waiter.reject(error); else waiter.resolve(true);
    }
  }

  receive(frame) {
    if (!frame || frame.version !== 1 || typeof frame.session !== 'string' ||
        !Number.isSafeInteger(frame.revision) || frame.revision < 0)
      throw new Error('Invalid UI frame');
    if (this.session && frame.session !== this.session) {
      for (const active of this.activeEffects.values()) active.controller.abort();
      this.activeEffects.clear();
      this.cancelledEffects.clear();
      this.revision = 0;
      this.seenEffects.clear();
      this.files.clear();
      this.drafts.clear();
      this.scene = new SceneReplica();
      this.settleInputs(new Error('UI session restarted'));
    }
    this.session = frame.session;
    if (frame.type === 'view') {
      if (frame.revision < this.revision) return;
      this.project(frame.view, frame.revision);
      this.revision = frame.revision;
      const waiter = this.waiters.find(item => item.input_id === frame.input_id);
      if (waiter) {
        this.waiters = this.waiters.filter(item => item !== waiter);
        clearTimeout(waiter.timer);
        waiter.resolve(frame.accepted === 1);
      }
    } else if (frame.type === 'effect') {
      const message = frame.message;
      if (!message || message.version !== 0 || message.direction !== 'effect' ||
          message.session_id !== this.session || typeof message.request_id !== 'string')
        throw new Error('Invalid effect frame');
      if (this.seenEffects.has(message.request_id)) return;
      this.seenEffects.add(message.request_id);
      const controller = new AbortController();
      this.activeEffects.set(message.request_id, {message, controller});
      void this.executeEffect(message, {signal: controller.signal})
        .catch(error => controller.signal.aborted ? undefined : this.complete(message, 'ui.effect.failed',
          {code: 'browser_effect', message: String(error)}));
    } else if (frame.type === 'closed') {
      this.closed = true;
      for (const active of this.activeEffects.values()) active.controller.abort();
      this.activeEffects.clear();
      this.files.clear();
      this.seenEffects.clear();
      this.drafts.clear();
      this.settleInputs();
      this.root.replaceChildren();
      void this.route.stop();
    } else if (frame.type === 'refusal') {
      this.settleInputs(new Error(frame.result?.message ?? 'Input refused'));
      this.root.dispatchEvent(new CustomEvent('ui-refusal', {detail: frame.result}));
    } else throw new Error('Unknown UI frame');
  }

  cancelEffect(name) {
    for (const [id, active] of this.activeEffects) {
      if (active.message.name !== name) continue;
      active.controller.abort();
      this.cancelledEffects.add(id);
      this.activeEffects.delete(id);
    }
    return this.input('cancel', {effect: name});
  }

  activate(command) {
    /* A genuine draft survives unrelated progress views. Detached stale DOM
     * nodes cannot manufacture one: only the input event records a draft. */
    for (const [node, draft] of this.drafts)
      if (this.root.querySelector(`[data-node-id="${node}"]`))
        void this.input('value', {node, value: draft.value});
    return this.input('command', {command});
  }

  project(view, revision = this.revision) {
    if (!view || typeof view.title !== 'string' || !Array.isArray(view.nodes) || view.nodes.length > 64)
      throw new Error('Invalid UI view');
    const ids = new Map();
    let scenes = 0;
    for (const node of view.nodes) {
      if (!node || typeof node.id !== 'string' || !/^[A-Za-z][A-Za-z0-9_.-]{0,127}$/.test(node.id) || ids.has(node.id) ||
          !Number.isInteger(node.row) || !Number.isInteger(node.column) || node.row < 0 || node.column < 0 ||
          !['scene', 'button', 'line', 'label', 'statusbar', 'input', 'textarea', 'group', 'document', 'link'].includes(node.kind) ||
          (['input', 'textarea'].includes(node.kind) && (typeof node.value !== 'string' || node.value.length > 1024)))
        throw new Error('Invalid UI node');
      const props = propertiesFor(node);
      if (props.parent && ids.get(props.parent)?.node.kind !== 'group')
        throw new Error('Parent must be an earlier semantic group');
      if (node.kind === 'document') documentBlocks(node.value);
      if (node.kind === 'link') safeHttpUrl(node.value);
      ids.set(node.id, {node, props});
      if (node.kind === 'scene' && ++scenes > 1) throw new Error('Too many scenes');
    }
    const previous = document.activeElement;
    const previousId = previous?.closest('[data-node-id]')?.dataset.nodeId;
    const selection = ['INPUT', 'TEXTAREA'].includes(previous?.tagName) ?
      {start: previous.selectionStart, end: previous.selectionEnd} : null;
    const scroll = new Map([...this.root.querySelectorAll('[data-node-id]')]
      .map(element => [element.dataset.nodeId, [element.scrollTop, element.scrollLeft]]));
    const candidateScene = this.scene.copy();
    const fragment = document.createDocumentFragment();
    const heading = document.createElement('h1');
    heading.textContent = view.title;
    fragment.append(heading);
    const grid = document.createElement('div');
    grid.className = 'ui-grid';
    const elements = new Map();
    for (const {node, props} of ids.values()) {
      let element;
      if (node.kind === 'scene') {
        candidateScene.apply(node.scenePacket, node.sceneFull);
        element = renderScene(node, candidateScene, input => void this.send(input.type, input));
      } else if (node.kind === 'group') {
        element = document.createElement('section');
        element.className = 'ui-group';
        element.setAttribute('aria-labelledby', `ui-${node.id}-heading`);
        const title = document.createElement(`h${props.level ?? 2}`);
        title.id = `ui-${node.id}-heading`;
        title.textContent = node.text;
        element.append(title);
      } else if (node.kind === 'document') {
        element = document.createElement('div');
        element.className = 'ui-document';
        const blocks = documentBlocks(node.value);
        if (props.format === 'markdown') renderMarkdownDocument(blocks, element, action => void this.activate(action));
        else for (const block of blocks) {
          const line = document.createElement(block.kind === 'heading' ? 'h3' : block.kind === 'list-item' ? 'p' : 'p');
          if (block.kind === 'list-item') line.className = 'ui-list-item';
          for (const run of block.runs) {
            let part;
            if (run.action) {
              part = document.createElement('button');
              part.type = 'button';
              part.className = 'ui-reference';
              part.dataset.reference = run.action;
              part.addEventListener('click', () => void this.activate(run.action));
            } else if (run.href) {
              part = document.createElement('a');
              part.href = safeHttpUrl(run.href);
              part.target = '_blank';
              part.rel = 'noopener noreferrer';
            } else part = document.createElement('span');
            part.textContent = run.text;
            line.append(part);
          }
          element.append(line);
        }
      } else if (node.kind === 'button') {
        element = document.createElement('button');
        element.type = 'button';
        element.textContent = node.text;
        element.disabled = props.disabled ?? false;
        element.addEventListener('click', () => void (props.cancel ? this.cancelEffect(props.cancel) :
          this.activate(node.action)));
      } else if (['input', 'textarea'].includes(node.kind)) {
        element = document.createElement(node.kind === 'textarea' ? 'textarea' : 'input');
        if (node.kind === 'input') element.type = 'text';
        else element.rows = 4;
        const draft = this.drafts.get(node.id);
        if (draft?.value === node.value) this.drafts.delete(node.id);
        element.value = this.drafts.get(node.id)?.value ?? node.value;
        element.maxLength = props.maxLength ?? 1024;
        element.disabled = props.disabled ?? false;
        const anchor = ids.get(node.anchor)?.node;
        element.setAttribute('aria-label', props.label ?? (anchor?.kind === 'label' ? anchor.text : node.id));
        element.addEventListener('input', () => this.drafts.set(node.id, {value: element.value}));
        element.addEventListener('change', () => {
          const value = element.value;
          void this.input('value', {base: revision, node: node.id, value});
        });
      } else if (node.kind === 'link') {
        element = document.createElement('a');
        element.textContent = node.text;
        element.href = safeHttpUrl(node.value);
        element.target = '_blank';
        element.rel = 'noopener noreferrer';
      } else if (node.kind === 'line') element = document.createElement('hr');
      else {
        element = document.createElement('div');
        element.textContent = node.text ?? '';
        if (node.kind === 'statusbar') {
          element.setAttribute('role', 'status');
          element.setAttribute('aria-live', 'polite');
          element.setAttribute('aria-atomic', 'true');
        }
      }
      element.dataset.nodeId = node.id;
      if (props.tone) element.dataset.tone = props.tone;
      element.id = `ui-${node.id}`;
      if (props.role) element.setAttribute('role', props.role);
      if (props.label && !['input', 'textarea'].includes(node.kind)) element.setAttribute('aria-label', props.label);
      if (props.description) element.setAttribute('aria-description', props.description);
      if (props.expanded !== undefined) element.setAttribute('aria-expanded', String(props.expanded));
      if (props.controls) element.setAttribute('aria-controls', `ui-${props.controls}`);
      if (props.parent) elements.get(props.parent).append(element);
      else {
        element.style.gridRowStart = String(node.row + 1);
        element.style.gridColumnStart = String(node.column + 1);
        grid.append(element);
      }
      elements.set(node.id, element);
    }
    fragment.append(grid);
    this.root.replaceChildren(fragment);
    this.scene = candidateScene;
    for (const [id, [top, left]] of scroll) {
      const element = elements.get(id);
      if (element) { element.scrollTop = top; element.scrollLeft = left; }
    }
    /* Focus directives are application owned. Unchanged directives must not
     * steal focus again when progress updates project an otherwise same view. */
    const focusRequest = [...ids.values()].map(record => record.props.focus).find(Boolean);
    const changedFocus = focusRequest && focusRequest !== this.lastFocusRequest;
    const target = changedFocus ? focusRequest : previousId;
    const restore = elements.get(target) ?? [...this.root.querySelectorAll('[data-reference]')]
      .find(element => element.dataset.reference === target) ??
      elements.get([...ids.values()].find(({node}) => node.kind === 'button' && node.action === target)?.node.id);
    if (restore) {
      if (!restore.matches('input,textarea,button,a,[tabindex]')) restore.tabIndex = -1;
      restore.focus({preventScroll: !changedFocus});
      if (!changedFocus && selection && restore.setSelectionRange) restore.setSelectionRange(selection.start, selection.end);
    }
    this.lastFocusRequest = focusRequest;
  }

  async complete(request, name, payload) {
    if (this.closed || request.session_id !== this.session) return;
    if (this.cancelledEffects.has(request.request_id)) return;
    const active = this.activeEffects.get(request.request_id);
    if (active?.controller.signal.aborted) return;
    if (name !== 'ui.effect.progress') this.activeEffects.delete(request.request_id);
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
