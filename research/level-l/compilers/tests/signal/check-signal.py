#!/usr/bin/env python3
"""Retained SIGNAL transfer controls within the combined whole-SIGNAL suite.

One source image/lane serves normal/private/EXIT/fatal execution and readable
RXAS. The combined runner adds ON/OFF, typed conditions and native payload
controls. This script never builds products; compiler and assembler modes are separate.
"""
import argparse
import os
from pathlib import Path
import re
import runpy
import shlex
import shutil
import subprocess
import tempfile

ADDRESS = runpy.run_path(str(Path(__file__).resolve().parents[1] / 'address' / 'check-address.py'))
LABELS = runpy.run_path(str(Path(__file__).resolve().parents[1] / 'labels' / 'check-labels.py'))
called = ADDRESS['called']
unit = ADDRESS['unit']
source_position = ADDRESS['source_position']
block_map = LABELS['block_map']
linear_path = LABELS['linear_path']

MARK = 'rexxpool.rexxvariablepool.setsignalline'
UPPER = 'rexxclassicunicodeproperties.rexxclassic_unicode_upper'
STRING = 'rexxvalue.rexxvalue.asstring'
QUERY = 'rexxactivation.rexxactivationarguments.programexitrequested'


def at_line(body, symbol, line):
    current = None; result = []; offset = 0
    for row in body.splitlines(keepends=True):
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            if fields[4] == 'source-c.crexx':
                current = int(fields[5])
        if current == line:
            result.extend((offset + match.start(), offset + match.end(), match[1].strip())
                          for match in called(row, symbol))
        offset += len(row)
    return result


def exit_bypass(body, source):
    _, line, _ = source_position(source, b'signal value ESCAPE()')
    queries = at_line(body, QUERY, line)
    assert len(queries) == 1, 'one root query after the SIGNAL operand call'
    _, end, value = queries[0]
    tail = body[end:]
    branch = re.search(r'^\s*brt\s+([^,\s]+),' + re.escape(value) + r'\s*\n\s*br\s+(\S+)\s*$', tail, re.M)
    assert branch and not re.search(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s', tail[:branch.start()], re.M), 'operand query directly controls unwind'
    blocks = block_map(body)
    exiting = linear_path(branch[1], blocks); continuing = linear_path(branch[2], blocks)
    assert all(not called(exiting, name) for name in (STRING, UPPER, MARK)), 'operand EXIT bypasses conversion, fold and mark'
    assert re.search(r'^\s*ret\s+r\d+\s*$', exiting, re.M), 'true edge restores private state and returns owned packet'
    assert all(len(called(continuing, name)) == 1 for name in (STRING, UPPER, MARK)), 'false continuation alone starts the SIGNAL segment'


def generated_signal_mapping(body, source, search_owner, loop_owners):
    """Generated edges retain provenance and their authored mark's ownership."""
    assert not re.search(
        r'^\s*load\s+r\d+,"source-c\.crexx:(?:\d+:\d+| (?:generated code|entering)[^"\n]*)"\s*$',
        body, re.M), 'source metadata replaces unobserved eager location strings'
    records = []; blocks = {}; current = None; block = None
    for row in body.splitlines():
        label = re.fullmatch(r'(llb\d+):', row.strip())
        if label:
            block = label[1]; current = None; blocks[block] = []
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            current = (int(fields[2]), int(fields[3]), fields[4],
                       int(fields[5]), int(fields[6]), int(fields[7]))
        entry = (row, current)
        records.append(entry)
        if block is not None:
            blocks[block].append(entry)

    def owning_mark(needle):
        _, line, column = source_position(source, needle)
        span = ('source-c.crexx', line, column, column + len(needle.decode('utf-8')))
        matches = [entry for entry in records
                   if called(entry[0], MARK) and entry[1] is not None and entry[1][2:] == span]
        assert len(matches) == 1 and matches[0][1][0] > 0, ('one authored SIGNAL mark owns the generated segment', span)
        return matches[0][1][0], span

    def check(entry, clause, span):
        row, record = entry
        assert record is not None and record[0] == clause and record[1] & 2, ('generated SIGNAL work keeps its owning nonzero clause and generated flags', row, record, clause)
        assert record[2:] == span, ('generated SIGNAL work retains the exact authored span', row, record, span)

    clause, span = owning_mark(b'signal value CHOOSE()')
    tests = []
    for index, entry in enumerate(records):
        match = re.match(r'\s*seq\s+(r\d+),' + re.escape(search_owner) + r',"', entry[0])
        if match:
            check(entry, clause, span); tests.append((index, match[1]))
    assert len(tests) >= 3, 'the VALUE search has independently mapped candidate tests'
    for index, result in tests:
        branch = None
        for entry in records[index + 1:]:
            if re.fullmatch(r'llb\d+:', entry[0].strip()):
                break
            conditional = re.fullmatch(r'\s*brt\s+(llb\d+),' + re.escape(result) + r'\s*', entry[0])
            if conditional:
                check(entry, clause, span); branch = conditional[1]
            elif re.fullmatch(r'\s*br\s+llb\d+\s*', entry[0]):
                check(entry, clause, span)
        assert branch is not None, 'each strict search result keeps its mapped branch'
        for entry in blocks[branch]:
            if re.match(r'\s*(?:signal(?:origin)?\s|ret(?:\s|$))', entry[0]):
                break
            if re.fullmatch(r'\s*null\s+' + re.escape(search_owner) + r'\s*', entry[0]) or re.fullmatch(r'\s*br\s+llb\d+\s*', entry[0]):
                check(entry, clause, span)

    clause, span = owning_mark(b'signal CONTROL_DONE')
    start = next(index for index, entry in enumerate(records)
                 if called(entry[0], MARK) and entry[1][0] == clause and entry[1][2:] == span)
    released = []
    for entry in records[start + 1:]:
        release = re.fullmatch(r'\s*null\s+(r\d+)\s*', entry[0])
        if release and release[1] in loop_owners:
            check(entry, clause, span); released.append(release[1])
        if re.fullmatch(r'\s*br\s+llb\d+\s*', entry[0]):
            check(entry, clause, span)
            break
    assert released == list(reversed(loop_owners)), 'both loop releases retain the mapped SIGNAL transfer'


def shapes(text, source, target):
    data = source.read_bytes(); body = unit(text)
    pushes = list(re.finditer(r'^\s*sigpush\s+"([^"\n]+)"\s*$', body, re.M))
    installs = list(re.finditer(r'^\s*sigbrv\s+[^,\n]+,[^,\n]+,"([^"\n]+)"\s*$', body, re.M))
    assert pushes and installs and {item[1] for item in pushes} == {item[1] for item in installs}, 'save and branch-install the same source-frame signal tags'
    assert max(item.start() for item in pushes) < min(item.start() for item in installs), 'save every outer policy before any branch handler snapshots the stack'
    assert 'CSIG2' not in text and 'llcsignalprotocol' not in text, 'compiler-only table/proof has no runtime payload or import'
    assert not called(body, 'llctarget.status') and not re.search(r'^\s*exit\s', body, re.M), 'same unit keeps owned completion'
    assert not called(body, 'rexxclassicconfig.rexxclassicconfig.§factory'), 'SIGNAL does not construct configuration or activation'
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
    assert called(body, 'rexxclassiccontext.rexxclassiccontext.bindpool'), 'private activation has existing checked restoration'
    _, line, _ = source_position(data, b'signal value CHOOSE()')
    conversions = at_line(body, STRING, line); folded = at_line(body, UPPER, line); marks = at_line(body, MARK, line)
    assert len(conversions) == len(folded) == len(marks) == 1, 'one direct conversion, shared fold and live-pool mark per VALUE site'
    assert conversions[0][0] < folded[0][0] < marks[0][0], 'operand conversion/fold precede causing-line mark'
    sequences = []; first_test = None
    current = None; offset = 0
    for row in body.splitlines(keepends=True):
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            if fields[4] == 'source-c.crexx': current = int(fields[5])
        if current == line:
            match = re.match(r'\s*seq\s+(r\d+),(r\d+),"', row)
            if match:
                sequences.append(match[2])
                if first_test is None:
                    first_test = (offset + len(row), match[1])
        offset += len(row)
    assert len(sequences) >= 3 and len(set(sequences)) == 1, 'all candidate tests directly read one stable runtime-name owner'
    owner = sequences[0]
    assert owner != 'r0', 'dynamic-name owner cannot alias fatal event carrier'
    copies = re.findall(r'^\s*copy\s+r\d+,' + re.escape(owner) + r'\s*$', body, re.M)
    assert len(copies) <= 2, 'no whole-text COPY per candidate test'
    assert re.search(r'^\s*null\s+' + re.escape(owner) + r'\s*$', body, re.M), 'runtime-name owner has an explicit release'
    after_test = body[first_test[0]:]
    true_branch = re.search(r'^\s*brt\s+(llb\d+),' + re.escape(first_test[1]) + r'\s*$', after_test, re.M)
    assert true_branch, 'strict test result directly selects its true edge'
    successful = block_map(body)[true_branch[1]]
    released = re.search(r'^\s*null\s+' + re.escape(owner) + r'\s*$', successful, re.M)
    transfer = re.search(r'^\s*br\s+llb\d+\s*$', successful, re.M)
    assert released and transfer and released.start() < transfer.start(), 'successful edge clears runtime name before real source transfer'
    fatal = text[text.index('llerror1_1:'):]
    relay = re.search(r'^\s*signalorigin\s', fatal, re.M)
    assert relay and re.search(r'^\s*null\s+' + re.escape(owner) + r'\s*$', fatal[:relay.start()], re.M), 'fatal cleanup releases the stable runtime name before relay'
    assert re.search(r'^\s*seq\s+r\d+,r\d+,"1\.20"\s*$', body, re.M) and re.search(r'^\s*seq\s+r\d+,r\d+,"1\.2"\s*$', body, re.M), 'numeric-looking targets use strict text comparisons'
    _, outer_line, _ = source_position(data, b'do OUTER = 1 to 3')
    _, inner_line, _ = source_position(data, b'do INNER = 1 to 3 until TICK()')
    outer = at_line(body, 'llctarget.donew', outer_line); inner = at_line(body, 'llctarget.donew', inner_line)
    assert len(outer) == len(inner) == 1 and outer[0][2] != inner[0][2], 'nested loops have distinct stable owners'
    _, signal_line, _ = source_position(data, b'signal CONTROL_DONE')
    close_mark = at_line(body, MARK, signal_line)
    assert len(close_mark) == 1
    suffix = body[close_mark[0][1]:]
    branch = re.search(r'^\s*br\s+llb\d+\s*$', suffix, re.M)
    assert branch, 'successful SIGNAL exposes the natural landing as a real CFG edge'
    before = suffix[:branch.start()]
    order = [match[1] for match in re.finditer(r'^\s*null\s+(r\d+)\s*$', before, re.M)
             if match[1] in (outer[0][2], inner[0][2])]
    assert order == [inner[0][2], outer[0][2]], 'reverse DO cleanup precedes transfer without advance/UNTIL'
    assert not called(before, 'rexxclassiccontext.rexxclassiccontext.bindpool') and not re.search(r'^\s*setnum', before, re.M), 'successful SIGNAL retains current private/numeric state'
    generated_signal_mapping(body, data, owner, (outer[0][2], inner[0][2]))
    exit_bypass(body, data)


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
    args.work.mkdir(parents=True, exist_ok=True); commands = 0
    with tempfile.TemporaryDirectory(prefix='signal-', dir=args.work) as temporary:
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
            completed = subprocess.run([str(item) for item in command], cwd=cwd, env=env, capture_output=True, timeout=900)
            commands += 1; diagnostic = completed.stdout + completed.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
            assert (completed.returncode != 0 if failure else completed.returncode == 0), (label, completed.returncode, diagnostic[-6000:])
            if silent:
                assert not diagnostic, (label, diagnostic[-4000:])
            return completed

        def compile_c(source, output, *, failure=False):
            return run('compile ' + source.name, [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []),
                       '-i', runtime, '-o', output, source], failure=failure, silent=not failure, cwd=host)

        def inspect(path, name):
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, args.inspect / f'{name}-{args.mode}-{args.ir}-{args.vm}.rxas')

        if args.phase in ('all', 'source', 'host', 'fatal', 'shapes'):
            source = work / 'source-c.crexx'; data = (fixtures / source.name).read_bytes()
            source.write_bytes(data if args.mode == 'opt' else data.replace(b'\n', b'\r\n'))
            assembly = application / 'source.rxas'; compile_c(source, assembly)
            inspect(assembly, 'signal-source'); text = assembly.read_text()
            assert str(work) not in text and str(build) not in text, 'stable logical source identity'
            shapes(text, source, (backend / 'llctarget.rxas').read_text())
            if args.phase in ('all', 'source'):
                run('source assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), assembly], silent=True)
                linked = application / 'source-linked'
                run('source link', [tool('rxlink'), '-o', linked, assembly.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)], silent=True)
                completed = run('SIGNAL source', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
                assert completed.stdout.replace(b'\r\n', b'\n') == expected and not completed.stderr, (completed.stdout, completed.stderr, expected)
            if args.phase in ('all', 'host', 'fatal'):
                exported = application / 'source-exported.rxas'
                text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=signalfixture.run\2', text, count=1, flags=re.M)
                text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"signalfixture.run"', text, count=1, flags=re.M)
                assert headers == metadata == 1, 'one test-only exported entry0, no production ABI change'
                exported.write_text(text, encoding='utf-8'); inspect(exported, 'signal-exported')
                run('exported assemble', [tool('rxas'), '-n', exported], silent=True)
                imports = work / 'B caller imports'; imports.mkdir()
                for provider in runtime.glob('*.rxbin'):
                    shutil.copy2(provider, imports / provider.name)
                shutil.copy2(exported.with_suffix('.rxbin'), imports / 'signalfixture.rxbin')
                caller = application / 'signal-host'
                run('B host compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import', '-i', imports, '-o', caller, fixtures / 'host.crexx'], silent=True)
                run('B host assemble', [tool('rxas'), '-n', caller], silent=True)
                linked = application / 'signal-host-linked'; control = work / 'host.ctl'
                inputs = [caller.with_suffix('.rxbin'), exported.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) + 'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run('B host link', [tool('rxlink'), '-c', control], silent=True)
                if args.phase in ('all', 'host'):
                    completed = run('borrowed SIGNAL state/frame/EXIT', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                    assert re.fullmatch(rb'SIGNAL_HOST\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
                if args.phase in ('all', 'fatal'):
                    rows = source.read_text().splitlines()
                    activation_source = (fixtures.parents[4] / 'lib/rxfnsc/RexxActivationArguments.crexx').read_bytes()
                    procedure_line = source_position(activation_source, b'assembler signal "CLASSIC_SYNTAX", "RXC-LC-17.1: PROCEDURE')[1]
                    cases = (('missing', b'16.1', rows.index("  when MODE = 'missing' then signal value MISSING_HELPER()") + 1, b'SIGNAL_ACTUAL|1\n'),
                             ('grouped', b'16.2', rows.index("  when MODE = 'grouped' then signal value 'dyn_group'") + 1, b''),
                             ('blank', b'16.1', rows.index("  when MODE = 'blank' then signal value 'EXACT '") + 1, b''),
                             ('nul', b'16.1', rows.index("  when MODE = 'nul' then signal value 'A' || '00'x || 'B'") + 1, b''),
                             ('procedure', b'17.1', procedure_line, b''))
                    for mode, code, line, actual in cases:
                        completed = run('unhandled ' + mode, [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', mode], failure=True, cwd=application)
                        assert completed.stdout.replace(b'\r\n', b'\n') == b'SIGNAL_FATAL|before\n' + actual, (mode, completed.stdout)
                        filename = 'RexxActivationArguments.crexx' if mode == 'procedure' else 'source-c.crexx'
                        assert b'CLASSIC_SYNTAX' in completed.stderr and code in completed.stderr and f'{filename}:{line}:'.encode() in completed.stderr, (mode, completed.stderr)
                        assert b'UNREACHED' not in completed.stdout
                        if mode == 'nul':
                            assert b'A\\x00B' in completed.stderr, completed.stderr

        if args.phase in ('all', 'refusals'):
            cases = (('bare', 'signal', 'signal', b'19.4'),
                     ('missing-value', 'if 0 then signal value', 'value', b'19.4'),
                     ('extra-direct', 'signal P extra', 'extra', b'19.4'),
                     ('unicode-unary', "if '雪' = '雪' then signal +", '+', b'35.1'),
                     ('static-grouped', 'if 0 then signal P; do; P: nop; end; P: return', 'P', b'16.2'))
            output = application / 'prior.rxas'; prior = b'unchanged prior output\n'; output.write_bytes(prior)
            for name, content, token, code in cases:
                source = work / (name + '.crexx'); data = ('options levelc\n' + content + '\n').encode(); source.write_bytes(data)
                completed = compile_c(source, output, failure=True)
                offset, line, column = source_position(data, token.encode())
                assert code in completed.stdout and not completed.stderr, (name, completed.stdout, completed.stderr)
                assert f'at line {line}, column {column} (byte {offset})'.encode() in completed.stdout, (name, completed.stdout)
                assert output.read_bytes() == prior and not Path(str(output) + '.llrxc-stage').exists(), 'failed SIGNAL publication remains atomic'

        if args.phase in ('all', 'contracts'):
            probe = host / 'signal-contracts-linked.rxbin'; shutil.copy2(backend / probe.name, probe)
            completed = run('SIGNAL AST/IR/source contracts', [host_vm, '--no-autoload', probe, '-a', runtime], cwd=host)
            assert re.fullmatch(rb'SIGNAL_CONTRACTS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
    print(f'SIGNAL_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
