#!/usr/bin/env python3
"""Focused RETURN checks over prepared products, with disposable outputs.

One C source compilation serves execution, RXAS and the direct B caller in
each lane. CALL owns unchanged raw-IR/45.1/44.1 controls; PROCEDURE owns the
broader recursion/fatal lifetime matrix. This runner does not launch builds.
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
    assert re.findall(r'^llf\d+\(\)', text, re.M) == ['llf1()'], 'one shared C body'
    header = re.search(r'^llf1\(\) \.locals=', text, re.M)
    assert header, 'source-unit function header'
    body = text[header.start():text.index('llerror1_1:')]
    assert 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int' in body
    return body, header.start()


def source_position(data, needle):
    offset = data.index(needle)
    line = data[:offset].count(b'\n') + 1
    first = data.rfind(b'\n', 0, offset) + 1
    column = len(data[first:offset].decode('utf-8')) + 1
    return offset, line, column


def shapes(text, source):
    body, start = unit(text)
    assert not re.search(r'^\s*exit\s', body, re.M), 'RETURN completes a frame'
    assert not named_calls(body, 'llctarget.status'), 'status conversion belongs to the outer wrapper'
    assert named_calls(text[:start], 'llctarget.status'), 'existing outer status adapter'
    assert named_calls(body, 'llctarget.present') and named_calls(body, 'llctarget.emptyreturn')
    data = source.read_bytes()
    _, line, column = source_position(data, b"return BUMP() || ':' || LOCAL")
    anchor = re.search(r'^\s*\.srcstep\s+\d+\s+\d+\s+(?:17|65)\s+"source-c\.crexx"\s+' +
                       rf'{line}\s+{column}\s+\d+\s+"', body, re.M)
    assert anchor, 'authored nested private RETURN span'
    terminal = re.search(r'^\s*ret\s+([^\s]+)\s*$', body[anchor.end():], re.M)
    assert terminal, 'nested RETURN terminal'
    segment = body[anchor.end():anchor.end() + terminal.end()]
    packet = named_calls(segment, 'llctarget.present')
    assert packet and packet[-1].group(1).strip() == terminal.group(1), 'owned packet survives cleanup to RETURN'
    owners = {match.group(1).strip() for match in named_calls(body, 'llctarget.donew')}
    closes = [match for match in re.finditer(r'^\s*null\s+([^\s]+)\s*$', segment, re.M)
              if match.group(1) in owners]
    restored = named_calls(segment, 'rexxclassiccontext.rexxclassiccontext.bindpool')
    assert owners and closes and restored, 'stateful/private cleanup is present'
    assert packet[-1].end() < closes[0].start() < restored[0].start(), 'result before DO close before pool restoration'
    pools = {match.group(1).strip() for match in named_calls(body, 'llctarget.poolnew')}
    releases = [match for match in re.finditer(r'^\s*null\s+([^\s]+)\s*$', segment, re.M)
                if match.group(1) in pools]
    assert releases and restored[0].end() < releases[0].start(), 'restore caller binding before private owner release'
    returned = terminal.group(1)
    assert not re.search(r'^\s*null\s+' + re.escape(returned) + r'\s*$', segment, re.M), 'owned packet is not cleared'
    # The Unicode condition precedes RETURN on the same physical line.
    _, line, column = source_position(data, "return 'é' || x2c('00') || '雪'".encode('utf-8'))
    ending = len(data.splitlines()[line - 1].decode('utf-8')) + 1
    assert re.search(r'^\s*\.srcstep\s+\d+\s+\d+\s+(?:17|65)\s+"source-c\.crexx"\s+' +
                     rf'{line}\s+{column}\s+{ending}\s+"', body, re.M), 'RETURN uses original scalar columns'


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
    build = args.build.resolve(); fixtures = Path(__file__).resolve().parent
    backend = build / 'research/level-l/compilers/backend' / args.mode
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    args.work.mkdir(parents=True, exist_ok=True)
    commands = 0
    with tempfile.TemporaryDirectory(prefix='return-', dir=args.work) as temporary:
        work = Path(temporary); runtime = work / 'runtime'; runtime.mkdir()
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
                name = plugin + '.rxplugin'
                source = build / 'bin' / name
                if not source.exists():
                    source = build / 'bin' / 'providers' / name
                shutil.copy2(source, providers / name)
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
                                *(runtime / (name + '.rxbin') for name in libraries)], silent=True)
            return linked.with_suffix('.rxbin')

        # Emit-only capture stays local to this recipe and never replaces a tool.
        def emission_hash(path):
            return hashlib.sha256(path.read_bytes()).hexdigest()

        if emit:
            emission_owner = 'return'
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
            text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=returnfixture.run\2', text, count=1, flags=re.M)
            text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"returnfixture.run"', text, count=1, flags=re.M)
            assert headers == metadata == 1, 'one test-only exported entry0'
            exported.write_text(text, encoding='utf-8'); inspect(exported, 'return-exported')
            return exported

        if args.phase in ('all', 'emit', 'source', 'host', 'shapes'):
            source = work / 'source-c.crexx'
            data = (fixtures / source.name).read_bytes()
            # Complementary lanes exercise both physical layouts with one
            # compilation of the maintained source fixture per lane.
            source.write_bytes(data.replace(b'\n', b'\r\n') if args.mode == 'noopt' else data)
            assembly = application / 'source.rxas'; compiled = compile_c(source, assembly)
            inspect(assembly, 'return-source'); text = assembly.read_text()
            assert str(work) not in text and str(build) not in text, 'stable logical source identity'
            shapes(text, source)
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
                shutil.copy2(exported.with_suffix('.rxbin'), imports / 'returnfixture.rxbin')
                caller = application / 'return-host'
                run('B host compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import',
                                       '-i', imports, '-o', caller, fixtures / 'host.crexx'], silent=True)
                run('B host assemble', [tool('rxas'), '-n', caller], silent=True)
                linked = application / 'return-host-linked'; control = work / 'host.ctl'
                inputs = [caller.with_suffix('.rxbin'), exported.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) + 'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run('B host link', [tool('rxlink'), '-c', control], silent=True)
                completed = run('full result/status and restoration', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                assert re.fullmatch(rb'RETURN_HOST\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)

        if args.phase in ('all', 'emit', 'refusals'):
            cases = (
                ('comma', 'return 1, 2\n', ','),
                ('close', 'return 1 )\n', ')'),
                ('dead-expression', 'if 0 then return +\nreturn 0\n', '+'),
                ('final-LF', 'return 1,\n', ','),
                ('final-CRLF-comments', 'return 1,\r\n /* tail */ ;\r\n', ','),
                ('hidden-comment-newline', 'return 1, /* hidden\n */ +2\n', ','),
                ('scalar-column', "if 'é' = 'é' then return 1,;\n", ','),
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
            probe = host / 'return-contracts-linked.rxbin'; shutil.copy2(backend / probe.name, probe)
            completed = run('independent source/AST contracts', [host_vm, '--no-autoload', probe], cwd=host)
            assert re.fullmatch(rb'RETURN_CONTRACTS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
        if emit:
            assert len(emission_records) == 7 + 1 and len(emission_started) == 7 + 1
            assert sum(row['disposition'] == 'EMITTED' for row in emission_records) == 1
            assert sum(row['disposition'] == 'REFUSAL' for row in emission_records) == 7
            assert len(emission_derived_records) == 1
            identity = dict(kind='EMISSION_ONLY', owner=emission_owner, mode=args.mode,
                            producer_vm=args.vm, semantic_ir=args.ir, dialect='C',
                            adapter_sha256=emission_hash(Path(__file__)),
                            entry_sha256=emission_hash(fixtures / ('check-' + emission_owner + '.py')),
                            compiler_sha256=emission_hash(compiler), vm_sha256=emission_hash(host_vm),
                            imports={path.name: emission_hash(path) for path in sorted(runtime.glob('*.rxbin'))},
                            providers={path.name: emission_hash(path) for path in sorted((host / 'providers').glob('*')) if path.is_file()},
                            host_libraries={path.name: emission_hash(path) for path in sorted(host.glob('*.dll'))},
                            bindings=[], routine_export='', host_export='',
                            limits=dict(frontend_bytes='1000000', nesting='512', output_bytes='67108864', registers='65536'),
                            requests=emission_records, derived=emission_derived_records)
            (emission_dir / 'identity.json').write_text(json.dumps(identity, indent=2) + '\n', encoding='utf-8')
            print(f'{emission_owner.upper()}_EMISSION|mode={args.mode}|vm={args.vm}|ir={args.ir}|requests={len(emission_records)}|derived=1|EMISSION')
            return
    print(f'RETURN_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
