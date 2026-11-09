import assert from 'node:assert/strict';
import {graphRepository} from './graph-repository.mjs';
import {mapRepository} from './map-repository.mjs';

const store = new Map();
const storage = {getItem: key => store.get(key) ?? null,
  setItem: (key, value) => store.set(key, value)};
const responses = [];
const complete = (request, name, payload) => responses.push({name, payload});
const graph = graphRepository(storage);
const map = mapRepository(storage);
const snapshot = JSON.stringify({format: 1, model: {created: 1}, undo: [], redo: []});
const record = JSON.stringify({turn: 2, unit_x: 20480});

assert.equal(await graph({name: 'graph.repository.load'}, complete), 1);
assert.equal(responses.pop().payload.code, 'not_found');
await graph({name: 'graph.repository.save', payload: {expected_revision: 0, snapshot}}, complete);
assert.deepEqual(responses.pop(), {name: 'graph.repository.saved', payload: {revision: 1}});
await graph({name: 'graph.repository.save', payload: {expected_revision: 0, snapshot}}, complete);
assert.equal(responses.pop().payload.code, 'conflict');
await graph({name: 'graph.repository.load'}, complete);
assert.deepEqual(responses.pop(), {name: 'graph.repository.loaded', payload: {revision: 1, snapshot}});
assert.equal(await graph({name: 'map.journal.load'}, complete), false);

await map({name: 'map.journal.load'}, complete);
assert.equal(responses.pop().payload.code, 'not_found');
await map({name: 'map.journal.append', payload: {expected_revision: 0, record}}, complete);
assert.deepEqual(responses.pop(), {name: 'map.journal.appended', payload: {revision: 1}});
await map({name: 'map.journal.append', payload: {expected_revision: 0, record}}, complete);
assert.equal(responses.pop().payload.code, 'conflict');
await map({name: 'map.journal.load'}, complete);
assert.deepEqual(responses.pop(), {name: 'map.journal.loaded', payload: {revision: 1, record}});
await map({name: 'map.journal.append', payload: {expected_revision: 1,
  record: JSON.stringify({turn: 4, unit_x: 53248})}}, complete);
assert.equal(responses.pop().payload.code, 'turn_sequence');
assert.equal(store.size, 2);
console.log('PASS: independent graph and map repositories');
