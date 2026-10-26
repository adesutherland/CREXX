#!/usr/bin/env python3
"""Independent EXIT checks over prepared products, with disposable outputs.

One C compilation per lane serves source execution, readable RXAS and a
direct B caller. The caller checks full results and ownership after source
unit return; a VM process exit cannot satisfy those controls. No builds run.
"""
import argparse
import json
import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


def named_calls(text, symbol):
    return list(re.finditer(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s+([^,\n]+),§?' +
                           re.escape(symbol) + r'\(\)(?:,|\s*$)', text, re.M))


def unit(text):
    headers = list(re.finditer(r'^llf\d+\(\) \.locals=', text, re.M))
    assert len(headers) == 1 and headers[0].group() == 'llf1() .locals=', 'one shared C source-unit body'
    start = headers[0].start()
    finish = text.index('llerror1_1:', start)
    body = text[start:finish]
    assert 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int' in body
    return text[:start], body


def source_position(data, needle):
    offset = data.index(needle)
    line = data[:offset].count(b'\n') + 1
    first = data.rfind(b'\n', 0, offset) + 1
    column = len(data[first:offset].decode('utf-8')) + 1
    return offset, line, column


def source_records(text):
    return list(re.finditer(
        r'^[ \t]*\.srcstep[ \t]+(\d+)[ \t]+(\d+)[ \t]+(\d+)[ \t]+'
        r'("[^"\r\n]*")[ \t]+(\d+)[ \t]+(\d+)[ \t]+(\d+)[ \t]+("[^"\r\n]*")[ \t]*$', text, re.M))


def guard_shapes(body):
    """Follow emitted data, without depending on allocated register numbers."""
    calls = list(re.finditer(r'^\s*call\s+([^,\n]+),llf1\(\),[^\n]+$', body, re.M))
    queries = named_calls(body, 'rexxactivation.rexxactivationarguments.programexitrequested')
    assert calls and len(calls) == len(queries), 'one existing root query per internal call'
    records = source_records(body)

    def source_at(offset):
        preceding = [record for record in records if record.end() <= offset]
        assert preceding, 'root guard instruction has an active source record'
        return preceding[-1]

    slots = set(); query_steps = set()
    for call, query in zip(calls, queries):
        assert call.end() < query.start(), 'query follows its invoking call'
        authored = source_at(call.start()); derived = source_at(query.start())
        assert int(authored.group(3)) & 1, 'invoking call has authored provenance'
        assert derived.group(3) in ('34', '98'), 'root query has generated inherited coordinates'
        assert int(derived.group(2)) > 0 and derived.group(2) == authored.group(2), 'root query retains its invoking clause'
        assert derived.groups()[3:] == authored.groups()[3:], 'root query retains its invoking call source range and text'
        assert all(int(derived.group(field)) > 0 for field in (5, 6, 7)), 'root query has positive source coordinates'
        step = int(derived.group(1))
        assert step not in query_steps, 'each root query has a distinct presentation step'
        query_steps.add(step)
        transport = body[call.end():query.start()]
        assert not re.search(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s', transport, re.M), 'no hidden helper before root query'
        assert not re.search(r'^\s*load\s+[^,\n]+,"[^"\n]*generated code in ', transport, re.M), 'paired query preserves authored call diagnostic state'
        result = call.group(1).strip()
        stores = re.findall(r'^\s*copy\s+([^,\n]+),' + re.escape(result) + r'\s*$', transport, re.M)
        assert len(stores) == 1 and stores[0] not in slots, 'fresh owned result carrier per call site'
        carrier = stores[0]; slots.add(carrier)
        after = body[query.end():]
        branch = re.search(r'^\s*brt\s+([^,\s]+),' + re.escape(query.group(1).strip()) + r'\s*$', after, re.M)
        assert branch, 'query directly controls an explicit true unwind edge'
        synthetic = source_at(query.end() + branch.start())
        assert synthetic.group(3) == '6', 'other synthetic guard transport remains without source coordinates'
        assert derived.group(1) != synthetic.group(1), 'root query presentation differs from its co-origin synthetic branch'
        assert not re.search(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s', after[:branch.start()], re.M), 'no helper between query and branch'
        target = re.search(r'^' + re.escape(branch.group(1)) + r':\s*$', body, re.M)
        assert target, 'visible program-unwind destination'
        suffix = body[target.end():]
        terminal = re.search(r'^\s*ret\s+([^\s]+)\s*$', suffix, re.M)
        assert terminal, 'program unwind returns an owned result'
        unwind = suffix[:terminal.end()]
        returned = terminal.group(1)
        assert returned == carrier or re.search(r'^\s*copy\s+' + re.escape(returned) + ',' + re.escape(carrier) + r'\s*$', unwind, re.M), 'unwind returns its matching call carrier'
        for forbidden in ('llctarget.present', 'llctarget.absent', 'llctarget.requirevalue', 'llctarget.setresult'):
            assert not named_calls(unwind, forbidden), ('unwind skips result creation and observation', forbidden)
        assert not re.search(r'^\s*null\s+' + re.escape(returned) + r'\s*$', unwind, re.M), 'cleanup preserves completed result'
        continuation = re.match(r'\s*br[ \t]+([^\s]+)[ \t]*(?:\r?\n|$)', after[branch.end():])
        assert continuation, 'visible ordinary continuation edge'
        target = re.search(r'^' + re.escape(continuation.group(1)) + r':\s*$', body, re.M)
        assert target, 'private ordinary continuation destination'
        normal = re.split(r'^[^\s:]+:\s*$', body[target.end():], maxsplit=1, flags=re.M)[0]
        carried = re.search(r'^\s*copy\s+[^,\n]+,' + re.escape(carrier) + r'\s*$', normal, re.M)
        assert carried, 'ordinary continuation loads its matching carrier'
        assert not re.search(r'^\s*load\s+[^,\n]+,"[^"\n]*generated code in ', normal[:carried.end()], re.M), 'continuation carrier transport preserves authored call diagnostic state'


def shapes(text, source, target):
    anchors = {}
    for record in source_records(text):
        step = int(record.group(1)); anchor = record.groups()[1:]
        assert step <= 4294967295, 'source step fits the RXAS metadata width'
        if step:
            assert anchors.setdefault(step, anchor) == anchor, ('one source anchor per nonzero step identity', step)
    wrapper, body = unit(text)
    assert not re.search(r'^\s*exit\s', body, re.M), 'source-unit EXIT returns through its frame'
    assert not named_calls(body, 'llctarget.status'), 'full result reaches the outer status bridge'
    assert named_calls(wrapper, 'llctarget.status'), 'existing outer status adapter'
    assert named_calls(body, 'llctarget.present') and named_calls(body, 'llctarget.absent'), 'present and absent EXIT packets'
    assert named_calls(body, 'rexxactivation.rexxactivationarguments.requestprogramexit'), 'existing shared program-root operation'
    assert named_calls(body, 'rexxactivation.rexxactivationarguments.programexitrequested'), 'existing shared program-root query'
    exports = re.findall(r'^\S+\([^\n]*\.expose=(llctarget\.[^\s]+)\s*$', target, re.M)
    assert len(exports) == len(set(exports)) == 39, ('target API including clausepending', exports)
    assert set(exports) == {'llctarget.' + name for name in (
        'literal', 'poolnew', 'poolread', 'poolwrite', 'binaryop', 'unaryop',
        'truth', 'sayvalue', 'sayconfigured', 'setting', 'digits', 'arguments',
        'mainargs', 'append', 'countargs', 'existsarg', 'present', 'absent',
        'emptyreturn', 'bifarguments', 'missing', 'missingconfigured',
        'requirevalue', 'setresult', 'status', 'lengthvalue', 'failuremessage',
        'whentruth', 'selectmissing', 'donew', 'docapture', 'dostart', 'doentry',
        'doadvance', 'whiletruth', 'untiltruth', 'invalidtransfer', 'parsesource',
        'clausepending',
    )}, ('exact target export inventory', exports)
    data = source.read_bytes()
    needle = b"exit LOCAL || ':' || BUMP()"
    _, line, column = source_position(data, needle)
    ending = column + len(needle.decode('utf-8'))
    assert re.search(r'^\s*\.srcstep\s+\d+\s+\d+\s+(?:17|65)\s+"source-c\.crexx"\s+' +
                     rf'{line}\s+{column}\s+{ending}\s+"', body, re.M), 'authored nested private EXIT span'
    guard_shapes(body)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
    parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
    parser.add_argument('--ir', choices=('on', 'off'), default='on')
    parser.add_argument('--phase', choices=('all', 'emit', 'source', 'host', 'refusals', 'contracts', 'shapes'), default='all')
    parser.add_argument('--inspect', type=Path)
    args = parser.parse_args()
    emit = args.phase == 'emit'
    if emit and args.inspect is None:
        parser.error('--phase emit requires --inspect')
    build = args.build.resolve()
    fixtures = Path(__file__).resolve().parent
    backend = build / 'research/level-l/compilers/backend' / args.mode
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    args.work.mkdir(parents=True, exist_ok=True)
    commands = 0
    with tempfile.TemporaryDirectory(prefix='exit-', dir=args.work) as temporary:
        work = Path(temporary)
        runtime = work / 'runtime'; runtime.mkdir()
        for name in libraries:
            root = backend if name == 'llctarget' else build / 'bin'
            shutil.copy2(root / (name + '.rxbin'), runtime / (name + '.rxbin'))
        dlls = tuple((build / 'bin').glob('*.dll')) if os.name == 'nt' else ()

        def tool(name):
            return build / 'bin' / (name + suffix)

        def stage(name, plugins):
            directory = work / name; directory.mkdir()
            vm = directory / (args.vm + suffix); shutil.copy2(tool(args.vm), vm)
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
            completed = subprocess.run([str(item) for item in command], cwd=cwd, env=env,
                                       capture_output=True, timeout=900)
            commands += 1
            diagnostic = completed.stdout + completed.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
            assert (completed.returncode != 0 if failure else completed.returncode == 0), (label, completed.returncode, diagnostic[-6000:])
            if silent:
                assert not diagnostic, (label, diagnostic[-4000:])
            return completed

        def compile_c(source, output, *, failure=False):
            emission_start(source)
            return run('compile ' + source.name,
                       [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []),
                        '-i', runtime, '-o', output, source], failure=failure, silent=not failure, cwd=host)

        def inspect(path, name):
            if args.inspect and not emit:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, args.inspect / f'{name}-{args.mode}-{args.ir}-{args.vm}.rxas')

        def image(assembly, name):
            run(name + ' assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), assembly], silent=True)
            linked = application / (name + '-linked')
            run(name + ' link', [tool('rxlink'), '-o', linked, assembly.with_suffix('.rxbin'),
                                *(runtime / (library + '.rxbin') for library in libraries)], silent=True)
            return linked.with_suffix('.rxbin')

        # Emit-only capture stays local to this recipe and never replaces a tool.
        def emission_hash(path):
            return hashlib.sha256(path.read_bytes()).hexdigest()

        if emit:
            emission_owner = 'exit'
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
            text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=exitfixture.run\2', text, count=1, flags=re.M)
            text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"exitfixture.run"', text, count=1, flags=re.M)
            assert headers == metadata == 1, 'one test-only exported entry0'
            exported.write_text(text, encoding='utf-8'); inspect(exported, 'exit-exported')
            return exported

        if args.phase in ('all', 'emit', 'source', 'host', 'shapes'):
            source = work / 'source-c.crexx'; data = (fixtures / source.name).read_bytes()
            source.write_bytes(data.replace(b'\n', b'\r\n') if args.mode == 'noopt' else data)
            assembly = application / 'source.rxas'; compiled = compile_c(source, assembly)
            inspect(assembly, 'exit-source'); text = assembly.read_text(encoding='utf-8')
            assert str(work) not in text and str(build) not in text, 'stable logical source identity'
            target = (backend / 'llctarget.rxas').read_text(encoding='utf-8')
            shapes(text, source, target)
            if emit:
                emission_finish(source, assembly, compiled)
                emission_derived(assembly, export_entry(text))
            if args.phase in ('all', 'source'):
                linked = image(assembly, 'source')
                completed = run('source execute', [target_vm, '--no-autoload', linked], cwd=application)
                expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
                assert completed.stdout.replace(b'\r\n', b'\n') == expected and not completed.stderr, ('independent source result', completed.stdout, completed.stderr, expected)
            if args.phase in ('all', 'host'):
                exported = export_entry(text)
                run('exported assemble', [tool('rxas'), '-n', exported], silent=True)
                imports = work / 'B caller imports'; imports.mkdir()
                for provider in runtime.glob('*.rxbin'):
                    shutil.copy2(provider, imports / provider.name)
                shutil.copy2(exported.with_suffix('.rxbin'), imports / 'exitfixture.rxbin')
                caller = application / 'exit-host'
                run('B host compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import',
                                       '-i', imports, '-o', caller, fixtures / 'host.crexx'], silent=True)
                run('B host assemble', [tool('rxas'), '-n', caller], silent=True)
                linked = application / 'exit-host-linked'; control = work / 'host.ctl'
                inputs = [caller.with_suffix('.rxbin'), exported.with_suffix('.rxbin'),
                          *(runtime / (library + '.rxbin') for library in libraries)]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) + 'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run('B host link', [tool('rxlink'), '-c', control], silent=True)
                completed = run('full result, skipped effects and restored ownership', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                assert re.fullmatch(rb'EXIT_HOST\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
                completed = run('expired program root during private EXIT', [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', 'expired-root'], failure=True, cwd=application)
                _, line, _ = source_position(source.read_bytes(), b"exit 'failure value'")
                assert not completed.stdout and b'REFERENCE_INVALID' in completed.stderr, (completed.stdout, completed.stderr)
                assert f'source-c.crexx:{line}:'.encode() in completed.stderr and b'UNREACHED' not in completed.stderr, ('original authored EXIT failure', completed.stderr)
                completed = run('expired inherited root during post-call query', [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', 'expired-query'], failure=True, cwd=application)
                _, line, _ = source_position(source.read_bytes(), b'call NORMAL')
                assert not completed.stdout and b'REFERENCE_INVALID' in completed.stderr, (completed.stdout, completed.stderr)
                assert f'source-c.crexx:{line}:'.encode() in completed.stderr and b'UNREACHED' not in completed.stderr, ('original authored CALL failure at generated query', completed.stderr)

        if args.phase in ('all', 'emit', 'refusals'):
            cases = (
                ('comma', 'exit 1, 2\n', ','),
                ('close', 'exit 1 )\n', ')'),
                ('dead-expression', 'if 0 then exit +\nreturn 0\n', '+'),
                ('scalar-column', "if 'é' = 'é' then exit 1,;\n", ','),
            )
            output = application / 'prior.rxas'; prior = b'unchanged prior output\n'; output.write_bytes(prior)
            for name, text, token in cases:
                source = work / (name + '.crexx'); source.write_bytes(text.encode('utf-8'))
                completed = compile_c(source, output, failure=True)
                diagnostic = completed.stdout + completed.stderr
                offset, line, column = source_position(text.encode('utf-8'), token.encode())
                position = f'at line {line}, column {column} (byte {offset})'.encode()
                assert b'35.1' in diagnostic and position in diagnostic, (name, position, diagnostic)
                assert output.read_bytes() == prior, (name, 'failed request replaced prior output')
                if emit: emission_finish(source, output, completed, refused=True)

        if args.phase in ('all', 'contracts'):
            probe = host / 'exit-contracts-linked.rxbin'; shutil.copy2(backend / probe.name, probe)
            completed = run('independent raw/schema contracts', [host_vm, '--no-autoload', probe, '-a', runtime], cwd=host)
            assert re.fullmatch(rb'EXIT_CONTRACTS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
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
    print(f'EXIT_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
