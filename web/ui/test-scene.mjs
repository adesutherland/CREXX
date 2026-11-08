import assert from 'node:assert/strict';
import {SceneReplica} from './scene.mjs';

const item = {id: 'node-a', kind: 'rect', x: 256, y: 256, w: 512, h: 512, label: 'A'};
const full = {version: 1, kind: 'full', epoch: 'graph', revision: 1, items: [item]};
const delta = {version: 1, kind: 'delta', epoch: 'graph', base: 1, revision: 2,
  changes: [{op: 'replace', item: {...item, x: 512}}],
  viewport: {origin_x: 256, origin_y: 0, ticks_per_pixel: 128, width: 420, height: 260}};
const replica = new SceneReplica();
replica.applyPacket(full);
replica.applyPacket(delta);
const copied = replica.copy();
assert.equal(copied.revision, replica.revision);
assert.equal(replica.revision, 2);
assert.equal(replica.items[0].x, 512);
assert.equal(replica.viewport.ticks_per_pixel, 128);
const broken = {...delta, base: 2, revision: 3,
  changes: [{op: 'remove', id: 'node-a'}, {op: 'remove', id: 'node-a'}]};
assert.throws(() => replica.applyPacket(broken), /Invalid scene change/);
assert.equal(replica.revision, 2);
assert.equal(replica.items[0].id, 'node-a');
const restored = {...full, revision: 3, items: [{...item, x: 768}]};
replica.apply(JSON.stringify(broken), JSON.stringify(restored));
assert.equal(replica.revision, 3);
assert.equal(replica.items[0].x, 768);
assert.throws(() => replica.apply(JSON.stringify(broken), JSON.stringify({...restored, revision: 4})),
  /does not match/);
assert.equal(replica.revision, 3);
assert.throws(() => replica.applyPacket({...full, revision: -1}), /Invalid scene envelope/);
console.log('PASS: browser scene delta, atomic refusal and full resync');
