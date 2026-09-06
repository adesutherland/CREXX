/* Optional client-contract qualification; no Node dependency in product builds.
 * node CheckClient.cjs BUILD/lib/ui/contracts DEPS/node_modules
 * DEPS contains ajv 8 and TypeScript 5.9 (or compatible newer releases).
 */
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const [generatedArgument, dependencyArgument] = process.argv.slice(2);
if (!generatedArgument || !dependencyArgument) throw new Error('generated and dependency directories required');
const generated = path.resolve(generatedArgument);
const dependencies = path.resolve(dependencyArgument);
const Ajv = require(require.resolve('ajv/dist/2020', { paths: [dependencies] }));
const ts = require(require.resolve('typescript', { paths: [dependencies] }));
const schema = JSON.parse(fs.readFileSync(path.join(generated, 'ui-contract.schema.json'), 'utf8'));
const validate = new Ajv({ strict: true, allErrors: true }).compile(schema);
const fixtures = fs.readFileSync(path.join(__dirname, 'conformance.jsonl'), 'utf8').trim().split('\n').map(JSON.parse);
for (const fixture of fixtures) {
  assert.equal(validate(fixture.message), fixture.valid, `${fixture.case}: ${JSON.stringify(validate.errors)}`);
}

// Exercise the actual generated declarations with an in-memory client. Invalid
// calls must fail type checking; an unused @ts-expect-error is itself a failure.
const binding = path.join(generated, 'ui-contract.ts');
const probe = path.join(generated, 'ui-contract-type-probe.ts');
const source = `import { uiEvent, uiEffect, type UIMessage } from './ui-contract';
const input = uiEvent('ui.input.key', {key:'Enter', phase:'press', modifiers:[]});
const name: 'ui.input.key' = input.name;
const close = uiEffect('ui.close', {});
const event: UIMessage = input;
if (event.name === 'ui.input.key') { const key: string = event.payload.key; }
// @ts-expect-error unknown phase
uiEvent('ui.input.key', {key:'Enter', phase:'unknown', modifiers:[]});
// @ts-expect-error missing required field
uiEvent('ui.input.key', {key:'Enter', phase:'press'});
// @ts-expect-error empty payload still forbids unknown fields
uiEvent('ui.ready', {extra:true});
// @ts-expect-error event is not an effect
uiEffect('ui.ready', {});
// @ts-expect-error unknown standard message
uiEvent('ui.invented', {});
// @ts-expect-error text is not a number
uiEffect('ui.timer.schedule', {delay_ms:'1'});
`;
const options = { strict: true, noEmit: true, target: ts.ScriptTarget.ES2020,
  module: ts.ModuleKind.CommonJS, moduleResolution: ts.ModuleResolutionKind.Node10 };
const host = ts.createCompilerHost(options);
const getSourceFile = host.getSourceFile.bind(host);
const fileExists = host.fileExists.bind(host);
host.getSourceFile = (file, languageVersion, ...rest) => file === probe
  ? ts.createSourceFile(file, source, languageVersion, true)
  : getSourceFile(file, languageVersion, ...rest);
host.fileExists = file => file === probe || fileExists(file);
const program = ts.createProgram([binding, probe], options, host);
const diagnostics = ts.getPreEmitDiagnostics(program);
if (diagnostics.length) {
  throw new Error(ts.formatDiagnosticsWithColorAndContext(diagnostics, {
    getCurrentDirectory: () => generated, getCanonicalFileName: x => x, getNewLine: () => '\n'
  }));
}
console.log(`PASS: UI JSON Schema (${fixtures.length} shared fixtures) and TypeScript client contracts`);
