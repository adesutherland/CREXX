#!/usr/bin/env python3
"""Independent C16 implicit commands/warnings over prepared products; never builds.

One maintained source per lane supplies runtime, B-host and readable RXAS
controls. Conditions/TRACE/native host transport remain CADDR-AC08 work.
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


# C15 owns the unchanged unit ABI and command-owner cleanup expectations.
ADDRESS = runpy.run_path(str(Path(__file__).resolve().parents[1] / 'address' / 'check-address.py'))
called = ADDRESS['called']
unit = ADDRESS['unit']
source_position = ADDRESS['source_position']


def expected_warnings(data, filename):
    data = data.replace(b'\r\n', b'\n')
    heads = ((b'  COMMAND\n', b'COMMAND'), (b'  MAKE()\n', b'MAKE'),
             (b'if 0 then ((DEAD));', b'DEAD'), ("if 'é' = 'é' then if 0 then SAME".encode(), b'SAME'),
             (b'if 0 then -1\n', b'-'), (b"if 0 then 'a' + 1\n", b"'a'"),
             (b'if 0 then 2 + 3\n', b'2'), (b'\nESCAPE()\n', b'ESCAPE'))
    records = []
    for statement, token in heads:
        first = data.index(statement) + statement.index(token)
        line = data[:first].count(b'\n') + 1
        start = data.rfind(b'\n', 0, first) + 1
        column = len(data[start:first].decode('utf-8')) + 1
        records.append((first, f'{filename}:{line}:{column}: warning RXC-LC-IMPLICIT_ADDRESS: Implicit ADDRESS command\n'.encode()))
    return b''.join(record for _, record in sorted(records))


def exit_bypass(body, source, prefix):
    _, line, _ = source_position(source, b'\nESCAPE()')
    line += 1
    current = None; query_results = []
    symbol = 'rexxactivation.rexxactivationarguments.programexitrequested'
    for row in body.splitlines():
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            if fields[4] == 'source-c.crexx':
                current = int(fields[5])
        if current == line:
            query_results.extend(item[1].strip() for item in called(row, symbol))
    assert len(query_results) == 1, 'implicit operand has one program-root query'
    query = [item for item in called(body, symbol) if item[1].strip() == query_results[0]]
    assert len(query) == 1, 'unique operand root-query result'
    after = body[query[0].end():]
    branch = re.search(r'^\s*brt\s+([^,\s]+),' + re.escape(query_results[0]) + r'\s*\n\s*br\s+(\S+)\s*$', after, re.M)
    assert branch and not re.search(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s', after[:branch.start()], re.M), 'root query immediately controls operand unwind'
    labels = list(re.finditer(r'^(llb\d+):\s*$', body, re.M))
    blocks = {label[1]: body[label.end():labels[i + 1].start() if i + 1 < len(labels) else len(body)] for i, label in enumerate(labels)}
    assert branch[1] in blocks and branch[2] in blocks and branch[1] != branch[2]
    true = blocks[branch[1]]; false = blocks[branch[2]]
    assert not called(true, prefix + '§factory') and not called(true, prefix + 'run'), 'EXIT true edge bypasses both implicit construction and dispatch'
    assert re.search(r'^\s*ret\s+r\d+\s*$', true, re.M), 'EXIT returns owned packet after cleanup'
    assert len(called(false, prefix + '§factory')) == len(called(false, prefix + 'run')) == 1, 'only false continuation constructs and consumes implicit command'


def shapes(text, source, target):
    body = unit(text); prefix = 'rexxclassicaddress.rexxclassicaddresscommand.'
    constructors = called(body, prefix + '§factory'); runs = called(body, prefix + 'run')
    assert constructors and len(constructors) == len(runs), 'one constructor/run per static surviving command'
    owners = ADDRESS['command_cleanup'](body, constructors, runs, prefix)
    previous = 0
    for constructor in constructors:
        before = body[previous:constructor.start()]
        conversions = called(before, 'rexxvalue.rexxvalue.asstring')
        environments = called(before, 'rexxactivation.rexxactivationarguments.addressenvironment')
        assert conversions and environments and conversions[-1].end() < environments[-1].start(), 'command conversion precedes parent active-environment capture'
        following = body[constructor.end():]
        capture = called(following, prefix + 'useactiveconnections')
        run = called(following, prefix + 'run')
        assert capture and run and capture[0].end() < run[0].start(), 'active connections captured on owned command before run'
        previous = constructor.end()
    fatal = text[text.index('llerror1:'):]
    restores = called(fatal, 'rexxclassiccontext.rexxclassiccontext.bindpool')
    assert restores
    for owner in owners:
        release = re.search(r'^\s*null\s+' + re.escape(owner) + r'\s*$', fatal, re.M)
        assert release and release.end() < restores[0].start(), 'pending implicit owners released before fatal private-pool restore'
    assert not called(body, 'llctarget.status') and not re.search(r'^\s*exit\s', body, re.M), 'source frame retains owned completion'
    assert not called(body, '_rxsysb._address_dispatch_request'), 'shared B adapter owns request/dispatch algorithms'
    definitions = re.findall(r'^\S+\([^\n]*\s\.locals=\d+[^\n]*\.expose=(llctarget\.[^\s]+)\s*$', target, re.M)
    assert len(definitions) == len(set(definitions)) == 39, 'target API including clausepending'
    assert set(definitions) == {'llctarget.' + name for name in (
        'literal', 'poolnew', 'poolread', 'poolwrite', 'binaryop', 'unaryop',
        'truth', 'sayvalue', 'sayconfigured', 'setting', 'digits', 'arguments',
        'mainargs', 'append', 'countargs', 'existsarg', 'present', 'absent',
        'emptyreturn', 'bifarguments', 'missing', 'missingconfigured',
        'requirevalue', 'setresult', 'status', 'lengthvalue', 'failuremessage',
        'whentruth', 'selectmissing', 'donew', 'docapture', 'dostart', 'doentry',
        'doadvance', 'whiletruth', 'untiltruth', 'invalidtransfer', 'parsesource',
        'clausepending',
    )}, ('exact target export inventory', definitions)
    exit_bypass(body, source.read_bytes(), prefix)


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
    with tempfile.TemporaryDirectory(prefix='command-', dir=args.work) as temporary:
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
                       '-i', runtime, '-o', output, source], failure=failure, cwd=host)

        def inspect(path, name):
            if args.inspect and not emit:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, args.inspect / f'{name}-{args.mode}-{args.ir}-{args.vm}.rxas')

        # Emit-only capture stays local to this recipe and never replaces a tool.
        def emission_hash(path):
            return hashlib.sha256(path.read_bytes()).hexdigest()

        if emit:
            emission_owner = 'command'
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
            text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=commandfixture.run\2', text, count=1, flags=re.M)
            text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"commandfixture.run"', text, count=1, flags=re.M)
            assert headers == metadata == 1, 'one test-only exported entry0, no production ABI change'
            exported.write_text(text, encoding='utf-8'); inspect(exported, 'command-exported')
            return exported

        if args.phase in ('all', 'emit', 'source', 'host', 'fatal', 'shapes'):
            source = work / 'source-c.crexx'; data = (fixtures / source.name).read_bytes()
            source.write_bytes(data if args.mode == 'opt' else data.replace(b'\n', b'\r\n'))
            assembly = application / 'source.rxas'; compiled = compile_c(source, assembly)
            assert not compiled.stdout and compiled.stderr.replace(b'\r\n', b'\n') == expected_warnings(source.read_bytes(), source.name), ('exact once-only CLI warnings', compiled.stdout, compiled.stderr)
            inspect(assembly, 'command-source'); text = assembly.read_text()
            assert str(work) not in text and str(build) not in text, 'stable logical source identities'
            shapes(text, source, (backend / 'llctarget.rxas').read_text())
            if emit:
                emission_finish(source, assembly, compiled)
                emission_derived(assembly, export_entry(text))
            if args.phase in ('all', 'source'):
                run('source assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), assembly], silent=True)
                linked = application / 'source-linked'
                run('source link', [tool('rxlink'), '-o', linked, assembly.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)], silent=True)
                completed = run('source commands/default', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
                assert completed.stdout.replace(b'\r\n', b'\n') == expected and not completed.stderr, (completed.stdout, completed.stderr)
            if args.phase in ('all', 'host', 'fatal'):
                exported = export_entry(text)
                run('exported assemble', [tool('rxas'), '-n', exported], silent=True)
                imports = work / 'B caller imports'; imports.mkdir()
                for provider in runtime.glob('*.rxbin'):
                    shutil.copy2(provider, imports / provider.name)
                shutil.copy2(exported.with_suffix('.rxbin'), imports / 'commandfixture.rxbin')
                caller = application / 'command-host'
                run('B host compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import', '-i', imports, '-o', caller, fixtures / 'host.crexx'], silent=True)
                run('B host assemble', [tool('rxas'), '-n', caller], silent=True)
                linked = application / 'command-host-linked'; control = work / 'host.ctl'
                inputs = [caller.with_suffix('.rxbin'), exported.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) + 'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run('B host link', [tool('rxlink'), '-c', control], silent=True)
                if args.phase in ('all', 'host'):
                    completed = run('active command/provider controls', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                    assert re.fullmatch(rb'COMMAND_HOST\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
                if args.phase in ('all', 'fatal'):
                    line = source.read_text().splitlines().index('LATEPROCEDURE:') + 2
                    completed = run('implicit no-op consumes PROCEDURE eligibility', [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', 'late'], failure=True, cwd=application)
                    assert completed.stdout.replace(b'\r\n', b'\n') == b'COMMAND_FATAL|before\n'
                    assert b'CLASSIC_SYNTAX' in completed.stderr and b'17.1' in completed.stderr and f'source-c.crexx:{line}:'.encode() in completed.stderr, completed.stderr
                    assert b'UNREACHED' not in completed.stdout

        if args.phase in ('all', 'emit', 'refusals'):
            output = application / 'prior.rxas'; prior = b'unchanged prior output\n'; output.write_bytes(prior)
            for name, content in (('number-assignment', '1 = 2'), ('constant-assignment', '7dogs = 2'),
                                  ('dot-assignment', '.name = 2'), ('dead-expression', 'if 0 then 1 +')):
                source = work / (name + '.crexx'); source.write_text('options levelc\n' + content + '\n')
                completed = compile_c(source, output, failure=True)
                diagnostic = re.escape(source.name.encode()) + rb': LL_COMPILER_SYNTAX at line \d+, column \d+ \(byte \d+\):'
                assert re.match(diagnostic, completed.stdout) and not completed.stderr, (name, completed.stdout, completed.stderr)
                assert output.read_bytes() == prior and not Path(str(output) + '.llrxc-stage').exists(), 'failed command compile retains prior publication'
                if emit: emission_finish(source, output, completed, refused=True)

        if args.phase in ('all', 'contracts'):
            probe = host / 'command-contracts-linked.rxbin'; shutil.copy2(backend / probe.name, probe)
            completed = run('warnings and active IR admission', [host_vm, '--no-autoload', probe, '-a', runtime], cwd=host)
            assert re.fullmatch(rb'COMMAND_CONTRACTS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
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
    print(f'COMMAND_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
