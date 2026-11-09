/* Asyncify suspends the VM only while its single owner reads stdin. */
const common = ['library', 'ui', 'ui_catalog', 'ui_contract', 'ui_scene', 'ui_web'];
const applications = {
  text: ['text_inspector', 'text_inspector_web'],
  graph: ['scene_editors', 'graph_web'],
  map: ['scene_editors', 'map_web'],
  process: ['process_designer', 'process_web'],
  probe: ['web_service_probe'],
};
let vm, started = false, done = false;
const queue = [];
let wake;
let output = '';
const decoder = new TextDecoder('utf-8', {fatal: true});
const encoder = new TextEncoder();

function read(capacity) {
  if (queue.length) return Promise.resolve(take(capacity));
  if (done) return Promise.resolve(null);
  return new Promise(resolve => { wake = {resolve, capacity}; });
}
function take(capacity) {
  const bytes = queue.shift();
  if (bytes.length > capacity) {
    queue.unshift(bytes.subarray(capacity));
    return bytes.subarray(0, capacity);
  }
  return bytes;
}
function push(bytes) {
  if (bytes.length > 262145) throw new Error('UI input exceeds limit');
  queue.push(bytes);
  if (wake) { const {resolve, capacity} = wake; wake = null; resolve(take(capacity)); }
}
function write(fd, bytes) {
  if (fd === 2) {
    self.postMessage({type: 'failure', message: decoder.decode(bytes)});
    return;
  }
  output += decoder.decode(bytes, {stream: true});
  if (output.length > 524288) throw new Error('UI output exceeds limit');
  let end;
  while ((end = output.indexOf('\n')) >= 0) {
    const line = output.slice(0, end);
    output = output.slice(end + 1);
    if (line) self.postMessage({type: 'frame', frame: JSON.parse(line)});
  }
}
function check(code) {
  if (code !== 0) throw new Error(vm.ccall('ui_vm_error', 'string', [], []));
}
function copy(bytes) {
  const pointer = vm._malloc(bytes.length);
  if (!pointer) throw new Error('Cannot allocate RXBIN transfer');
  vm.HEAPU8.set(bytes, pointer);
  return pointer;
}

self.onmessage = async ({data}) => {
  try {
    if (data?.type === 'input' && started && !done) {
      if (!data.input || data.input.version !== 1) throw new Error('Invalid UI input');
      push(encoder.encode(JSON.stringify(data.input) + '\n'));
      return;
    }
    if (data?.type !== 'start' || started || typeof data.session !== 'string' ||
        !Object.hasOwn(applications, data.app))
      throw new Error('Invalid UI Worker start');
    started = true;
    const {default: createVM} = await import('./vm/crexx_ui_vm.mjs');
    vm = await createVM({noInitialRun: true,
      locateFile: name => new URL(`./vm/${name}`, import.meta.url).href,
      uiRead: read, uiWrite: write});
    check(vm._ui_vm_create());
    for (const name of [...common, ...applications[data.app]]) {
      const response = await fetch(new URL(`./modules/${name}.rxbin`, import.meta.url));
      if (!response.ok) throw new Error(`Missing RXBIN: ${name}`);
      const bytes = new Uint8Array(await response.arrayBuffer());
      const pointer = copy(bytes);
      try { check(vm._ui_vm_load(pointer, bytes.length)); }
      finally { vm._free(pointer); }
    }
    const code = await vm.ccall('ui_vm_run', 'number', ['string'], [data.session], {async: true});
    check(code);
    if (vm._ui_vm_program_rc() !== 0) throw new Error('UI program failed');
    check(vm._ui_vm_destroy());
    done = true;
    self.close();
  } catch (error) {
    done = true;
    self.postMessage({type: 'failure', message: String(error)});
    self.close();
  }
};
