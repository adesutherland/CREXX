#!/usr/bin/env python3
"""Independent practical C21 TRACE checks over prepared products; never builds.

One source image serves options, values, noninteractive counts, errors and the
ordinary B environment/borrowed-context controls. Partial event coverage is
permitted; retained values are checked against independent authored expectations.
An explicit prepared directory may reuse reviewed RXAS after an adapter-only
repair; exact source bytes must match, and all execution/oracle checks still run.
"""
import argparse
import json
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
source_position = ADDRESS['source_position']


def unit(text):
    """The source body is distinct from its generated trace/error helpers."""
    start = re.search(r'^llf1\(\) \.locals=', text, re.M)
    assert start, 'one shared source-unit entry'
    following = re.search(r'^\S+\(\) \.locals=', text[start.end():], re.M)
    end = start.end() + following.start() if following else len(text)
    return text[start.start():end]


def unescape(text):
    def replace(match):
        if match[1]:
            return chr(int(match[1], 16))
        return {'n': '\n', 'r': '\r', 't': '\t', 'f': '\f', '\\': '\\', '"': '"'}[match[2]]
    return re.sub(r'\\(?:x([0-9A-Fa-f]{2})|([nrtf\\"]))', replace, text)


def records(completed):
    """Separate ordinary stdout from standard TRACE records on either sink."""
    plain = []; events = []
    for data, is_error in ((completed.stdout, False), (completed.stderr, True)):
        current = None
        for raw in data.replace(b'\r\n', b'\n').splitlines(keepends=True):
            line = raw.rstrip(b'\n').decode('utf-8')
            source = re.fullmatch(r'\s*(\d+) \*-\* (.*)', line)
            value = re.fullmatch(r'\s*(>=>|>V>|>L>|>O>|>P>|>F>|>C>|>>>|>\.>)\s+"(.*)"', line)
            message = re.fullmatch(r'\s*\+\+\+\s+(.*)', line)
            command = re.fullmatch(r'\s*\*-\* (.*)', line)
            file = re.fullmatch(r'\s*>  >\s+(.*)', line)
            if source:
                current = int(source[1]); events.append(('source', current, unescape(source[2])))
            elif value:
                assert current is not None, ('value has a preceding source clause', line)
                events.append((value[1], current, unescape(value[2])))
            elif message:
                events.append(('message', current, unescape(message[1])))
            elif command:
                events.append(('command', current, unescape(command[1])))
            elif file:
                events.append(('file', current, unescape(file[1])))
            else:
                assert not is_error, ('unexpected stderr', raw)
                plain.append(raw)
    return b''.join(plain), events


def source_line(source, needle):
    return source_position(source.read_bytes(), needle.encode())[1]


def value_oracle(source, events):
    payload = '雪é\0tail '
    expected = {
        "PAYLOAD = '雪é'": {'雪é', '00', '\0', '雪é\0', 'tail ', payload},
        'COPY = PAYLOAD': {payload},
        "say 'VALUE|'": {'VALUE|', payload, 'VALUE|' + payload},
        'N = 1': {'1'},
        'do J = 1 to 2': {'1', '2', '3'},
        '  N = N + J': {'1', '2', '4'},
        'if N = 4 then do': {'4', '1'},
        "  BRANCH = 'right'": {'right'},
        "say 'FLOW|'": {'FLOW|', '4', '|', 'right', 'FLOW|4', 'FLOW|4|', 'FLOW|4|right'},
    }
    allowed = {source_line(source, needle): values for needle, values in expected.items()}
    source_rows = source.read_text().splitlines()
    for line, text in enumerate(source_rows, 1):
        if text == 'trace off':
            allowed[line] = {'off'}
        if text == 'trace intermediates':
            allowed[line] = {'intermediates'}
    sources = []; values = []
    for kind, line, value in events:
        if kind == 'source':
            assert 1 <= line <= len(source_rows), ('authored source line', line)
            assert value == source_rows[line - 1], ('truthful complete source text', line, value, source_rows[line - 1])
            sources.append(line)
        elif kind.startswith('>') and kind != 'file':
            assert line in allowed and value in allowed[line], ('actual value at authored clause', kind, line, repr(value), allowed.get(line))
            values.append((kind, line, value))
        elif kind == 'file':
            assert value == source.name, ('stable logical source identity', value)
        else:
            assert False, ('unexpected value-lane record', kind, line, value)
    payload_line = source_line(source, "PAYLOAD = '雪é'")
    copy_line = source_line(source, 'COPY = PAYLOAD')
    loop_line = source_line(source, '  N = N + J')
    assert any(kind == '>=>' and value == payload and line in (payload_line, copy_line) for kind, line, value in values), 'representative assignment displays exact Unicode/NUL/trailing-space value'
    assert any(kind == '>V>' and value == payload for kind, line, value in values), 'representative read displays the actual RexxValue text'
    assert sources.count(loop_line) == 2, ('executed loop body source appears on each entry', sources)
    assert source_line(source, "else BRANCH = 'wrong'") not in sources, 'unreached branch emits no source/value event'
    assert source_line(source, "say 'QUIET|done'") not in sources, 'Off has no breakpoint delivery'


def shapes(text, source, disabled, target, lean_query):
    body = unit(text); quiet = unit(disabled)
    assert called(body, 'rexxactivation.rexxactivationarguments.settraceoption'), 'instruction uses the sole activation trace parser'
    assert called(body, 'rexxclassicbiftrace.rexxclassicbif_trace_frame'), 'BIF retains its existing activation binding'
    assert re.search(r'^\s*metalinkpreg\s', text, re.M), 'generated handler reads event-named registers in the interrupted frame'
    assert re.search(r'^__rxtrace_handler\(\) \.locals=\d+', text, re.M) and not re.search(r'^\s*\.meta\s+"__rxtrace_handler"\s*=', text, re.M), 'private handler keeps its module-local body without a conflicting global callable descriptor'
    assert '.traceevent ' in text and re.search(r'^\s*\.srcstep\s+\d+\s+[1-9]\d*\s', body, re.M), 'authored nonzero clause identities and retained semantic events'
    assert not re.search(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s+[^\n]*llc(?:trace|clause|procedure|do)protocol', text, re.M | re.I), 'compiler proofs do not execute in the target'
    begins = called(body, 'rexxactivation.rexxactivationarguments.beginclausetime')
    assert len(begins) == 1, 'only the authored TIME operand requests its clause clock'
    assert not re.search(r'^\s*(?:bpon|bpoff|metalinkpreg|sigcall|sigsave|sigrestore)\b', quiet, re.M), 'plain constant source has no trace-only VM operations'
    assert not re.search(r'(?:_trace_|tracecontroller|tracecontext|settraceoption|tracesetting)', disabled, re.I), 'plain source has no trace-only imports/setup/value conversions'
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
    if lean_query:
        query_line = source_line(source, "say 'STATE|bare='")
        current = None; rows = []
        for row in body.splitlines():
            if row.lstrip().startswith('.srcstep '):
                fields = shlex.split(row)
                current = int(fields[5]) if fields[4] == source.name else None
            if current == query_line:
                rows.append(row)
        query = '\n'.join(rows)
        assert called(query, 'rexxclassicbiftrace.rexxclassicbif_trace_frame'), 'selected zero-argument TRACE query remains an ordinary BIF call'
        assert not called(query, 'rxfnsb._trace_set') and not called(query, 'rxfnsb._trace_mode_from_option'), 'proved omitted TRACE operand does not resynchronize unchanged controller state'
    # Follow the declared EXIT continuation rather than register-number goldens.
    line = source_line(source, 'trace value ESCAPE()'); current = None; query_registers = []
    query_symbol = 'rexxactivation.rexxactivationarguments.programexitrequested'
    for row in body.splitlines():
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            if fields[4] == source.name:
                current = int(fields[5])
        if current == line:
            query_registers.extend(item[1].strip() for item in called(row, query_symbol))
    assert len(query_registers) == 1, 'TRACE VALUE has one operand-EXIT query'
    query = [item for item in called(body, query_symbol) if item[1].strip() == query_registers[0]]
    assert len(query) == 1
    tail = body[query[0].end():]
    branch = re.search(r'^\s*brt\s+(\S+),' + re.escape(query_registers[0]) + r'\s*\n\s*br\s+(\S+)\s*$', tail, re.M)
    assert branch, 'root EXIT controls explicit setter/unwind continuations'
    labels = list(re.finditer(r'^(llb\d+):\s*$', body, re.M))
    blocks = {label[1]: body[label.end():labels[i + 1].start() if i + 1 < len(labels) else len(body)] for i, label in enumerate(labels)}
    exiting = blocks[branch[1]]
    assert not called(exiting, 'rexxactivation.rexxactivationarguments.settraceoption'), 'operand EXIT bypasses TRACE state update'
    assert re.search(r'^\s*ret\s+r\d+\s*$', exiting, re.M), 'operand EXIT performs ordinary owned completion'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
    parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
    parser.add_argument('--ir', choices=('on', 'off'), default='on')
    parser.add_argument('--phase', choices=('all', 'source', 'extensions', 'host', 'fatal', 'arithmetic-fatal', 'refusals', 'contracts', 'shapes'), default='all')
    parser.add_argument('--inspect', type=Path)
    parser.add_argument('--prepared', type=Path)
    args = parser.parse_args()
    if args.prepared:
        assert (args.mode, args.ir, args.vm) == ('opt', 'on', 'rxbvm'), 'prepared reuse is restricted to its qualified compiler/IR/VM identity'
        args.prepared = args.prepared.resolve()
    fixtures = Path(__file__).resolve().parent; build = args.build.resolve()
    backend = build / 'research/level-l/compilers/backend' / args.mode
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    args.work.mkdir(parents=True, exist_ok=True); commands = 0
    with tempfile.TemporaryDirectory(prefix='trace-', dir=args.work) as temporary:
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
        env = dict(os.environ, PATH=str(empty_path), CREXX_TRACE='R', CREXX_TRACE_TO='stdout')
        env.pop('CREXX_PROVIDER_PATH', None)
        assert not any(shutil.which(name, path=env['PATH']) for name in ('rxc', 'rxc-c'))

        def run(label, command, *, failure=False, silent=False, data=b'', cwd=work):
            nonlocal commands
            completed = subprocess.run([str(item) for item in command], cwd=cwd, env=env,
                                       input=data, capture_output=True, timeout=900)
            commands += 1; diagnostic = completed.stdout + completed.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
            assert (completed.returncode != 0 if failure else completed.returncode == 0), (label, completed.returncode, diagnostic[-6000:])
            if silent:
                assert not diagnostic, (label, diagnostic[-4000:])
            return completed

        def compile_c(source, output, *, failure=False, providers=()):
            if args.prepared and not failure and source.name in ('source-c.crexx', 'disabled-c.crexx'):
                assert not providers, 'prepared main/disabled inputs have no external provider binding'
                prepared_source = args.prepared / source.name
                assert prepared_source.read_bytes() == source.read_bytes() == (fixtures / source.name).read_bytes(), 'prepared RXAS requires byte-identical maintained source'
                name = 'source.rxas' if source.name == 'source-c.crexx' else 'disabled.rxas'
                shutil.copy2(args.prepared / name, output)
                return None
            return run('compile ' + source.name, [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []),
                       '-i', runtime, '-o', output, *providers, source], failure=failure, silent=not failure, cwd=host)

        def inspect(path, name):
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, args.inspect / f'{name}-{args.mode}-{args.ir}-{args.vm}.rxas')

        if args.phase in ('all', 'source', 'extensions', 'host', 'fatal', 'arithmetic-fatal', 'shapes'):
            source = work / 'source-c.crexx'; data = (fixtures / source.name).read_bytes()
            source.write_bytes(data if args.mode == 'opt' else data.replace(b'\n', b'\r\n'))
            assembly = application / 'source.rxas'; compile_c(source, assembly)
            inspect(assembly, 'trace-source'); text = assembly.read_text()
            assert str(work) not in text and str(build) not in text, 'stable logical source identity'
            disabled_source = work / 'disabled-c.crexx'; shutil.copy2(fixtures / disabled_source.name, disabled_source)
            disabled = application / 'disabled.rxas'; compile_c(disabled_source, disabled)
            inspect(disabled, 'trace-disabled')
            shapes(text, source, disabled.read_text(), (backend / 'llctarget.rxas').read_text(), args.mode == 'opt' and args.ir == 'on')
            if args.phase in ('all', 'source', 'extensions', 'fatal', 'arithmetic-fatal'):
                run('source assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), assembly], silent=True)
                linked = application / 'source-linked'
                run('source link', [tool('rxlink'), '-o', linked, assembly.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)], silent=True)
                image = linked.with_suffix('.rxbin')
                if args.phase in ('all', 'source'):
                    completed = run('TRACE state and call isolation', [target_vm, '--no-autoload', image], cwd=application)
                    plain, events = records(completed)
                    assert plain == (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n'), (plain, completed.stderr)
                    completed = run('TRACE actual Unicode/NUL values', [target_vm, '--no-autoload', image, '-a', 'values'], cwd=application)
                    plain, events = records(completed)
                    assert plain == 'VALUE|雪é\0tail \nFLOW|4|right\nQUIET|done\n'.encode(), plain
                    value_oracle(source, events)
                    completed = run('TRACE noninteractive counts and intact input', [target_vm, '--no-autoload', image, '-a', 'counts'], data=b'control-sentinel\n', cwd=application)
                    plain, events = records(completed)
                    assert plain == b'COUNTS|kept-one|kept-two|CONTROL-SENTINEL\n', plain
                    source_lines = [line for kind, line, value in events if kind == 'source']
                    assert source_line(source, "A = 'kept-one'") in source_lines and source_line(source, "B = 'kept-two'") in source_lines, 'negative count does not enact suppression'
                    assert not any(kind == 'message' for kind, line, value in events), 'interactive flag causes no prompt'
                    completed = run('TRACE Labels accepted and quiet', [target_vm, '--no-autoload', image, '-a', 'labels'], cwd=application)
                    plain, events = records(completed)
                    assert plain == b'LABELS|quiet\n' and not events, (plain, events)
                if args.phase in ('all', 'source', 'extensions'):
                    completed = run('TRACE ASM actual decoded instructions', [target_vm, '--no-autoload', image, '-a', 'asm'], cwd=application)
                    assert not completed.stderr and len(completed.stdout) < 250000, 'small ASM lane remains bounded and uses its selected sink'
                    lines = completed.stdout.replace(b'\r\n', b'\n').splitlines()
                    assert lines[-1] == b'EXTENSION|asm' and 1 <= len(lines[:-1]) <= 1500, lines[-5:]
                    decoded = [re.fullmatch(rb'\s*[0-9A-F]+\s+@\s+[0-9A-F]+:[0-9A-F]+\s+([A-Z][A-Z0-9_]*)\s+.*\*.*', line, re.I) for line in lines[:-1]]
                    assert all(decoded), [line for line, matched in zip(lines[:-1], decoded) if not matched][:5]
                    assert any(matched[1].lower().startswith(b'load') and b'"asm-actual"' in line for line, matched in zip(lines[:-1], decoded)), 'ASM decodes the actual authored literal load, including assembler fusion'
                    completed = run('TRACE LLM valid JSON and truthful origins', [target_vm, '--no-autoload', image, '-a', 'llm'], cwd=application)
                    assert not completed.stderr and len(completed.stdout) < 250000, 'small LLM lane remains bounded and uses its selected sink'
                    lines = completed.stdout.replace(b'\r\n', b'\n').splitlines()
                    assert lines[-1] == b'EXTENSION|llm' and 1 <= len(lines[:-1]) <= 1500, lines[-5:]
                    objects = [json.loads(line) for line in lines[:-1]]
                    source_rows = source.read_text().splitlines()
                    for event in objects:
                        assert event['event'] == 'trace' and event['format'] == 'llm' and event['mode'] == 'LLM', event
                        assert event['file'] == source.name and event['asm'], event
                        line = int(event['line'])
                        assert 1 <= line <= len(source_rows) and event['source'] == source_rows[line - 1], event
                    payload_line = source_line(source, "  LLMPAYLOAD = '雪'")
                    assert any(int(event['line']) == payload_line and event['source'] == "  LLMPAYLOAD = '雪'" for event in objects), 'LLM preserves the exact Unicode source line and logical filename'
                    for mode in ('caught', 'caught-bif'):
                        completed = run('inherited child handler and parent observer restoration ' + mode, [target_vm, '--no-autoload', image, '-a', mode], cwd=application)
                        plain, events = records(completed)
                        assert plain == b'CAUGHT|L|1|untouched|SYNTAX\nAFTER_CHILD|R|1|untouched\n', (mode, plain)
                if args.phase in ('all', 'fatal', 'arithmetic-fatal'):
                    cases = (('invalid', 'trace value BADOPTION()', b'24.1', b'TRACE_ACTUAL|1\n'),
                             ('scan', "trace value 'scan'", b'24.1', b''),
                             ('bif-invalid', "BAD = trace('scan')", b'40.28', b''),
                             ('bif-count', "BAD = trace('O','R')", b'40.4', b''),
                             ('fatal-value', 'VALUE = 1 / 0', b'42.3', None))
                    for mode, needle, code, expected in cases:
                        if args.phase == 'arithmetic-fatal' and mode != 'fatal-value':
                            continue
                        completed = run('fatal ' + mode, [target_vm, '--no-autoload', image, '-a', mode], failure=True, cwd=application)
                        assert b'CLASSIC_SYNTAX' in completed.stderr and code in completed.stderr, (mode, completed.stderr)
                        line = source_line(source, needle)
                        if expected is not None:
                            assert f'source-c.crexx:{line}:'.encode() in completed.stderr, (mode, line, completed.stderr)
                            assert completed.stdout.replace(b'\r\n', b'\n') == expected, (mode, completed.stdout)
                        else:
                            # The agreed SIGNAL boundary preserves the actual
                            # helper's raw origin. TRACE separately observes the
                            # authored failing clause before execution.
                            assert b'source: RexxClassicBifs.crexx:' in completed.stderr and re.search(rb'at module \d+ \(RexxClassicBifs\) address \d+', completed.stderr), completed.stderr
                            authored = source.read_text().splitlines()[line - 1].encode()
                            assert re.search(rb'^\s*' + str(line).encode() + rb' \*-\* ' + re.escape(authored) + rb'\r?$', completed.stdout, re.M) and b'"bad"' not in completed.stdout, completed.stdout
                        assert b'UNREACHED' not in completed.stdout and b'Assertion' not in completed.stderr
            if args.phase in ('all', 'host'):
                exported = application / 'source-exported.rxas'
                text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=tracefixture.run\2', text, count=1, flags=re.M)
                text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"tracefixture.run"', text, count=1, flags=re.M)
                assert headers == metadata == 1, 'one test-only exported entry, no product ABI change'
                exported.write_text(text, encoding='utf-8'); inspect(exported, 'trace-exported')
                run('exported assemble', [tool('rxas'), '-n', exported], silent=True)
                provider = application / 'traceprobe'
                run('prepare tiny signed B provider', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import', '-i', runtime, '-o', provider, fixtures / 'provider-b.crexx'], silent=True)
                run('signed B provider assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), provider], silent=True)
                shutil.copy2(provider.with_suffix('.rxbin'), runtime / 'traceprobe.rxbin')
                external_source = work / 'external-c.crexx'; shutil.copy2(fixtures / external_source.name, external_source)
                external = application / 'external.rxas'
                compile_c(external_source, external, providers=('--provider', 'traceprobe', 'traceprobe.observe'))
                external_text = external.read_text(); external_body = unit(external_text)
                provider_calls = called(external_body, 'traceprobe.observe')
                assert len(provider_calls) == 1, 'one ordinary signed provider invocation'
                following = external_body[provider_calls[0].end():]
                after_line = source_line(external_source, 'AFTER = RESULT')
                next_source = re.search(r'^\s*\.srcstep\s+\d+\s+\d+\s+\d+\s+"external-c\.crexx"\s+' + str(after_line) + r'\s', following, re.M)
                assert next_source and called(following[:next_source.start()], 'rxfnsb._trace_set'), 'foreign provider resynchronizes parent state before its next authored operation'
                assert re.search(r'^\s*sigpop\s+"BREAKPOINT"', following, re.M), 'foreign-only source restores enclosing BREAKPOINT policy on completion'
                external_text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=externaltracefixture.run\2', external_text, count=1, flags=re.M)
                external_text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"externaltracefixture.run"', external_text, count=1, flags=re.M)
                assert headers == metadata == 1, 'one test-only foreign caller entry'
                external.write_text(external_text, encoding='utf-8'); inspect(external, 'trace-external')
                run('foreign caller assemble', [tool('rxas'), '-n', external], silent=True)
                imports = work / 'B caller imports'; imports.mkdir()
                for provider in runtime.glob('*.rxbin'):
                    shutil.copy2(provider, imports / provider.name)
                shutil.copy2(exported.with_suffix('.rxbin'), imports / 'tracefixture.rxbin')
                shutil.copy2(external.with_suffix('.rxbin'), imports / 'externaltracefixture.rxbin')
                caller = application / 'trace-host'
                run('B host compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import', '-i', imports, '-o', caller, fixtures / 'host.crexx'], silent=True)
                run('B host assemble', [tool('rxas'), '-n', caller], silent=True)
                linked = application / 'trace-host-linked'; control = work / 'host.ctl'
                inputs = [caller.with_suffix('.rxbin'), exported.with_suffix('.rxbin'), external.with_suffix('.rxbin'), runtime / 'traceprobe.rxbin', *(runtime / (name + '.rxbin') for name in libraries)]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) + 'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run('B host link', [tool('rxlink'), '-c', control], silent=True)
                completed = run('borrowed state/inhibition/EXIT', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                plain, events = records(completed)
                assert re.fullmatch(rb'TRACE_HOST\|checks=\d+\|failures=0\n', plain) and not events, (plain, events)
                for option in ('C', 'N', 'E', 'F'):
                    completed = run('coarse command option ' + option, [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', 'commands', option], cwd=application)
                    plain, events = records(completed)
                    assert re.fullmatch(rb'TRACE_HOST\|checks=\d+\|failures=0\n', plain), plain
                    before = [value for kind, line, value in events if kind == 'command']
                    after = [value for kind, line, value in events if kind == 'message']
                    assert before == (['ok', 'positive', 'negative', 'failure'] if option == 'C' else []), (option, before)
                    expected = ['RC=-3 TRACE-TEST negative', 'RC=8 TRACE-TEST failure']
                    if option != 'F':
                        expected.insert(0, 'RC=7 TRACE-TEST positive')
                    assert after == expected, (option, after, expected)

        if args.phase in ('all', 'refusals'):
            cases = (('scan-static', 'trace scan', 'scan', b'24.1'),
                     ('invalid-static', 'trace zebra', 'zebra', b'24.1'),
                     ('missing-value', 'trace value', 'value', b'35.1'),
                     ('bad-sign', 'trace +', '+', b'19.6'),
                     ('extra-static', 'trace r extra', 'extra', b'19.6'),
                     ('dead-invalid', 'if 0 then trace scan', 'scan', b'24.1'),
                     ('unicode-operand', "if '雪' = '雪' then trace value 1 +", '+', b'35.1'))
            output = application / 'prior.rxas'; prior = b'unchanged prior output\n'; output.write_bytes(prior)
            for name, content, token, code in cases:
                source = work / (name + '.crexx'); data = ('options levelc\n' + content + '\n').encode(); source.write_bytes(data)
                completed = compile_c(source, output, failure=True)
                offset, line, column = source_position(data, token.encode())
                diagnostic = completed.stdout + completed.stderr
                assert code in diagnostic and f'at line {line}, column {column} (byte {offset})'.encode() in diagnostic, (name, diagnostic)
                assert output.read_bytes() == prior and not Path(str(output) + '.llrxc-stage').exists(), 'failed TRACE publication remains atomic'

        if args.phase in ('all', 'contracts'):
            probe = host / 'trace-contracts-linked.rxbin'; shutil.copy2(backend / probe.name, probe)
            completed = run('TRACE AST/IR/source contracts', [host_vm, '--no-autoload', probe, '-a', runtime], cwd=host)
            assert re.fullmatch(rb'TRACE_CONTRACTS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
    print(f'TRACE_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
