#!/usr/bin/env python3
"""Independent PROCEDURE source, borrowed-owner, refusal and RXAS checks.

Consumes prepared product/compiler images; does not rebuild shared targets.
The native B compiler is used only for the direct B caller fixture. All new C
source goes through the B/L compiler with native compilers absent from PATH.
Temporary outputs disappear on success or failure; --inspect is disposable.
"""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


def calls(text, symbol):
    return list(re.finditer(r'^\s*call\s+([^,\n]+),' + re.escape(symbol) + r'\(\),', text, re.M))


def unit_body(text):
    units = re.findall(r'^llf\d+\(\)', text, re.M)
    assert units == ['llf1()'], ('one source-unit procedure', units)
    match = re.search(r'^llf1\(\)[\s\S]*?(?=^llerror1_1:)', text, re.M)
    assert match, 'readable C unit and finite error suffix'
    body = match.group()
    assert 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int' in body, 'required context/entry metadata'
    return body


def check_consume_origin(text, source):
    """The context load and consume at SHARED's first ARG share one clause."""
    authored = b'arg X, OMITTED, EMPTY'
    offset = source.index(authored)
    line = source[:offset].count(b'\n') + 1
    blocks = re.findall(r'^llb\d+:\n([\s\S]*?)(?=^llb\d+:|^llerror1_1:|\Z)', text, re.M)
    pattern = r'^\.srcstep (\d+) (\d+) 17 "source-c\.crexx" (\d+) 1 \d+ "arg X, OMITTED, EMPTY"$'
    selected = [block for block in blocks
                if calls(block, 'rexxactivation.rexxactivationarguments.consumefirstinstruction')
                and re.search(pattern, block, re.M)]
    assert len(selected) == 1, 'one first-ARG eligibility edge'
    records = re.findall(pattern, selected[0], re.M)
    assert len(records) == 1, ('one authored record for context load and consume', records)
    origin, clause, authored_line = map(int, records[0])
    assert origin > 0 and clause == offset + 1 and authored_line == line, 'consume retains its authored ARG clause association'


def check_fatal_cleanup(text):
    """Cleanup restores bindings and preserves the raw event, tag and payload."""
    receivers = set(re.findall(r'^\s*link(?:ref)?\s+([^,\n]+),', unit_body(text), re.M))
    start = text.index('llerror1:')
    suffix = text[start:]
    first_clear = re.search(r'^\s*null\s+', suffix, re.M)
    assert first_clear and len(receivers) == 2, ('two borrowed receiver locals', receivers)
    detached = set(re.findall(r'^\s*unlink\s+([^\s]+)\s*$', suffix[:first_clear.start()], re.M))
    assert receivers <= detached, ('detach borrowed receivers before clearing fatal temporaries', receivers, detached)
    restored = calls(suffix, 'rexxclassiccontext.rexxclassiccontext.bindpool')
    assert not calls(suffix, 'llctarget.failuremessage'), 'relay the original payload without message formatting'
    relays = re.findall(r'^\s*signalorigin\s+"([A-Z_]+)",([^,\n]+),([^\n]+)', suffix, re.M)
    names = {'CLASSIC_SYNTAX', 'DIVISION_BY_ZERO', 'CONVERSION_ERROR', 'INVALID_ARGUMENTS',
             'OBJECT_NOT_INITIALIZED', 'REFERENCE_INVALID', 'UNICODE_ERROR', 'OUT_OF_RANGE', 'FAILURE'}
    assert {name for name, _, _ in relays} == names, ('all finite fatal relays', relays)
    events = {event.strip() for _, _, event in relays}
    assert len(events) == 1, ('same original raw event for every relay', events)
    event = next(iter(events))
    payloads = {payload.strip() for _, payload, _ in relays}
    assert len(payloads) == 1, ('same original payload for every relay', payloads)
    payload = next(iter(payloads))
    branch = re.search(r'^\s*beq\s+llraise1_1,([^,\n]+),1\s*$', suffix, re.M)
    assert branch, 'saved discriminator selects the original fatal name'
    tag = branch.group(1).strip()
    capture = re.search(r'^\s*linkattr1\s+([^,\n]+),' + re.escape(event) + r',5\s*\n'
                        r'\s*linkattr1\s+([^,\n]+),\1,5\s*\n'
                        r'\s*copy\s+' + re.escape(payload) + r',\2\s*\n'
                        r'\s*unlink\s+\2\s*\n\s*unlink\s+\1\s*\n'
                        r'\s*null\s+\2\s*\n\s*null\s+\1\s*$', suffix, re.M)
    assert restored and capture and restored[0].start() < capture.start(), 'restore binding before copying the original raw payload'
    assert len({event, payload, tag, *capture.groups()}) == 5, 'raw payload transport uses separate borrowed and owned cells'
    before_clear = suffix[:first_clear.start()]
    saved_events = re.findall(r'^\s*copy\s+([^,\n]+),' + re.escape(event) + r'\s*$', before_clear, re.M)
    saved_tags = re.findall(r'^\s*copy\s+([^,\n]+),' + re.escape(tag) + r'\s*$', before_clear, re.M)
    assert len(saved_events) == len(saved_tags) == 1, 'save the original event and discriminator before cleanup'
    saved_event = saved_events[0].strip(); saved_tag = saved_tags[0].strip()
    after_restore = suffix[restored[0].end():capture.start()]
    for destination, saved in ((event, saved_event), (tag, saved_tag)):
        assert re.search(r'^\s*copy\s+' + re.escape(destination) + ',' + re.escape(saved) + r'\s*$', after_restore, re.M), ('restore the saved fatal value after binding cleanup', destination)
    before_capture = suffix[:capture.start()]
    for match in re.finditer(r'^\s*call\s+([^,\n]+),', before_capture, re.M):
        assert match.group(1).strip() not in (event, tag, saved_event, saved_tag), ('cleanup overwrites saved event/discriminator', match.group())
    assert suffix.index('signalorigin') > capture.end(), 'cleanup and raw payload copy precede relay'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
    parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
    parser.add_argument('--ir', choices=('on', 'off'), default='on')
    parser.add_argument('--phase', choices=('all', 'source', 'refusals', 'fatal', 'shapes', 'context', 'contracts'), default='all')
    parser.add_argument('--inspect', type=Path, help='disposable readable RXAS review destination')
    args = parser.parse_args()
    build = args.build.resolve()
    backend = build / 'research/level-l/compilers/backend' / args.mode
    fixtures = Path(__file__).resolve().parent
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    args.work.mkdir(parents=True, exist_ok=True)
    commands = 0

    with tempfile.TemporaryDirectory(prefix='procedure-', dir=args.work) as temporary:
        work = Path(temporary)
        runtime = work / 'runtime'
        runtime.mkdir()
        for name in libraries:
            source = backend if name == 'llctarget' else build / 'bin'
            shutil.copy2(source / (name + '.rxbin'), runtime / (name + '.rxbin'))
        dlls = tuple((build / 'bin').glob('*.dll')) if os.name == 'nt' else ()

        def tool(name):
            return build / 'bin' / (name + suffix)

        def stage(directory, providers):
            directory.mkdir()
            vm = directory / (args.vm + suffix)
            shutil.copy2(tool(args.vm), vm)
            for dll in dlls:
                shutil.copy2(dll, directory / dll.name)
            provider_dir = directory / 'providers'
            provider_dir.mkdir()
            for provider in providers:
                name = provider + '.rxplugin'
                source = build / 'bin' / name
                if not source.exists():
                    source = build / 'bin' / 'providers' / name
                shutil.copy2(source, provider_dir / name)
            return vm

        host = work / 'C compiler host'
        host_vm = stage(host, ('rxfs', 'rxcontract'))
        compiler = host / 'llrxc-c-linked.rxbin'
        shutil.copy2(backend / compiler.name, compiler)
        application = work / 'static application'
        target_vm = stage(application, ('rxfs', 'rxcstream'))
        empty_path = work / 'empty PATH'
        empty_path.mkdir()
        env = dict(os.environ, PATH=str(empty_path))
        env.pop('CREXX_PROVIDER_PATH', None)
        assert not any(shutil.which(name, path=env['PATH']) for name in ('rxc', 'rxc-c'))

        def run(label, command, *, failure=False, silent=False, cwd=work):
            nonlocal commands
            completed = subprocess.run([str(part) for part in command], cwd=cwd, env=env,
                                       capture_output=True, timeout=900)
            commands += 1
            diagnostic = completed.stdout + completed.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
            assert (completed.returncode != 0 if failure else completed.returncode == 0), (label, completed.returncode, diagnostic[-6000:])
            if silent:
                assert not diagnostic, (label, diagnostic[-4000:])
            return completed

        def compile_c(source, destination, *, failure=False):
            return run('compile ' + source.name,
                       [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []),
                        '-i', runtime, '-o', destination, source], failure=failure, silent=not failure, cwd=host)

        def inspect(assembly, name):
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(assembly, args.inspect / f'{name}-{args.mode}-{args.ir}-{args.vm}.rxas')

        def image(assembly, name, *, assembler='noopt'):
            run(name + ' assemble', [tool('rxas'), *(['-n'] if assembler == 'noopt' else []), assembly], silent=True)
            output = application / (name + '-' + assembler + '-linked')
            run(name + ' link', [tool('rxlink'), '-o', output, assembly.with_suffix('.rxbin'),
                                 *(runtime / (library + '.rxbin') for library in libraries)], silent=True)
            return output.with_suffix('.rxbin')

        def execute(assembly, name, expected, *, assembler='noopt', code=None, signal=None, location=None):
            linked = image(assembly, name, assembler=assembler)
            completed = run(name + ' execute', [target_vm, '--no-autoload', linked], failure=code is not None, cwd=application)
            assert completed.stdout.replace(b'\r\n', b'\n') == expected, (name, completed.stdout, expected)
            if code is None:
                assert not completed.stderr, (name, completed.stderr)
            else:
                assert code in completed.stderr and signal in completed.stderr, (name, completed.stderr)
                if location:
                    assert location in completed.stderr, (name, location, completed.stderr)
                assert b'Assertion' not in completed.stderr and b'UNREACHED' not in completed.stdout, (name, completed.stderr)
            return completed

        if args.phase in ('all', 'source'):
            assembly = application / 'source.rxas'
            compile_c(fixtures / 'source-c.crexx', assembly)
            inspect(assembly, 'source')
            text = assembly.read_text(encoding='utf-8')
            unit_body(text)
            check_consume_origin(text, (fixtures / 'source-c.crexx').read_bytes())
            assert str(build) not in text and str(work) not in text, 'stable logical source names'
            assert calls(text, 'rexxpool.rexxvariablepool.exposesymbol') and calls(text, 'rexxpool.rexxvariablepool.exposeindirect'), 'direct and indirect use shared pool bodies'
            assert 'rexxclassicparse.execute' in text, 'ARG/PARSE retain shared executor'
            expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
            for assembler in ('noopt', 'opt'):
                execute(assembly, 'source', expected, assembler=assembler)

        if args.phase in ('all', 'refusals'):
            # Group-only targets remain nonentries; tests/call owns their
            # reached43.1 behavior now that unresolved ordinary calls compile.
            negatives = (
                ('bare-main', 'procedure', b'17.1'),
                ('after-nop', 'p: nop; procedure', b'17.1'),
                ('dead-nested', 'if 0 then procedure', b'17.1'),
                ('else-placement', 'p: if 0 then nop; else procedure', b'17.1'),
                ('otherwise-placement', 'p: select; when 1 then nop; otherwise procedure; end', b'17.1'),
                ('dead-tail', 'exit 0; p: procedure expose', None),
                ('empty-indirect', 'p: procedure expose ()', None),
                ('numeric-indirect', 'p: procedure expose (123)', None),
                ('indirect-expression', 'p: procedure expose (name + 1)', None),
                ('unclosed-indirect', 'p: procedure expose (name', None),
                ('unknown-tail', 'p: procedure strange', None),
            )
            for name, statement, code in negatives:
                source = work / (name + '.crexx')
                source.write_text('options levelc\n' + statement + '\n', encoding='utf-8')
                destination = application / (name + '.rxas')
                destination.write_bytes(b'previous output\n')
                completed = compile_c(source, destination, failure=True)
                diagnostic = completed.stdout + completed.stderr
                assert source.name.encode() + b':' in diagnostic and b'LL_COMPILER_' in diagnostic, (name, diagnostic)
                assert all(word in diagnostic for word in (b'line ', b'column ', b'byte ', b'^')), (name, diagnostic)
                if code:
                    assert code in diagnostic, (name, diagnostic)
                assert destination.read_bytes() == b'previous output\n', name
                assert not Path(str(destination) + '.llrxc-stage').exists(), name

        if args.phase in ('all', 'fatal'):
            cases = [('main', "say 'before'\np: procedure\nsay 'UNREACHED'\n", b'17.1', b'CLASSIC_SYNTAX', 3),
                     ('second', "call p\nexit 0\np: procedure\nsay 'before'\nq: procedure\nsay 'UNREACHED'\n", b'17.1', b'CLASSIC_SYNTAX', 6)]
            for name, instruction in (('nop', 'nop'), ('options', 'options'), ('literal-options', "options 'held'"),
                                      ('empty-arg', 'arg'), ('comma-arg', 'arg ,,,;'), ('group', 'do; end')):
                cases.append((name, "say 'before'\ncall p\nexit 0\np:\n" + instruction + "\nq: procedure\nsay 'UNREACHED'\n", b'17.1', b'CLASSIC_SYNTAX', 7))
            cases.append(('private-divide', "call p\nexit 0\np: procedure expose SHARED\nSHARED = 'written'\nsay 'before'\nBAD = 1 / 0\nsay 'UNREACHED'\n", b'42.3', b'CLASSIC_SYNTAX', 7))
            cases.append(('nested-group', "say 'before'\ncall p\nexit 0\np:\ndo\nINNER:\nprocedure\nsay 'UNREACHED'\nend\nreturn\n", b'17.1', b'CLASSIC_SYNTAX', 8))
            for name, body, code, signal, line in cases:
                source = work / ('fatal-' + name + '.crexx')
                source.write_text('options levelc\n' + body, encoding='utf-8')
                assembly = application / ('fatal-' + name + '.rxas')
                compile_c(source, assembly)
                inspect(assembly, 'fatal-' + name)
                text = assembly.read_text(encoding='utf-8')
                check_fatal_cleanup(text)
                execute(assembly, name, b'before\n', code=code, signal=signal,
                        location=(source.name + ':' + str(line) + ':').encode())
            source = work / 'private-exit.crexx'
            source.write_text("options levelc\nLIST = ''\ncall p\nsay 'UNREACHED'\np: procedure expose X (LIST)\nsay 'before'\nexit 0\nsay 'UNREACHED'\n", encoding='utf-8')
            assembly = application / 'private-exit.rxas'
            compile_c(source, assembly)
            inspect(assembly, 'private-exit')
            text = assembly.read_text(encoding='utf-8')
            body = unit_body(text)
            assert not re.search(r'^\s*exit\s', body, re.M), 'private EXIT returns through its source-unit frame'
            requests = calls(body, 'rexxactivation.rexxactivationarguments.requestprogramexit')
            assert len(requests) == 1, 'private EXIT records one shared program-root request'
            request = requests[0]
            links = re.findall(r'^\s*link\s+[^,\n]+,([^\n]+)$', body[:request.start()], re.M)
            assert links, 'EXIT reads its owned result packet before recording the root'
            packet = links[-1].strip()
            tail = body[request.end():]
            terminal = re.search(r'^\s*ret\s+([^\s]+)\s*$', tail, re.M)
            assert terminal and terminal.group(1) == packet, 'private EXIT returns the same owned result packet'
            cleanup = tail[:terminal.start()]
            restores = calls(cleanup, 'rexxclassiccontext.rexxclassiccontext.bindpool')
            owners = calls(body[:request.start()], 'llctarget.poolnew')
            assert len(restores) == len(owners) == 1, 'private EXIT restores its installed pool before returning'
            owner = owners[0].group(1).strip()
            assert re.search(r'^\s*null\s+' + re.escape(owner) + r'\s*$', cleanup[restores[0].end():], re.M), 'restore precedes private pool owner release'
            assert not re.search(r'^\s*null\s+' + re.escape(packet) + r'\s*$', cleanup, re.M), 'cleanup preserves the returned packet'
            check_fatal_cleanup(text)
            execute(assembly, 'private-exit', b'before\n')

        if args.phase in ('all', 'shapes'):
            shapes = (
                ('label-free', 'exit 0\n', False),
                ('shared', 'call p\nexit 0\np: nop\nreturn\n', False),
                ('private', "LIST = ''\ncall p\nexit 0\np: procedure expose X (LIST)\nreturn\n", True),
            )
            for name, source_text, private in shapes:
                source = work / ('shape-' + name + '.crexx')
                source.write_text('options levelc\n' + source_text, encoding='utf-8')
                assembly = application / ('shape-' + name + '.rxas')
                compile_c(source, assembly)
                inspect(assembly, 'shape-' + name)
                text = assembly.read_text(encoding='utf-8')
                body = unit_body(text)
                identities = re.findall(r'^\s*beq\s+llprocedure1_\d+_match,[^,\n]+,(\d+)\s*$', body, re.M)
                expected_identities = ['0'] if name == 'label-free' else ['0', '1']
                assert identities == expected_identities, ('direct integer entry dispatch', name, identities)
                assert len(calls(body, 'rexxactivation.rexxactivationarguments.procedureeligible')) == len(expected_identities), 'one eligibility check per matched dispatch, with enter owning its guard'
                factories = calls(body, 'llctarget.poolnew') + calls(body, 'rexxpool.rexxvariablepool.§factory')
                if private:
                    assert len(calls(body, 'rexxactivation.rexxactivationarguments.enterprocedure')) == 1, 'one executed PROCEDURE site'
                    assert len(factories) == 1, 'one reached replacement-pool factory site'
                    assert len(calls(body, 'rexxpool.rexxvariablepool.exposesymbol')) == 1, 'one direct exposure site'
                    assert len(calls(body, 'rexxpool.rexxvariablepool.exposeindirect')) == 1, 'one indirect exposure site'
                    assert len(calls(body, 'rexxclassiccontext.rexxclassiccontext.bindpool')) >= 2, 'normal binding install/restoration'
                    check_fatal_cleanup(text)
                else:
                    assert not factories, 'no private pool for label-free/shared body'
                    assert not calls(body, 'rexxpool.rexxvariablepool.inheritcommandstatus'), 'shared entry has no private status inheritance'
                    assert not calls(body, 'rexxclassiccontext.rexxclassiccontext.bindpool'), 'shared body has no private binding machinery'
                assert not re.search(r'^\s*(?:parseplan|parseword|parsepos|parseeos)\s', text, re.M)

        if args.phase in ('all', 'context'):
            assembly = application / 'context.rxas'
            compile_c(fixtures / 'context-c.crexx', assembly)
            inspect(assembly, 'context-generated')
            text = assembly.read_text(encoding='utf-8')
            unit_body(text)
            check_fatal_cleanup(text)
            # Export the real compiled unit for a test-only B caller. This
            # changes symbol visibility, not source-body execution or lowering.
            text, declarations = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=procedurefixture.run\2', text, count=1, flags=re.M)
            text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"procedurefixture.run"', text, count=1, flags=re.M)
            assert declarations == metadata == 1, 'single test-only exported unit'
            assembly.write_text(text, encoding='utf-8')
            inspect(assembly, 'context-exported')
            run('context assemble', [tool('rxas'), '-n', assembly], silent=True)
            imports = work / 'B caller imports'
            imports.mkdir()
            for file in runtime.glob('*.rxbin'):
                shutil.copy2(file, imports / file.name)
            shutil.copy2(assembly.with_suffix('.rxbin'), imports / 'procedurefixture.rxbin')
            caller = application / 'procedure-host'
            run('B caller compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import',
                                     '-i', imports, '-o', caller, fixtures / 'host.crexx'], silent=True)
            run('B caller assemble', [tool('rxas'), '-n', caller], silent=True)
            linked = application / 'procedure-host-linked'
            control = work / 'host.ctl'
            inputs = [caller.with_suffix('.rxbin'), assembly.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)]
            control.write_text(''.join('INPUT ' + str(file) + '\n' for file in inputs) + 'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
            run('B caller link', [tool('rxlink'), '-c', control], silent=True)
            completed = run('direct B caller restoration', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
            assert re.fullmatch(rb'PROCEDURE_HOST\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, completed
            # Raw faults retain their payload and report the authored B caller;
            # generated dispatch/prologue locations are not authored C clauses.
            for name, signal, detail, caller_line, location in (
                    ('wrong-positive', b'INVALID_ARGUMENTS', b'activation does not match entry kind', 26, None),
                    ('wrong-zero', b'INVALID_ARGUMENTS', b'activation does not match entry kind', 32, None),
                    ('unknown-entry', b'INVALID_ARGUMENTS', b'invalid source-unit entry identity', 38, None),
                    ('expired-pool', b'REFERENCE_INVALID', b'Classic pool reference is not live', 45, None),
                    ('expired-config', b'REFERENCE_INVALID', b'Classic configuration reference is not live', 52, b'context-c.crexx:5:')):
                completed = run(name, [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', name], failure=True, cwd=application)
                assert not completed.stdout and signal in completed.stderr and detail in completed.stderr, (name, completed.stdout, completed.stderr)
                assert completed.stderr.count(b'PANIC:') == 1, (name, completed.stderr)
                caller_location = f'source: host.crexx:{caller_line}:'.encode()
                assert caller_location in completed.stderr, (name, caller_location, completed.stderr)
                if location:
                    assert location in completed.stderr, (name, completed.stderr)
                assert b'UNREACHED' not in completed.stdout and b'Assertion' not in completed.stderr

        if args.phase in ('all', 'contracts'):
            probe = backend / 'procedure-contracts-linked.rxbin'
            completed = run('independent AST/schema/admission', [host_vm, '--no-autoload', probe, '-a', runtime], cwd=host)
            assert re.fullmatch(rb'PROCEDURE_CONTRACTS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, completed

    print(f'PROCEDURE|phase={args.phase}|mode={args.mode}|vm={args.vm}|ir={args.ir}|commands={commands}|failures=0')


if __name__ == '__main__':
    main()
