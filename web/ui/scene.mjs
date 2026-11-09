/* Version-1 scene projection only: graph and map rules stay in cREXX. */
const LIMIT = 2 ** 40;
const validInt = (value, limit = LIMIT) => Number.isSafeInteger(value) && Math.abs(value) <= limit;
const validViewport = value => value &&
  validInt(value.origin_x) && validInt(value.origin_y) &&
  validInt(value.ticks_per_pixel, 65536) && value.ticks_per_pixel > 0 &&
  validInt(value.width, 1000000) && value.width > 0 &&
  validInt(value.height, 1000000) && value.height > 0;
const defaultViewport = {origin_x: 0, origin_y: 0, ticks_per_pixel: 256, width: 420, height: 260};
function validItem(item) {
  return item && typeof item.id === 'string' && item.id.length > 0 && item.id.length <= 128 &&
    ['rect', 'edge', 'tile', 'unit'].includes(item.kind) &&
    typeof item.label === 'string' && item.label.length <= 4096 &&
    validInt(item.x) && validInt(item.y) && validInt(item.w) && validInt(item.h) &&
    item.w >= 0 && item.h >= 0;
}

export class SceneReplica {
  constructor() { this.epoch = ''; this.revision = 0; this.items = []; this.viewport = defaultViewport; }
  copy() {
    const result = new SceneReplica();
    result.epoch = this.epoch;
    result.revision = this.revision;
    result.items = this.items.slice();
    result.viewport = {...this.viewport};
    return result;
  }
  apply(packetText, fullText) {
    const packet = JSON.parse(packetText);
    try { this.applyPacket(packet); }
    catch {
      const full = JSON.parse(fullText);
      if (full.kind !== 'full' || full.epoch !== packet.epoch || full.revision !== packet.revision)
        throw new Error('Scene resync does not match current frame');
      this.applyPacket(full, true);
    }
  }
  applyPacket(packet, authoritative = false) {
    if (!packet || packet.version !== 1 || typeof packet.epoch !== 'string' ||
        !packet.epoch || packet.epoch.length > 64 || !validInt(packet.revision, Number.MAX_SAFE_INTEGER) ||
        packet.revision < 0)
      throw new Error('Invalid scene envelope');
    let items;
    if (packet.kind === 'full') {
      if (!Array.isArray(packet.items) || packet.items.length > 512 ||
          packet.items.some(item => !validItem(item))) throw new Error('Invalid scene snapshot');
      items = packet.items;
    } else if (packet.kind === 'delta' && !authoritative) {
      if (packet.epoch !== this.epoch || packet.base !== this.revision ||
          packet.revision !== packet.base + 1 || !Array.isArray(packet.changes) ||
          packet.changes.length > 128) throw new Error('Scene revision gap');
      items = this.items.slice();
      for (const change of packet.changes) {
        const index = items.findIndex(item => item.id === (change.id ?? change.item?.id));
        if (change.op === 'remove' && index >= 0) items.splice(index, 1);
        else if (change.op === 'add' && index < 0 && validItem(change.item)) items.push(change.item);
        else if (change.op === 'replace' && index >= 0 && validItem(change.item)) items[index] = change.item;
        else throw new Error('Invalid scene change');
      }
    } else throw new Error('Invalid scene kind');
    if (new Set(items.map(item => item.id)).size !== items.length || items.length > 512)
      throw new Error('Duplicate scene ID');
    const viewport = packet.viewport ?? (packet.kind === 'full' ? defaultViewport : this.viewport);
    if (!validViewport(viewport)) throw new Error('Invalid scene viewport');
    this.epoch = packet.epoch;
    this.revision = packet.revision;
    this.items = items;
    this.viewport = viewport;
  }
}

export function renderScene(node, replica, input) {
  const {width, height, origin_x, origin_y, ticks_per_pixel: scale} = replica.viewport;
  const toX = x => (x - origin_x) / scale;
  const toY = y => (y - origin_y) / scale;
  const isMap = replica.items.some(item => item.kind === 'tile' || item.kind === 'unit');
  const element = isMap ? document.createElement('canvas') :
    document.createElementNS('http://www.w3.org/2000/svg', 'svg');
  element.dataset.nodeId = node.id;
  element.setAttribute('role', 'img');
  element.setAttribute('aria-label', isMap ? 'Tile Map Demo' : 'Editable graph');
  element.tabIndex = 0;
  element.style.width = `${width}px`;
  element.style.height = `${height}px`;
  element.style.border = '1px solid #8b9caf';
  if (isMap) {
    const ratio = window.devicePixelRatio || 1;
    element.width = Math.ceil(width * ratio);
    element.height = Math.ceil(height * ratio);
    const draw = () => {
      const context = element.getContext('2d');
      if (!context) throw new Error('Map canvas context unavailable');
      context.setTransform(ratio, 0, 0, ratio, 0, 0);
      context.clearRect(0, 0, width, height);
      for (const item of replica.items) {
        const x = toX(item.x), y = toY(item.y), w = item.w / scale, h = item.h / scale;
        if (x + w < 0 || y + h < 0 || x > width || y > height) continue;
        context.fillStyle = item.kind === 'tile' ? '#bed9ad' : '#dcad74';
        context.fillRect(x, y, w, h);
        context.strokeStyle = '#40536c'; context.strokeRect(x, y, w, h);
        context.fillStyle = '#172336'; context.fillText(item.label, x + 4, y + 17);
      }
    };
    element.addEventListener('contextlost', event => event.preventDefault());
    element.addEventListener('contextrestored', draw);
    draw();
  } else {
    element.setAttribute('viewBox', `0 0 ${width} ${height}`);
    for (const item of replica.items) {
      const isEdge = item.kind === 'edge';
      const shape = document.createElementNS('http://www.w3.org/2000/svg', isEdge ? 'line' : 'rect');
      if (isEdge) {
        shape.setAttribute('x1', toX(item.x)); shape.setAttribute('y1', toY(item.y));
        shape.setAttribute('x2', toX(item.x + item.w)); shape.setAttribute('y2', toY(item.y + item.h));
        shape.setAttribute('stroke', '#40536c'); shape.setAttribute('stroke-width', '2');
      } else {
        shape.setAttribute('x', toX(item.x)); shape.setAttribute('y', toY(item.y));
        shape.setAttribute('width', item.w / scale); shape.setAttribute('height', item.h / scale);
        shape.setAttribute('fill', '#b3cfef'); shape.setAttribute('stroke', '#40536c');
      }
      shape.dataset.itemId = item.id;
      element.append(shape);
      if (item.label) {
        const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        label.setAttribute('x', toX(item.x) + 4); label.setAttribute('y', toY(item.y) + 17);
        label.textContent = item.label;
        element.append(label);
      }
    }
  }
  element.addEventListener('pointerdown', event => {
    const bounds = element.getBoundingClientRect();
    const x = Math.floor(event.clientX - bounds.left), y = Math.floor(event.clientY - bounds.top);
    if (x >= 0 && y >= 0 && x < width && y < height)
      input({type: 'pointer', node: node.id, phase: 'press', pointer: {
        version: 1, kind: 'pointer', epoch: replica.epoch, revision: replica.revision, x, y,
      }});
    element.focus();
  });
  element.addEventListener('keydown', event => {
    input({type: 'key', node: node.id, epoch: replica.epoch,
      revision: replica.revision, key: event.key});
    if (['ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown', 'Delete'].includes(event.key))
      event.preventDefault();
  });
  return element;
}
