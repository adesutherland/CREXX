/* Preflight and launch a trusted GTK or ANSI UI package from a build/install tree. */
import {readFile} from 'node:fs/promises';
import {spawn} from 'node:child_process';
import {join, resolve} from 'node:path';
import {preflightPackage, CORE_MODULES} from './package-registry.mjs';

const [filename, rootName, route, mode] = process.argv.slice(2);
if (!filename || !rootName || !['gtk', 'ansi'].includes(route) ||
    (mode && mode !== '--check')) {
  console.error('Usage: node run-package.mjs PACKAGE_JSON BUILD_OR_INSTALL_ROOT gtk|ansi [--check]');
  process.exit(2);
}
const pkg = JSON.parse(await readFile(resolve(filename), 'utf8'));
const checked = preflightPackage(pkg, route);
const root = resolve(rootName);
const paths = [checked.selected.entry, ...[...pkg.modules].reverse()].map(record => join(root, record.path));
paths.push(...CORE_MODULES[route].map(name => join(root, 'bin', `${name}.rxbin`)));
for (const path of [join(root, 'bin/rxvm'), ...paths]) {
  if (!(await readFile(path)).length) throw new Error(`Missing required UI asset: ${path}`);
}
if (mode === '--check') {
  console.log(`PASS: ${pkg.id} ${route} package preflight`);
} else {
  const child = spawn(join(root, 'bin/rxvm'), paths, {stdio: 'inherit'});
  child.on('error', error => { console.error(error); process.exitCode = 1; });
  child.on('close', (code, signal) => { process.exitCode = signal ? 1 : code; });
}
