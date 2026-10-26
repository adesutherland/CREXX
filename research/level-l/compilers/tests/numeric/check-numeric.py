#!/usr/bin/env python3
"""Independent C17 NUMERIC checks over prepared products; never builds.

One source/lane serves output, borrowed host, fatal and RXAS controls. Shared
NUM-B/target-context tests own stored/VM atomicity; configured resumption,
LOSTDIGITS, TRACE and platform closure remain NUM-C-AC07 work.
"""
import argparse
import json
import hashlib
import os
from pathlib import Path
import re
import runpy
import shlex
import shutil
import subprocess
import tempfile


ADDRESS = runpy.run_path(str(Path(__file__).resolve().parents[1] / 'address' / 'check-address.py'))
called = ADDRESS['called']
unit = ADDRESS['unit']
source_position = ADDRESS['source_position']

# Current shared target contract, including Group 2's configured host adapters.
TARGET_EXPORTS = {'llctarget.' + name for name in (
    'literal', 'poolnew', 'poolread', 'poolwrite', 'binaryop', 'unaryop',
    'truth', 'sayvalue', 'sayconfigured', 'setting', 'digits', 'arguments',
    'mainargs', 'append', 'countargs', 'existsarg', 'present', 'absent',
    'emptyreturn', 'bifarguments', 'missing', 'missingconfigured', 'requirevalue',
    'setresult', 'status', 'lengthvalue', 'failuremessage', 'whentruth',
    'selectmissing', 'donew', 'docapture', 'dostart', 'doentry', 'doadvance',
    'whiletruth', 'untiltruth', 'invalidtransfer', 'parsesource',
    'clausepending',
)}


def exit_bypass(body, source):
    _, line, _ = source_position(source, b'numeric form value ESCAPE()')
    current = None; results = []
    symbol = 'rexxactivation.rexxactivationarguments.programexitrequested'
    for row in body.splitlines():
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            if fields[4] == 'source-c.crexx':
                current = int(fields[5])
        if current == line:
            results.extend(item[1].strip() for item in called(row, symbol))
    assert len(results) == 1, 'NUMERIC operand has one program-root query'
    queries = [item for item in called(body, symbol) if item[1].strip() == results[0]]
    assert len(queries) == 1
    after = body[queries[0].end():]
    branch = re.search(r'^\s*brt\s+([^,\s]+),' + re.escape(results[0]) + r'\s*\n\s*br\s+(\S+)\s*$', after, re.M)
    assert branch and not re.search(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s', after[:branch.start()], re.M), 'query directly controls NUMERIC operand unwind'
    labels = list(re.finditer(r'^(llb\d+):\s*$', body, re.M))
    blocks = {label[1]: body[label.end():labels[i + 1].start() if i + 1 < len(labels) else len(body)] for i, label in enumerate(labels)}
    assert branch[1] in blocks and branch[2] in blocks and branch[1] != branch[2]
    exiting = blocks[branch[1]]; continuing = blocks[branch[2]]
    assert not called(exiting, 'llctarget.setting') and not re.search(r'^\s*setnum(?:dgts|fuz|frm)\s', exiting, re.M), 'operand EXIT bypasses setter and same-frame install'
    assert re.search(r'^\s*ret\s+r\d+\s*$', exiting, re.M), 'true edge returns owned packet after private cleanup'
    assert len(called(continuing, 'llctarget.setting')) == 1, 'false continuation alone changes numeric setting'


def shapes(text, source, target):
    body = unit(text); settings = called(body, 'llctarget.setting')
    assert settings, 'NUMERIC selects existing sole shared-setting adapter'
    for index, setting in enumerate(settings):
        following = body[setting.end():settings[index + 1].start() if index + 1 < len(settings) else len(body)]
        installed = re.search(r'^\s*setnumfuz\s+0\s*\n\s*setnumdgts\s+(r\d+)\s*\n\s*setnumfuz\s+(r\d+)\s*\n\s*setnumfrm\s+(r\d+)\s*$', following, re.M)
        assert installed, 'successful setter precedes FUZZ0/DIGITS/FUZZ/FORM in executing C frame'
        prefix = following[:installed.start()]
        getters = [called(prefix, 'rexxactivation.rexxactivationarguments.' + name) for name in ('numericdigits', 'numericfuzz', 'numericform')]
        assert all(len(items) == 1 for items in getters), 'one stored integer read per installed setting'
        assert [items[0][1].strip() for items in getters] == list(installed.groups()), 'same validated integer results feed the frame install'
        assert not re.search(r'^\s*(?:br\w*|b(?:eq|ne|lt|le|gt|ge)|signal\w*|ret|exit)\s', prefix, re.M), 'frame install is on normal setting-return path'
    _, dynamic_line, _ = source_position(source.read_bytes(), b'numeric form value MAKEFORM()')
    current = None; dynamic_settings = 0; dynamic_literals = 0
    for row in body.splitlines():
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            if fields[4] == 'source-c.crexx':
                current = int(fields[5])
        if current == dynamic_line:
            dynamic_settings += bool(called(row, 'llctarget.setting'))
            dynamic_literals += bool(called(row, 'llctarget.literal'))
    assert (dynamic_settings, dynamic_literals) == (1, 0), 'dynamic FORM carries its V once without reconstructing a text literal'
    assert not called(body, 'rexxclassicconfig.rexxclassicconfig.§factory'), 'no per-statement numeric configuration construction'
    assert not called(body, 'llctarget.status') and not re.search(r'^\s*exit\s', body, re.M), 'source frame preserves full owned completion'
    definitions = re.findall(r'^\S+\([^\n]*\s\.locals=\d+[^\n]*\.expose=(\S+)\s*$', target, re.M)
    assert len(definitions) == len(set(definitions)), 'unique target exports'
    assert set(definitions) == TARGET_EXPORTS, ('current shared target export surface', definitions)
    assert not re.search(r'^(?:bad|bifcontext|lostdigitscheck)\([^\n]*\s\.locals=\d+[^\n]*\.expose=', target, re.M), 'shared target implementation helpers remain private'
    data = source.read_bytes(); needle = "if 'é' = 'é' then numeric form value,".encode()
    offset, line, column = source_position(data, needle)
    numeric = data.index(b'numeric form value', offset)
    column += len(data[offset:numeric].decode('utf-8'))
    assert re.search(r'^\s*\.srcstep\s+\d+\s+\d+\s+(?:17|65)\s+"source-c\.crexx"\s+' + rf'{line}\s+{column}\s+\d+\s+"', body, re.M), 'continued NUMERIC after Unicode retains authored scalar columns'
    exit_bypass(body, source.read_bytes())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
    parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
    parser.add_argument('--ir', choices=('on', 'off'), default='on')
    parser.add_argument('--phase', choices=('all', 'emit', 'source', 'host', 'fatal', 'refusals', 'contracts', 'shapes'), default='all')
    parser.add_argument('--inspect', type=Path)
    args = parser.parse_args()
    emit = args.phase == 'emit'
    if emit and args.inspect is None:
        parser.error('--phase emit requires --inspect')
    build = args.build.resolve(); fixtures = Path(__file__).resolve().parent
    backend = build / 'research/level-l/compilers/backend' / args.mode
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    args.work.mkdir(parents=True, exist_ok=True); commands = 0
    with tempfile.TemporaryDirectory(prefix='numeric-', dir=args.work) as temporary:
        work = Path(temporary); runtime = work / 'runtime'; runtime.mkdir()
        for name in libraries:
            root = backend if name == 'llctarget' else build / 'bin'
            shutil.copy2(root / (name + '.rxbin'), runtime / (name + '.rxbin'))
        dlls = tuple((build / 'bin').glob('*.dll')) if os.name == 'nt' else ()

        def tool(name):
            return build / 'bin' / (name + suffix)

        def stage(name, plugins):
            directory = work / name; directory.mkdir(); vm = directory / (args.vm + suffix)
            shutil.copy2(tool(args.vm), vm)
            for dll in dlls:
                shutil.copy2(dll, directory / dll.name)
            providers = directory / 'providers'; providers.mkdir()
            for plugin in plugins:
                filename = plugin + '.rxplugin'
                source = build / 'bin' / filename
                if not source.exists():
                    source = build / 'bin' / 'providers' / filename
                shutil.copy2(source, providers / filename)
            return directory, vm

        host, host_vm = stage('C compiler host', ('rxfs', 'rxcontract'))
        application, target_vm = stage('static application', ('rxfs', 'rxcstream'))
        compiler = host / 'llrxc-c-linked.rxbin'; shutil.copy2(backend / compiler.name, compiler)
        empty_path = work / 'empty PATH'; empty_path.mkdir()
        env = dict(os.environ, PATH=str(empty_path)); env.pop('CREXX_PROVIDER_PATH', None)
        assert not any(shutil.which(name, path=env['PATH']) for name in ('rxc', 'rxc-c'))

        def run(label, command, *, failure=False, silent=False, cwd=work):
            nonlocal commands
            completed = subprocess.run([str(item) for item in command], cwd=cwd, env=env, capture_output=True, timeout=900)
            commands += 1; diagnostic = completed.stdout + completed.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
            assert (completed.returncode != 0 if failure else completed.returncode == 0), (label, completed.returncode, diagnostic[-6000:])
            if silent:
                assert not diagnostic, (label, diagnostic[-4000:])
            return completed

        def compile_c(source, output, *, failure=False):
            emission_start(source)
            return run('compile ' + source.name, [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []),
                       '-i', runtime, '-o', output, source], failure=failure, silent=not failure, cwd=host)

        def inspect(path, name):
            if args.inspect and not emit:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, args.inspect / f'{name}-{args.mode}-{args.ir}-{args.vm}.rxas')

        # Emit-only capture stays local to this recipe and never replaces a tool.
        def emission_hash(path):
            return hashlib.sha256(path.read_bytes()).hexdigest()

        if emit:
            emission_owner = 'numeric'
            emission_dir = args.inspect.resolve() / f'{emission_owner}-{args.mode}-{args.ir}-{args.vm}'
            emission_dir.mkdir(parents=True)  # Refuse reuse of an earlier collection.
            emission_records = []
            emission_derived_records = []
            emission_started = []

        def emission_start(source):
            if emit:
                case = source.stem
                assert case not in emission_started, ('duplicate request identity', case)
                emission_started.append(case)
                print(f'START|{emission_owner}.{case}')

        def emission_finish(source, output, completed, *, refused=False):
            case = source.stem
            assert case in emission_started and not any(row['case'] == case for row in emission_records)
            assert (completed.returncode != 0) == refused
            data = source.read_bytes()
            (emission_dir / (case + '.source')).write_bytes(data)
            (emission_dir / (case + '.stdout')).write_bytes(completed.stdout)
            (emission_dir / (case + '.stderr')).write_bytes(completed.stderr)
            record = dict(case=case, logical=source.name, source_sha256=hashlib.sha256(data).hexdigest(),
                          argv=completed.args, status=completed.returncode,
                          stdout_sha256=hashlib.sha256(completed.stdout).hexdigest(),
                          stderr_sha256=hashlib.sha256(completed.stderr).hexdigest(),
                          disposition='REFUSAL' if refused else 'EMITTED')
            if refused:
                # The owner has already checked exact diagnostics and the prior-output sentinel.
                (emission_dir / (case + '.previous')).write_bytes(output.read_bytes())
                record['previous_sha256'] = emission_hash(output)
            else:
                assert output.stat().st_size > 0
                shutil.copyfile(output, emission_dir / (case + '.rxas'))
                record['rxas_sha256'] = emission_hash(output)
            emission_records.append(record)
            print(f"{record['disposition']}|{emission_owner}.{case}")

        def emission_derived(base, exported):
            assert len(emission_derived_records) == 0
            assert emission_hash(base) == next(row['rxas_sha256'] for row in emission_records if row['disposition'] == 'EMITTED')
            shutil.copyfile(exported, emission_dir / 'entry-export.rxas')
            emission_derived_records.append(dict(case='entry-export', base_sha256=emission_hash(base),
                                                 rxas_sha256=emission_hash(exported), transform='existing-entry-export'))
            print(f'DERIVED|{emission_owner}.entry-export')

        def export_entry(text):
            exported = application / 'source-exported.rxas'
            text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=numericfixture.run\2', text, count=1, flags=re.M)
            text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"numericfixture.run"', text, count=1, flags=re.M)
            assert headers == metadata == 1, 'one test-only exported entry0, no production ABI change'
            exported.write_text(text, encoding='utf-8'); inspect(exported, 'numeric-exported')
            return exported

        if args.phase in ('all', 'emit', 'source', 'host', 'fatal', 'shapes'):
            source = work / 'source-c.crexx'; data = (fixtures / source.name).read_bytes()
            source.write_bytes(data if args.mode == 'opt' else data.replace(b'\n', b'\r\n'))
            assembly = application / 'source.rxas'; compiled = compile_c(source, assembly)
            inspect(assembly, 'numeric-source'); text = assembly.read_text()
            assert str(work) not in text and str(build) not in text, 'stable logical source identity'
            shapes(text, source, (backend / 'llctarget.rxas').read_text())
            if emit:
                emission_finish(source, assembly, compiled)
                emission_derived(assembly, export_entry(text))
            if args.phase in ('all', 'source'):
                run('source assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), assembly], silent=True)
                linked = application / 'source-linked'
                run('source link', [tool('rxlink'), '-o', linked, assembly.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)], silent=True)
                completed = run('NUMERIC source', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
                assert completed.stdout.replace(b'\r\n', b'\n') == expected and not completed.stderr, (completed.stdout, completed.stderr, expected)
            if args.phase in ('all', 'host', 'fatal'):
                exported = export_entry(text)
                run('exported assemble', [tool('rxas'), '-n', exported], silent=True)
                imports = work / 'B caller imports'; imports.mkdir()
                for provider in runtime.glob('*.rxbin'):
                    shutil.copy2(provider, imports / provider.name)
                shutil.copy2(exported.with_suffix('.rxbin'), imports / 'numericfixture.rxbin')
                caller = application / 'numeric-host'
                run('B host compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import', '-i', imports, '-o', caller, fixtures / 'host.crexx'], silent=True)
                run('B host assemble', [tool('rxas'), '-n', caller], silent=True)
                linked = application / 'numeric-host-linked'; control = work / 'host.ctl'
                inputs = [caller.with_suffix('.rxbin'), exported.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) + 'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run('B host link', [tool('rxlink'), '-c', control], silent=True)
                if args.phase in ('all', 'host'):
                    completed = run('borrowed NUMERIC state/frame/EXIT', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                    assert re.fullmatch(rb'NUMERIC_HOST\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
                if args.phase in ('all', 'fatal'):
                    rows = source.read_text().splitlines()
                    cases = (('invalid-form', b'33.3', rows.index('  numeric form value BADFORM()') + 1, b'NUMERIC_ACTUAL|1\n'),
                             ('reset-digits', b'33.1', rows.index('  numeric digits') + 1, b''))
                    for mode, code, line, actual in cases:
                        completed = run('unhandled ' + mode, [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', mode], failure=True, cwd=application)
                        assert completed.stdout.replace(b'\r\n', b'\n') == b'NUMERIC_FATAL|before\n' + actual, (mode, completed.stdout)
                        assert b'CLASSIC_SYNTAX' in completed.stderr and code in completed.stderr and f'source-c.crexx:{line}:'.encode() in completed.stderr, (mode, completed.stderr)
                        assert b'UNREACHED' not in completed.stdout

        if args.phase in ('all', 'emit', 'refusals'):
            cases = (('missing-setting', 'numeric', 'numeric', b'25.15'),
                     ('bad-form', 'numeric form bogus', 'bogus', b'25.11'),
                     ('dead-value', 'if 0 then numeric form value', 'value', b'35.1'),
                     ('unicode-operator', "if '雪' = '雪' then numeric fuzz 1 +", '+', b'35.1'))
            output = application / 'prior.rxas'; prior = b'unchanged prior output\n'; output.write_bytes(prior)
            for name, content, token, code in cases:
                source = work / (name + '.crexx'); data = ('options levelc\n' + content + '\n').encode(); source.write_bytes(data)
                completed = compile_c(source, output, failure=True)
                offset, line, column = source_position(data, token.encode())
                assert code in completed.stdout and not completed.stderr, (name, completed.stdout, completed.stderr)
                assert f'at line {line}, column {column} (byte {offset})'.encode() in completed.stdout, (name, completed.stdout)
                assert output.read_bytes() == prior and not Path(str(output) + '.llrxc-stage').exists(), 'failed NUMERIC publication remains atomic'
                if emit: emission_finish(source, output, completed, refused=True)

        if args.phase in ('all', 'contracts'):
            probe = host / 'numeric-contracts-linked.rxbin'; shutil.copy2(backend / probe.name, probe)
            completed = run('NUMERIC AST/IR/source contracts', [host_vm, '--no-autoload', probe, '-a', runtime], cwd=host)
            assert re.fullmatch(rb'NUMERIC_CONTRACTS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
        if emit:
            assert len(emission_records) == 4 + 1 and len(emission_started) == 4 + 1
            assert sum(row['disposition'] == 'EMITTED' for row in emission_records) == 1
            assert sum(row['disposition'] == 'REFUSAL' for row in emission_records) == 4
            assert len(emission_derived_records) == 1
            identity = dict(kind='EMISSION_ONLY', owner=emission_owner, mode=args.mode,
                            producer_vm=args.vm, semantic_ir=args.ir, dialect='C',
                            adapter_sha256=emission_hash(Path(__file__)),
                            entry_sha256=emission_hash(fixtures / ('check-' + emission_owner + '.py')),
                            compiler_sha256=emission_hash(compiler), vm_sha256=emission_hash(host_vm),
                            shape_inputs={'llctarget.rxas': emission_hash(backend / 'llctarget.rxas')},
                            imports={path.name: emission_hash(path) for path in sorted(runtime.glob('*.rxbin'))},
                            providers={path.name: emission_hash(path) for path in sorted((host / 'providers').glob('*')) if path.is_file()},
                            host_libraries={path.name: emission_hash(path) for path in sorted(host.glob('*.dll'))},
                            bindings=[], routine_export='', host_export='',
                            limits=dict(frontend_bytes='1000000', nesting='512', output_bytes='67108864', registers='65536'),
                            requests=emission_records, derived=emission_derived_records)
            (emission_dir / 'identity.json').write_text(json.dumps(identity, indent=2) + '\n', encoding='utf-8')
            print(f'{emission_owner.upper()}_EMISSION|mode={args.mode}|vm={args.vm}|ir={args.ir}|requests={len(emission_records)}|derived=1|EMISSION')
            return
    print(f'NUMERIC_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
