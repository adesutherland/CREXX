/* Focused native owner-turn check; the Wasm Worker runs the same RXBIN images. */
import assert from 'node:assert/strict';
import {spawn} from 'node:child_process';
import {StringDecoder} from 'node:string_decoder';
import {join, resolve} from 'node:path';

const build = resolve(process.argv[2] ?? 'cmake-build-ui');
const modules = ['text_inspector_web', 'text_inspector', 'ui_web', 'ui_scene',
  'ui_contract', 'ui_catalog', 'ui', 'library'];
const images = modules.map(name => name.startsWith('text_') ?
  join(build, 'examples/ui/text-inspector', `${name}.rxbin`) : join(build, 'bin', `${name}.rxbin`));
const child = spawn(join(build, 'bin/rxvm'), [...images, '-a', 'protocol-test'],
  {stdio: ['pipe', 'pipe', 'pipe']});
const queue = [];
let wake;
let text = '';
const decoder = new StringDecoder('utf8');
child.stdout.on('data', bytes => {
  text += decoder.write(bytes);
  let end;
  while ((end = text.indexOf('\n')) >= 0) {
    const line = text.slice(0, end); text = text.slice(end + 1);
    const frame = JSON.parse(line);
    if (wake) { const resolve = wake; wake = null; resolve(frame); }
    else queue.push(frame);
  }
});
const diagnostics = [];
child.stderr.on('data', bytes => diagnostics.push(String(bytes)));
function next() {
  if (queue.length) return Promise.resolve(queue.shift());
  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error('Timed out waiting for cREXX UI frame')), 5000);
    wake = frame => { clearTimeout(timer); resolve(frame); };
  });
}
function send(type, base, fields = {}) {
  child.stdin.write(JSON.stringify({version: 1, type, session: 'protocol-test', base, ...fields}) + '\n');
}
function completion(request, name, payload) {
  return {version: 0, direction: 'event', name, target: request.target, payload,
    request_id: request.request_id, session_id: request.session_id, sequence: 0};
}
try {
  let view = await next();
  assert.equal(view.type, 'view');
  assert.equal(view.view.nodes.find(node => node.id === 'status').text, 'Status: Ready');
  send('command', view.revision, {command: 'document.open'});
  view = await next();
  let effect = (await next()).message;
  assert.equal(effect.name, 'ui.resource.choose');
  send('cancel', view.revision, {request_id: effect.request_id});
  view = await next();
  assert.equal(view.view.nodes.find(node => node.id === 'status').text, 'Status: Open cancelled');
  send('command', view.revision, {command: 'document.open'});
  view = await next(); effect = (await next()).message;
  const grant = {id: 'test-grant', scope: 'session', display_name: 'fixture.txt',
    readable: true, writable: false};
  send('complete', view.revision, {message: completion(effect, 'ui.resource.selected', {resource: grant})});
  view = await next(); effect = (await next()).message;
  assert.equal(effect.name, 'io.text.read');
  const completed = completion(effect, 'io.text.read.completed',
    {resource: grant, records: ['alpha beta', 'gamma']});
  send('complete', view.revision, {message: completed});
  view = await next();
  assert.equal(view.view.nodes.find(node => node.id === 'words').text, 'Words: 3');
  send('complete', view.revision, {message: completed});
  assert.equal((await next()).result.code, 'invalid_input');
  send('command', 1, {command: 'document.clear'});
  view = await next();
  assert.equal(view.view.nodes.find(node => node.id === 'words').text, 'Words: 3');
  send('command', view.revision, {command: 'document.quit'});
  assert.equal((await next()).type, 'closed');
  child.stdin.end();
  const code = await new Promise(resolve => child.on('exit', resolve));
  assert.equal(code, 0, diagnostics.join(''));
  console.log('PASS: browser UI native owner turns and correlated effects');
} catch (error) {
  child.kill();
  console.error(error, diagnostics.join(''));
  process.exitCode = 1;
}
