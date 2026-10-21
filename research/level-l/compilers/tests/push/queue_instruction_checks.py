"""Shared harness for the independently delivered PUSH and QUEUE fixtures.

This is confined to queue insertion tests. It launches prepared tools only;
the source, expected output, host and refusals belong to each instruction.
Configured host failures, TRACE and platform delivery remain STEP07/08.
"""
import argparse
from collections import Counter
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tempfile


def calls(text, symbol):
    return list(re.finditer(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s+([^,\n]+),§?' +
                           re.escape(symbol) + r'\(\)(?:,|\s*$)', text, re.M))


def unit(text):
    assert re.findall(r'^llf\d+\(\)', text, re.M) == ['llf1()'], 'one shared source body'
    start = re.search(r'^llf1\(\) \.locals=', text, re.M)
    assert start, 'anchored source-unit definition'
    body = text[start.start():text.index('llerror1_1:')]
    assert 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int' in body
    return body


def position(data, offset):
    line = data[:offset].count(b'\n') + 1
    start = data.rfind(b'\n', 0, offset) + 1
    return line, len(data[start:offset].decode('utf-8')) + 1


def shapes(instruction, text, source):
    body = unit(text); name = instruction.lower()
    producer = 'rexxclassicconfig.rexxclassicconfig.' + name + 'text'
    assert calls(body, producer), 'existing config method owns queue insertion'
    assert not calls(body, 'rxfnsb.' + name), 'caller delegates through its live config'
    assert not re.search(r'^\s*(?:push|queue)\s', body, re.M), 'no duplicate caller queue algorithm'
    rows = source.read_text().splitlines()
    critical = {i for i, row in enumerate(rows, 1)
                if row.strip() in (name, name + ' NEXT()')}
    writes = Counter(); current = None; operand_rows = []
    operand_line = rows.index('  push EXITVALUE()') + 1 if instruction == 'PUSH' else None
    # A call's generated carrier/query records retain its proved clause, while
    # truthfully carrying no authored filename/line of their own.
    operand_clauses = set()
    if operand_line is not None:
        for row in body.splitlines():
            if row.lstrip().startswith('.srcstep '):
                fields = shlex.split(row)
                if fields[4] == 'source-c.crexx' and int(fields[5]) == operand_line:
                    if int(fields[2]):
                        operand_clauses.add(int(fields[2]))
        assert operand_clauses, 'operand call and PUSH retain clause ownership'
    operand_active = False
    for row in body.splitlines():
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            current = int(fields[5]) if fields[4] == 'source-c.crexx' else None
            operand_active = operand_line is not None and (current == operand_line or
                (fields[4] == '' and int(fields[2]) in operand_clauses))
        if calls(row, producer) and current in critical:
            writes[current] += 1
        if operand_active:
            operand_rows.append(row)
    assert critical and all(writes[line] == 1 for line in critical), ('one insertion for each bare/once-only source instruction', critical, writes)
    if operand_line is not None:
        operand = '\n'.join(operand_rows)
        guarded = calls(operand, 'rexxactivation.rexxactivationarguments.programexitrequested')
        inserted = calls(operand, producer)
        assert guarded and len(inserted) == 1 and guarded[-1].end() < inserted[0].start(), 'operand EXIT is checked before insertion'
        between = operand[guarded[-1].end():inserted[0].start()]
        branch = re.search(r'^\s*brt\s+([^,\s]+),' + re.escape(guarded[-1][1].strip()) + r'\s*$', between, re.M)
        assert branch, 'operand EXIT query result controls its true edge'
        false = re.search(r'^\s*br\s+(\S+)\s*$', between[branch.end():], re.M)
        assert false, 'operand EXIT guard has a separate normal continuation'

        def source_block(label):
            start = re.search(r'^' + re.escape(label) + r':\s*$', body, re.M)
            assert start, ('guard target is an emitted source block', label)
            following = re.search(r'^llb\d+:\s*$', body[start.end():], re.M)
            return body[start.end():start.end() + following.start()] if following else body[start.end():]

        unwind = source_block(branch[1]); continuation = source_block(false[1])
        assert not calls(unwind, producer) and re.search(r'^\s*ret\s+\S+\s*$', unwind, re.M), 'operand EXIT true target returns without insertion'
        assert len(calls(continuation, producer)) == 1, 'only the normal false continuation inserts'
    # Source columns count scalars, including the non-Latin1 operand; no
    # encoding through ordinal C2X is used to check this text.
    data = source.read_bytes(); literal = (name + " 'é🙂'").encode('utf-8')
    offset = data.index(literal); line, column = position(data, offset)
    ending = len(data.splitlines()[line - 1].decode('utf-8')) + 1
    assert re.search(r'^\s*\.srcstep\s+\d+\s+\d+\s+(?:17|65)\s+"source-c\.crexx"\s+' +
                     rf'{line}\s+{column}\s+{ending}\s+"', body, re.M), 'queue instruction retains original byte/scalar source mapping'
    assert not re.search(r'^\s*exit\s', body, re.M), 'queue insertion returns through the existing frame protocol'
    receivers = set(re.findall(r'^\s*link(?:ref)?\s+([^,\n]+),', body, re.M))
    assert receivers, 'config and current pool use checked scoped borrows'
    fatal = text[text.index('llerror1:'):]
    first_clear = re.search(r'^\s*null\s+', fatal, re.M)
    assert first_clear, 'finite fatal cleanup'
    detached = set(re.findall(r'^\s*unlink\s+(\S+)\s*$', fatal[:first_clear.start()], re.M))
    assert receivers <= detached, ('detach actual receiver locals before fatal NULL', receivers, detached)
    owners = {match[1].strip() for match in calls(body, 'llctarget.poolnew')}
    normal_restores = calls(body, 'rexxclassiccontext.rexxclassiccontext.bindpool')
    assert owners and len(normal_restores) >= 2, 'private entries install and restore the current binding'
    fatal_restores = calls(fatal, 'rexxclassiccontext.rexxclassiccontext.bindpool')
    releases = [match for match in re.finditer(r'^\s*null\s+(\S+)\s*$', fatal, re.M)
                if match[1] in owners]
    assert fatal_restores and releases and fatal_restores[0].end() < releases[0].start(), 'restore original live pool before private owner release'
    assert 'signalorigin' in fatal, 'fatal cleanup preserves original event/source'


def main(instruction, fixtures):
    assert instruction in ('PUSH', 'QUEUE')
    name = instruction.lower(); fixtures = Path(fixtures).resolve()
    parser = argparse.ArgumentParser(description=instruction + ' standalone source/host/RXAS checks; never builds.')
    parser.add_argument('--build', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
    parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
    parser.add_argument('--ir', choices=('on', 'off'), default='on')
    parser.add_argument('--phase', choices=('all', 'source', 'host', 'fatal', 'refusals', 'shapes', 'contracts'), default='all')
    parser.add_argument('--inspect', type=Path)
    args = parser.parse_args(); build = args.build.resolve()
    backend = build / 'research/level-l/compilers/backend' / args.mode
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    args.work.mkdir(parents=True, exist_ok=True); commands = 0
    with tempfile.TemporaryDirectory(prefix=name + '-', dir=args.work) as temporary:
        work = Path(temporary); runtime = work / 'runtime'; runtime.mkdir()
        for library in libraries:
            root = backend if library == 'llctarget' else build / 'bin'
            shutil.copy2(root / (library + '.rxbin'), runtime / (library + '.rxbin'))
        dlls = tuple((build / 'bin').glob('*.dll')) if os.name == 'nt' else ()

        def tool(program):
            return build / 'bin' / (program + suffix)

        def stage(label, plugins):
            directory = work / label; directory.mkdir()
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
        assert not any(shutil.which(program, path=env['PATH']) for program in ('rxc', 'rxc-c'))

        def run(label, command, *, failure=False, silent=False, data=b'', cwd=work):
            nonlocal commands
            result = subprocess.run([str(item) for item in command], cwd=cwd, env=env,
                                    input=data, capture_output=True, timeout=900)
            commands += 1; diagnostic = result.stdout + result.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
            assert (result.returncode != 0 if failure else result.returncode == 0), (label, result.returncode, diagnostic[-6000:])
            if silent:
                assert not diagnostic, (label, diagnostic[-4000:])
            return result

        def compile_c(source, output, *, failure=False):
            return run('compile ' + source.name,
                       [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []),
                        '-i', runtime, '-o', output, source], failure=failure, silent=not failure, cwd=host)

        def inspect(path, label):
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, args.inspect / f'{name}-{label}-{args.mode}-{args.ir}-{args.vm}.rxas')

        if args.phase in ('all', 'source', 'host', 'fatal', 'shapes'):
            source = work / 'source-c.crexx'; data = (fixtures / source.name).read_bytes()
            source.write_bytes(data.replace(b'\n', b'\r\n') if args.mode == 'noopt' else data)
            assembly = application / 'source.rxas'; compile_c(source, assembly)
            inspect(assembly, 'source'); text = assembly.read_text()
            assert str(work) not in text and str(build) not in text, 'stable logical source identity'
            shapes(instruction, text, source)
            if args.phase in ('all', 'source'):
                run('source assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), assembly], silent=True)
                linked = application / 'source-linked'
                run('source link', [tool('rxlink'), '-o', linked, assembly.with_suffix('.rxbin'),
                                    *(runtime / (library + '.rxbin') for library in libraries)], silent=True)
                result = run('ordering/count/null/text source control', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
                assert result.stdout.replace(b'\r\n', b'\n') == expected and not result.stderr, (result.stdout, result.stderr, expected)
            if args.phase in ('all', 'host', 'fatal'):
                exported = application / 'source-exported.rxas'
                text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', rf'\1 .expose={name}fixture.run\2', text, count=1, flags=re.M)
                text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', rf'\1"{name}fixture.run"', text, count=1, flags=re.M)
                assert headers == metadata == 1, 'one test-only exported entry0'
                exported.write_text(text, encoding='utf-8'); inspect(exported, 'exported')
                run('exported assemble', [tool('rxas'), '-n', exported], silent=True)
                imports = work / 'B caller imports'; imports.mkdir()
                for library in runtime.glob('*.rxbin'):
                    shutil.copy2(library, imports / library.name)
                shutil.copy2(exported.with_suffix('.rxbin'), imports / (name + 'fixture.rxbin'))
                caller = application / (name + '-host')
                run('B host compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import',
                                       '-i', imports, '-o', caller, fixtures / 'host.crexx'], silent=True)
                run('B host assemble', [tool('rxas'), '-n', caller], silent=True)
                linked = application / (name + '-host-linked'); control = work / 'host.ctl'
                inputs = [caller.with_suffix('.rxbin'), exported.with_suffix('.rxbin'), *(runtime / (library + '.rxbin') for library in libraries)]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) +
                                   'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run('B host link', [tool('rxlink'), '-c', control], silent=True)
                if args.phase in ('all', 'host'):
                    result = run('selected queue/full bytes/once-only/private alias controls',
                                 [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], data=b'UNEXPECTED FALLBACK\n', cwd=application)
                    expected = instruction.encode() + rb'_HOST\|checks=\d+\|failures=0\r?\n'
                    assert re.fullmatch(expected, result.stdout) and not result.stderr, (result.stdout, result.stderr)
                if args.phase in ('all', 'fatal'):
                    line = source.read_text().splitlines().index('LATE:') + 2
                    result = run('reached bare instruction consumes PROCEDURE eligibility',
                                 [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', 'late-bare'], failure=True, cwd=application)
                    assert result.stdout.replace(b'\r\n', b'\n') == instruction.encode() + b'_FATAL|before\n', result.stdout
                    assert b'17.1' in result.stderr and b'CLASSIC_SYNTAX' in result.stderr and f'source-c.crexx:{line}:'.encode() in result.stderr, (line, result.stderr)
                    assert b'UNREACHED' not in result.stdout and b'Assertion' not in result.stderr

        if instruction == 'PUSH' and args.phase in ('all', 'contracts'):
            probe = backend / 'llc-push-contracts-linked.rxbin'
            result = run('PUSH schema and fresh/loaded admission',
                         [host_vm, '--no-autoload', probe, '-a', runtime], cwd=host)
            assert re.fullmatch(rb'PUSH_CONTRACTS\|checks=\d+\|failures=0\r?\n', result.stdout) and not result.stderr, (result.stdout, result.stderr)

        if args.phase in ('all', 'refusals'):
            output = application / 'prior.rxas'; prior = b'unchanged prior output\n'; output.write_bytes(prior)
            cases = json.loads((fixtures / 'refusals.json').read_text())
            for index, case in enumerate(cases, 1):
                source = work / (name + '-invalid-' + str(index) + '.crexx')
                data = ('options levelc\n' + case['source'] + '\n').encode('utf-8'); source.write_bytes(data)
                result = compile_c(source, output, failure=True); diagnostic = result.stdout + result.stderr
                assert case['code'].encode() in diagnostic and source.name.encode() in diagnostic, (case['purpose'], diagnostic)
                location = re.search(rb'at line (\d+), column (\d+) \(byte (\d+)\)', diagnostic)
                assert location, (case['purpose'], 'source position is retained', diagnostic)
                line, column, offset = map(int, location.groups())
                assert data.rfind(name.encode()) <= offset <= len(data) and position(data, offset) == (line, column), (case['purpose'], 'original byte/scalar position', line, column, offset, diagnostic)
                assert output.read_bytes() == prior and not Path(str(output) + '.llrxc-stage').exists(), (case['purpose'], 'failed request is atomic')
    print(f'{instruction}_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')
