import assert from 'node:assert/strict';
import {HostedRoute} from './hosted.mjs';

const route = new HostedRoute('test-token');
route.session = 'hosted-test';
let attempts = 0;
let deleted = '';
const refusals = [];
route.onFrame = frame => refusals.push(frame);
route.poll = async () => { attempts++; throw new Error('synthetic transport loss'); };
route.request = async (path, init) => {
  assert.equal(init.method, 'DELETE');
  deleted = path;
  return {closed: true};
};
await route.loop();
assert.equal(attempts, 3);
assert.equal(refusals.length, 1);
assert.equal(refusals[0].result.code, 'transport');
assert.equal(deleted, '/api/sessions/hosted-test');
assert.equal(route.stopped, true);

const malformed = new HostedRoute('test-token');
malformed.session = 'hosted-malformed';
const frames = [];
let releases = 0;
malformed.onFrame = frame => {
  if (frame.type === 'view') throw new Error('bad projected view');
  frames.push(frame);
};
malformed.request = async (_path, init) => {
  if (init?.method === 'DELETE') { releases++; return {closed: true}; }
  return {frames: [{sequence: 1, frame: {type: 'view'}}]};
};
await malformed.loop();
assert.equal(frames.length, 1);
assert.equal(frames[0].result.code, 'invalid_frame');
assert.equal(releases, 1);
console.log('PASS: hosted UI bounded transport retry and session release');
