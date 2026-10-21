#!/usr/bin/env python3
"""Independent explicit ADDRESS C15 checks over prepared products; never builds.

One maintained C source per lane serves default output, pure B environment,
EXIT/fatal checks and RXAS. Implicit C16 warnings and configured conditions,
TRACE and host/platform delivery retain their separate acceptance owners.
"""
import argparse
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tempfile


def called(text, symbol):
    return list(re.finditer(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s+([^,\n]+),§?' +
                           re.escape(symbol) + r'\(\)(?:,|\s*$)', text, re.M | re.I))


def unit(text):
    headers = list(re.finditer(r'^llf\d+\(\) \.locals=', text, re.M))
    assert len(headers) == 1 and headers[0].group() == 'llf1() .locals=', 'one shared C unit definition'
    start = headers[0].start()
    body = text[start:text.index('llerror1_1:', start)]
    assert 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int' in body
    return body


def source_position(data, needle):
    offset = data.index(needle)
    line = data[:offset].count(b'\n') + 1
    first = data.rfind(b'\n', 0, offset) + 1
    return offset, line, len(data[first:offset].decode('utf-8')) + 1


def command_cleanup(body, constructors, runs, prefix):
    owners = [item[1].strip() for item in constructors]
    assert all(re.fullmatch(r'r[1-9]\d*', owner) for owner in owners), ('dedicated nonzero command owners', owners)
    assert len(owners) == len(set(owners)), ('distinct owner per static command', owners)
    for constructor, run, owner in zip(constructors, runs, owners):
        assert constructor.end() < run.start(), ('construction precedes consuming run', owner)
        links = list(re.finditer(r'^\s*link\s+([^,\s]+),' + re.escape(owner) + r'\s*$',
                                 body[constructor.end():run.start()], re.M))
        assert links, ('run borrows its stable command owner', owner)
        receiver = links[-1][1]
        after = body[run.end():]
        cleared = re.search(r'^\s*null\s+' + re.escape(owner) + r'\s*$', after, re.M)
        assert cleared, ('normal consuming run releases command owner', owner)
        inspection = after[:cleared.start()]
        getters = [called(inspection, prefix + name) for name in ('syntaxerror', 'conditionid', 'conditiondescription')]
        assert all(len(items) == 1 for items in getters), ('inspect each result before owner release', owner)
        assert getters[0][0].end() < getters[1][0].start() < getters[2][0].start(), ('ordered command result inspection', owner)
        detached = re.search(r'^\s*unlink\s+' + re.escape(receiver) + r'\s*$', inspection, re.M)
        assert detached and getters[-1][0].end() < detached.start(), ('end command borrow before owner release', owner)
        assert not re.search(r'^\s*(?:br\w*|b(?:eq|ne|lt|le|gt|ge)|signal\w*|ret|exit)\s', inspection, re.M), ('release command before normal condition transfer', owner)
    return owners


def operand_exit_bypass(body, source, prefix):
    _, line, _ = source_position(source, b"address 'BRIDGE-TEST' ESCAPE()")
    current = None; queries = []
    query_symbol = 'rexxactivation.rexxactivationarguments.programexitrequested'
    for row in body.splitlines(keepends=True):
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            if fields[4] == 'source-c.crexx':
                current = int(fields[5])
        if current == line:
            queries.extend(called(row, query_symbol))
    assert len(queries) == 1, 'command operand has one root-exit query'
    query = queries[0]
    # Locate the full-text call by its unique result register, then follow the
    # declared CFG labels; physical blocks between the query and targets differ.
    matches = [item for item in called(body, query_symbol) if item[1].strip() == query[1].strip()]
    assert len(matches) == 1, 'unique command operand query result'
    after = body[matches[0].end():]
    branch = re.search(r'^\s*brt\s+([^,\s]+),' + re.escape(query[1].strip()) +
                       r'\s*\n\s*br\s+([^\s]+)\s*$', after, re.M)
    assert branch, 'operand query controls explicit true/false continuations'
    assert not re.search(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s', after[:branch.start()], re.M), 'no helper between operand query and branch'
    blocks = {}
    labels = list(re.finditer(r'^(llb\d+):\s*$', body, re.M))
    for index, label in enumerate(labels):
        end = labels[index + 1].start() if index + 1 < len(labels) else len(body)
        blocks[label[1]] = body[label.end():end]
    assert branch[1] in blocks and branch[2] in blocks and branch[1] != branch[2], 'distinct declared operand continuations'
    exiting = blocks[branch[1]]; continuing = blocks[branch[2]]
    assert not called(exiting, prefix + '§factory') and not called(exiting, prefix + 'run'), 'operand EXIT bypasses command construction and dispatch'
    assert re.search(r'^\s*ret\s+r\d+\s*$', exiting, re.M), 'operand EXIT returns the owned result after cleanup'
    assert len(called(continuing, prefix + '§factory')) == len(called(continuing, prefix + 'run')) == 1, 'false continuation alone constructs and consumes the command'


def lasting_clause(body, source):
    needle = b"address 'ALPHA' with output,"
    offset, line, column = source_position(source, needle)
    whole_line = source[offset:source.index(b'\n', offset)].rstrip(b'\r').decode('utf-8')
    child_column = column + whole_line.index('output')
    spans = {(column, len(whole_line) + 1), (child_column, len(whole_line) + 1)}
    found = set(); clauses = set()
    for row in body.splitlines():
        if not row.lstrip().startswith('.srcstep '):
            continue
        fields = shlex.split(row)
        if fields[4] != 'source-c.crexx' or int(fields[5]) != line:
            continue
        span = (int(fields[6]), int(fields[7]))
        if span[0] not in (column, child_column):
            continue
        assert span in spans and fields[8] == whole_line and int(fields[3]) & 1, 'lasting ADDRESS/WITH retains exact authored child span'
        found.add(span); clauses.add(int(fields[2]))
    assert found == spans, 'lasting selection and connection source anchors are present'
    assert len(clauses) == 1 and 0 not in clauses, 'lasting ADDRESS/WITH shares one nonzero clause identity'


def shapes(text, source, target):
    body = unit(text)
    prefix = 'rexxclassicaddress.rexxclassicaddresscommand.'
    constructors = called(body, prefix + '§factory')
    runs = called(body, prefix + 'run')
    assert constructors and len(constructors) == len(runs), 'one owner construction and consuming run per surviving static command'
    for name in ('withconnection', 'withbinding'):
        assert called(body, prefix + name), 'existing B adapter owns ' + name
    for name in ('setaddressenvironment', 'swapaddressenvironment', 'setaddressconnection'):
        assert called(body, 'rexxactivation.rexxactivationarguments.' + name), 'existing activation owner ' + name
    assert called(body, 'addressprobe.observe'), 'one explicitly prepared ordinary signed state provider'
    assert not called(body, '_rxsysb._address_dispatch_request'), 'request snapshots and dispatch stay in the shared B adapter'
    assert not called(body, 'llctarget.status') and not re.search(r'^\s*exit\s', body, re.M), 'source frame retains full completion'
    definitions = re.findall(r'^\S+\([^\n]*\s\.locals=\d+[^\n]*\.expose=(llctarget\.[^\s]+)\s*$', target, re.M)
    assert len(definitions) == len(set(definitions)) == 36, ('unchanged target export surface', definitions)
    owners = command_cleanup(body, constructors, runs, prefix)
    operand_exit_bypass(body, source.read_bytes(), prefix)
    lasting_clause(body, source.read_bytes())
    fatal = text[text.index('llerror1:'):]
    restored = called(fatal, 'rexxclassiccontext.rexxclassiccontext.bindpool')
    assert restored, 'private source has fatal pool restoration'
    for owner in owners:
        cleared = re.search(r'^\s*null\s+' + re.escape(owner) + r'\s*$', fatal, re.M)
        assert cleared and cleared.end() < restored[0].start(), ('fatal command release precedes private pool restoration', owner)
    # Count only exact authored command-line records, excluding helper/source
    # records and arbitrary register names. This line has no STREAM resource.
    needle = b"address 'BRIDGE-TEST' 'again :SCALAR'"
    _, line, _ = source_position(source.read_bytes(), needle)
    current = None; conversions = 0; creations = 0; invocations = 0
    for row in body.splitlines():
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            current = int(fields[5]) if fields[4] == 'source-c.crexx' else None
        if current != line:
            continue
        conversions += bool(called(row, 'rexxvalue.rexxvalue.asstring'))
        creations += bool(called(row, prefix + '§factory'))
        invocations += bool(called(row, prefix + 'run'))
    assert (conversions, creations, invocations) == (1, 1, 1), ('one static text conversion/new/run in repeated command', conversions, creations, invocations)
    data = source.read_bytes(); needle = "if 'é' = 'é' then address CREXX ''".encode()
    offset, line, column = source_position(data, needle)
    address = data.index(b'address CREXX', offset)
    column += len(data[offset:address].decode('utf-8'))
    assert re.search(r'^\s*\.srcstep\s+\d+\s+\d+\s+(?:17|65)\s+"source-c\.crexx"\s+' +
                     rf'{line}\s+{column}\s+\d+\s+"', body, re.M), 'ADDRESS after Unicode retains scalar source columns'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
    parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
    parser.add_argument('--ir', choices=('on', 'off'), default='on')
    parser.add_argument('--phase', choices=('all', 'source', 'host', 'fatal', 'refusals', 'contracts', 'shapes'), default='all')
    parser.add_argument('--inspect', type=Path)
    args = parser.parse_args()
    build = args.build.resolve(); fixtures = Path(__file__).resolve().parent
    backend = build / 'research/level-l/compilers/backend' / args.mode
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    args.work.mkdir(parents=True, exist_ok=True)
    commands = 0
    with tempfile.TemporaryDirectory(prefix='address-', dir=args.work) as temporary:
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
                shutil.copy2(build / 'bin' / (plugin + '.rxplugin'), providers / (plugin + '.rxplugin'))
            return directory, vm

        host, host_vm = stage('C compiler host', ('rxfs',))
        application, target_vm = stage('static application', ('rxfs', 'rxcstream'))
        compiler = host / 'llrxc-c-linked.rxbin'; shutil.copy2(backend / compiler.name, compiler)
        empty_path = work / 'empty PATH'; empty_path.mkdir()
        env = dict(os.environ, PATH=str(empty_path)); env.pop('CREXX_PROVIDER_PATH', None)
        assert not any(shutil.which(name, path=env['PATH']) for name in ('rxc', 'rxc-c'))

        def run(label, command, *, failure=False, silent=False, data=b'', cwd=work):
            nonlocal commands
            completed = subprocess.run([str(item) for item in command], cwd=cwd, env=env,
                                       input=data, capture_output=True, timeout=900)
            commands += 1
            diagnostic = completed.stdout + completed.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
            assert (completed.returncode != 0 if failure else completed.returncode == 0), (label, completed.returncode, diagnostic[-6000:])
            if silent:
                assert not diagnostic, (label, diagnostic[-4000:])
            return completed

        def compile_c(source, output, *, failure=False):
            return run('compile ' + source.name,
                       [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []),
                        '-i', runtime, '-o', output, '--provider', 'addressprobe', 'addressprobe.observe', source], failure=failure, silent=not failure, cwd=host)

        def inspect(path, name):
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, args.inspect / f'{name}-{args.mode}-{args.ir}-{args.vm}.rxas')

        # Explicit owning B preparation is outside the C request process/closure.
        if args.phase != 'contracts':
            provider = application / 'addressprobe'
            run('prepare ordinary B state provider', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []),
                '-x', '--no-exe-import', '-i', runtime, '-o', provider, fixtures / 'provider-b.crexx'], silent=True)
            inspect(provider.with_suffix('.rxas'), 'address-provider')
            run('state provider assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), provider], silent=True)
            shutil.copy2(provider.with_suffix('.rxbin'), runtime / 'addressprobe.rxbin')

        if args.phase in ('all', 'source', 'host', 'fatal', 'shapes'):
            source = work / 'source-c.crexx'
            data = (fixtures / source.name).read_bytes()
            source.write_bytes(data.replace(b'\n', b'\r\n') if args.mode == 'noopt' else data)
            assembly = application / 'source.rxas'; compile_c(source, assembly)
            inspect(assembly, 'address-source'); text = assembly.read_text()
            assert str(work) not in text and str(build) not in text, 'stable logical source identity'
            shapes(text, source, (backend / 'llctarget.rxas').read_text())
            if args.phase in ('all', 'source'):
                run('source assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), assembly], silent=True)
                linked = application / 'source-linked'
                run('source link', [tool('rxlink'), '-o', linked, assembly.with_suffix('.rxbin'),
                                    runtime / 'addressprobe.rxbin', *(runtime / (name + '.rxbin') for name in libraries)], silent=True)
                completed = run('default selection/resources', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
                assert completed.stdout.replace(b'\r\n', b'\n') == expected and not completed.stderr, (completed.stdout, completed.stderr, expected)
            if args.phase in ('all', 'host', 'fatal'):
                exported = application / 'source-exported.rxas'
                text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=addressfixture.run\2', text, count=1, flags=re.M)
                text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"addressfixture.run"', text, count=1, flags=re.M)
                assert headers == metadata == 1, 'one test-only exported entry0, no production ABI change'
                exported.write_text(text, encoding='utf-8'); inspect(exported, 'address-exported')
                run('exported assemble', [tool('rxas'), '-n', exported], silent=True)
                imports = work / 'B caller imports'; imports.mkdir()
                for provider in runtime.glob('*.rxbin'):
                    shutil.copy2(provider, imports / provider.name)
                shutil.copy2(exported.with_suffix('.rxbin'), imports / 'addressfixture.rxbin')
                caller = application / 'address-host'
                run('B host compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import',
                                       '-i', imports, '-o', caller, fixtures / 'host.crexx'], silent=True)
                run('B host assemble', [tool('rxas'), '-n', caller], silent=True)
                linked = application / 'address-host-linked'; control = work / 'host.ctl'
                inputs = [caller.with_suffix('.rxbin'), exported.with_suffix('.rxbin'), runtime / 'addressprobe.rxbin', *(runtime / (name + '.rxbin') for name in libraries)]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) + 'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run('B host link', [tool('rxlink'), '-c', control], silent=True)
                if args.phase in ('all', 'host'):
                    completed = run('pure provider and owned context controls', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                    assert re.fullmatch(rb'ADDRESS_HOST\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
                if args.phase in ('all', 'fatal'):
                    rows = source.read_text().splitlines()
                    cases = (('late', b'17.1', rows.index('LATEPROCEDURE:') + 2),
                             ('bad-count', b'54.1', rows.index("address 'BRIDGE-TEST' 'empty' with input stem INPUT.") + 1))
                    for mode, code, line in cases:
                        completed = run('fatal ' + mode, [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', mode], failure=True, cwd=application)
                        assert completed.stdout.replace(b'\r\n', b'\n') == b'ADDRESS_FATAL|before\n', (mode, completed.stdout)
                        assert code in completed.stderr and b'CLASSIC_SYNTAX' in completed.stderr, (mode, completed.stderr)
                        assert f'source-c.crexx:{line}:'.encode() in completed.stderr, (mode, line, completed.stderr)
                        assert b'UNREACHED' not in completed.stdout and b'Assertion' not in completed.stderr

        if args.phase in ('all', 'refusals'):
            cases = (('value', 'address value', None, b'19.1'),
                     ('with', "address 'ENV' with banana", 'banana', b'25.5'),
                     ('input', "address 'ENV' with input banana", 'banana', b'25.6'),
                     ('input-append', "address 'ENV' with input append", 'append', b'25.6'),
                     ('output', "address 'ENV' with output banana", 'banana', b'25.7'),
                     ('error', "address 'ENV' with error banana", 'banana', b'25.14'),
                     ('append', "address 'ENV' with output append normal", 'normal', b'25.8'),
                     ('replace', "address 'ENV' with error replace normal", 'normal', b'25.9'),
                     ('stream', "address 'ENV' with input stream 'file'", "'file'", b'53.1'),
                     ('stem', "address 'ENV' with output stem 'items.'", "'items.'", b'53.2'),
                     ('dead-simple', "if 0 then address 'ENV' with output stem ITEMS", 'ITEMS', b'53.3'))
            output = application / 'prior.rxas'; prior = b'unchanged prior output\n'; output.write_bytes(prior)
            for name, content, token, code in cases:
                source = work / (name + '.crexx'); data = ('options levelc\n' + content + '\n').encode(); source.write_bytes(data)
                completed = compile_c(source, output, failure=True)
                diagnostic = completed.stdout + completed.stderr
                assert code in diagnostic, (name, diagnostic)
                if token:
                    offset, line, column = source_position(data, token.encode())
                    assert f'at line {line}, column {column} (byte {offset})'.encode() in diagnostic, (name, diagnostic)
                assert output.read_bytes() == prior and not Path(str(output) + '.llrxc-stage').exists(), (name, 'atomic failed output')

        if args.phase in ('all', 'contracts'):
            probe = host / 'address-contracts-linked.rxbin'
            shutil.copy2(backend / probe.name, probe)
            inspect_args = []
            if args.inspect:
                raw_inspect = args.inspect.resolve() / f'address-raw-{args.mode}-{args.ir}-{args.vm}'
                raw_inspect.mkdir(parents=True, exist_ok=True)
                inspect_args = [raw_inspect]
            completed = run('raw/source admission', [host_vm, '--no-autoload', probe, '-a', runtime, *inspect_args], cwd=host)
            assert re.fullmatch(rb'ADDRESS_CONTRACTS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
    print(f'ADDRESS_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
