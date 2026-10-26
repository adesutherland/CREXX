#!/usr/bin/env python3
"""Whole SIGNAL conditions over prepared products, in one frozen batch.

One condition C image/lane serves host, real producer and fatal controls.
The all phase also invokes the retained static SIGNAL runner once in the same
lane. Shared B tests are reused; this script never starts a product build.
"""
import argparse
import os
from pathlib import Path
import re
import runpy
import shutil
import subprocess
import sys
import tempfile


ADDRESS = runpy.run_path(str(Path(__file__).resolve().parents[1] / 'address' / 'check-address.py'))
called = ADDRESS['called']
source_position = ADDRESS['source_position']


def unit(text):
    # Include the cold dispatcher and finite fatal suffix. Their private labels
    # are implementation details, unlike the owning function definition.
    header = re.search(r'^llf1\(\) \.locals=\d+[^\n]*$', text, re.M)
    assert header, 'one shared source unit definition'
    following = re.search(r'^\S+\([^\n]*\) \.locals=\d+', text[header.end():], re.M)
    end = header.end() + following.start() if following else len(text)
    body = text[header.start():end]
    assert 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int' in body
    return body


def shapes(text, source, target):
    body = unit(text)
    all_pushes = list(re.finditer(r'^\s*sigpush\s+"([^"\n]+)"\s*$', body, re.M))
    pushes = [item for item in all_pushes if item[1] != 'BREAKPOINT']
    installs = list(re.finditer(r'^\s*sigbrv\s+[^,\n]+,[^,\n]+,"([^"\n]+)"\s*$', body, re.M))
    assert pushes and installs and {item[1] for item in pushes} == {item[1] for item in installs}, 'save and branch-install the same source-frame condition tags'
    assert max(item.start() for item in pushes) < min(item.start() for item in installs), 'save every outer condition policy before any branch handler snapshots the stack'
    observer_pushes = [item for item in all_pushes if item[1] == 'BREAKPOINT']
    observer_installs = list(re.finditer(r'^\s*sigcall\s+([^,\n]+),"BREAKPOINT"\s*$', body, re.M))
    observer_pops = list(re.finditer(r'^\s*sigpop\s+"BREAKPOINT"\s*$', body, re.M))
    assert len(observer_pushes) == 1, 'save the outer BREAKPOINT observer once'
    assert observer_installs and all(item[1] == '__rxtrace_handler()' for item in observer_installs), 'install only the private TRACE observer for BREAKPOINT'
    assert observer_pops, 'restore the outer BREAKPOINT observer'
    assert observer_pushes[0].start() < min(item.start() for item in installs), 'save the observer before branch handlers capture their stack marker'
    assert all(observer_pushes[0].start() < item.start() for item in observer_installs + observer_pops), 'observer installation and restoration follow its save'
    assert called(body, 'rexxactivation.rexxactivationarguments.setsignalpolicy'), 'source ON/OFF uses the existing activation policy owner'
    assert called(body, 'rexxclassicbifs.rexxclassic_signal_event_id'), 'one shared typed-event validator is reused'
    assert called(body, 'rexxclassicbifs.rexxclassic_signal_record_at_line'), 'handler state/SIGL/catalog recording stays shared through the additive source adapter'
    for tag in ('CLASSIC_SYNTAX', 'CLASSIC_CONDITION'):
        assert re.search(r'^\s*sigpush\s+"' + tag + r'"\s*$', body, re.M), ('save the outer VM policy', tag)
        assert re.search(r'^\s*sigpop\s+"' + tag + r'"\s*$', text, re.M), ('restore the outer VM policy', tag)
        assert re.search(r'^\s*sighalt\s+"' + tag + r'"\s*$', text, re.M), ('unhandled C relay is nonresumable after saved policy restoration', tag)
    assert re.search(r'^\s*sigbrv\s', body, re.M), 'VM entry binds the original event for the visible cold dispatch'
    assert re.search(r'^\s*signalorigin\s+"CLASSIC_CONDITION",', text, re.M), 'unselected/untyped event relays its owned payload and source'
    assert not called(body, 'rexxclassicbifs.rexxclassicbif_call'), 'no runtime BIF name dispatcher'
    assert not called(body, 'llctarget.status') and not re.search(r'^\s*exit\s', body, re.M), 'source frame returns full optional completion'
    assert not called(body, 'rexxactivation.rexxactivationarguments.beginclausetime'), 'clock-free condition unit gains no lazy-reset calls'
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
    data = source.read_bytes()
    for needle in (b'NV_WRITE = LIVE_UNSET', b'parse value \'left:right=tail\'', b'do CONTROL =', b'LD_WRITE = 1234'):
        _, line, _ = source_position(data, needle)
        assert re.search(r'^\s*\.srcstep\s+\d+\s+\d+\s+(?:17|65)\s+"conditions-c\.crexx"\s+' + str(line) + r'\s+\d+\s+\d+\s+"', body, re.M), ('guarded producer keeps truthful authored source', needle)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
    parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
    parser.add_argument('--ir', choices=('on', 'off'), default='on')
    parser.add_argument('--phase', choices=('all', 'source', 'host', 'fatal', 'refusals', 'contracts', 'native', 'shapes'), default='all')
    parser.add_argument('--inspect', type=Path)
    args = parser.parse_args()
    fixtures = Path(__file__).resolve().parent; repository = fixtures.parents[4]
    build = args.build.resolve(); backend = build / 'research/level-l/compilers/backend' / args.mode
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    provider_source = repository / 'compiler/tests/rexx_src/levelc_signal_event_provider.crexx'
    provider_name = 'levelc_signal_event_provider'
    args.work.mkdir(parents=True, exist_ok=True); commands = 0
    with tempfile.TemporaryDirectory(prefix='signal-conditions-', dir=args.work) as temporary:
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
            result = subprocess.run([str(item) for item in command], cwd=cwd, env=env, capture_output=True, timeout=1800)
            commands += 1; diagnostic = result.stdout + result.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
            assert (result.returncode != 0 if failure else result.returncode == 0), (label, result.returncode, diagnostic[-6000:])
            if silent:
                assert not diagnostic, (label, diagnostic[-4000:])
            return result

        def inspect(path, name):
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, args.inspect / f'{name}-{args.mode}-{args.ir}-{args.vm}.rxas')

        def compile_c(source, output, *, bind=False, failure=False):
            binding = ['--provider', provider_name, provider_name + '.raiseevent'] if bind else []
            return run('compile ' + source.name, [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []),
                       '-i', runtime, '-o', output, *binding, source], failure=failure, silent=not failure, cwd=host)

        if args.phase == 'all':
            retained = [sys.executable, fixtures / 'check-signal.py', '--build', build,
                        '--work', args.work / 'static', '--mode', args.mode, '--vm', args.vm,
                        '--ir', args.ir, '--phase', 'all']
            if args.inspect:
                retained.extend(('--inspect', args.inspect))
            completed = run('retained static SIGNAL controls', retained)
            summary = re.fullmatch(rb'SIGNAL_CHECK\|mode=' + args.mode.encode() + rb'\|vm=' + args.vm.encode() +
                                   rb'\|ir=' + args.ir.encode() + rb'\|phase=all\|commands=(\d+)\|PASS\r?\n', completed.stdout)
            assert summary and not completed.stderr, (completed.stdout, completed.stderr)
            commands += int(summary[1]) - 1  # Count the nested tools in place of their Python launcher.

        if args.phase in ('all', 'source', 'host', 'fatal', 'shapes'):
            # Owning B prepares the unchanged static provider before C requests.
            provider = application / provider_name
            run('unchanged B event provider compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import', '-i', runtime, '-o', provider, provider_source], silent=True)
            run('event provider assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), provider], silent=True)
            shutil.copy2(provider.with_suffix('.rxbin'), runtime / (provider_name + '.rxbin'))
            inspect(provider.with_suffix('.rxas'), 'signal-conditions-provider')
            source = work / 'conditions-c.crexx'; original = (fixtures / source.name).read_bytes()
            data = original if args.mode == 'opt' else original.replace(b'\n', b'\r\n')
            source.write_bytes(data); assembly = application / 'conditions.rxas'
            compile_c(source, assembly, bind=True); inspect(assembly, 'signal-conditions-source')
            text = assembly.read_text()
            assert str(work) not in text and str(build) not in text, 'C source uses its stable logical identity'
            shapes(text, source, (backend / 'llctarget.rxas').read_text())
            if args.phase != 'shapes':
                exported = application / 'conditions-exported.rxas'
                text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=conditionfixture.run\2', text, count=1, flags=re.M)
                text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"conditionfixture.run"', text, count=1, flags=re.M)
                assert headers == metadata == 1, 'test-only entry0 export, no provider ABI change'
                exported.write_text(text, encoding='utf-8'); inspect(exported, 'signal-conditions-exported')
                run('C source assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), exported], silent=True)
                imports = work / 'B caller imports'; imports.mkdir()
                for module in runtime.glob('*.rxbin'):
                    shutil.copy2(module, imports / module.name)
                shutil.copy2(exported.with_suffix('.rxbin'), imports / 'conditionfixture.rxbin')
                caller = application / 'conditions-host'
                run('B condition host compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import', '-i', imports, '-o', caller, fixtures / 'conditions-host.crexx'], silent=True)
                run('B condition host assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), caller], silent=True)
                linked = application / 'conditions-linked'; control = work / 'conditions.ctl'
                inputs = [caller.with_suffix('.rxbin'), exported.with_suffix('.rxbin'), provider.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) + 'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run('condition host link', [tool('rxlink'), '-c', control], silent=True)
                provider_data = provider_source.read_bytes()
                provider_line = source_position(provider_data, b'assembler signal "CLASSIC_CONDITION",event')[1]
                markers = (b'/* LIVE_SYNTAX_CAUSE */', b'/* LIVE_NOVALUE_CAUSE */', b'/* PARSE_PATTERN_CAUSE */',
                           b'/* PARSE_POSITION_CAUSE */', b'/* DO_ADVANCE_CAUSE */', b'/* LIVE_LOSTDIGITS_CAUSE */', b'/* CHILD_NOVALUE_CAUSE */')
                lines = [source_position(data, marker)[1] for marker in markers]
                common = [str(provider_line), str(work / 'uncreated directory' / 'missing input'), *(str(line) for line in lines)]
                if args.phase in ('all', 'source', 'host'):
                    expected = (fixtures / 'conditions-c.expected').read_bytes().replace(b'\r\n', b'\n')
                    for record, observation in (
                            (b'COND|address-error=1\n', b'       +++   RC=-3 MISSING-SIGNAL-ENV missing\n'),
                            (b'COND|address-failure=1\n', b'       +++   RC=19 CONDITION-TEST failure\n')):
                        assert expected.count(record) == 1, ('one command-condition observation anchor', record)
                        expected = expected.replace(record, observation + record)
                    for mode in ('normal', 'exit-handler'):
                        completed = run('SIGNAL ' + mode, [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', mode, *common], cwd=application)
                        actual = completed.stdout.replace(b'\r\n', b'\n'); prefix = expected if mode == 'normal' else b''
                        assert actual.startswith(prefix), (mode, actual, prefix)
                        assert re.fullmatch(rb'SIGNAL_CONDITIONS_HOST\|mode=' + mode.encode() + rb'\|checks=\d+\|failures=0\n', actual[len(prefix):]) and not completed.stderr, (mode, completed.stdout, completed.stderr)
                if args.phase in ('all', 'fatal'):
                    cases = (('off-syntax', b'40.14', b'/* OFF_CAUSE */', b''),
                             ('once-syntax', b'40.14', b'/* ONCE_SECOND_CAUSE */', b'SIGNAL_ONCE|caught\n'),
                             ('missing-syntax', b'16.1', b'/* MISSING_SYNTAX_CAUSE */', b''),
                             ('missing-typed', b'16.1', None, b''), ('missing-default', b'16.1', None, b''))
                    for mode, code, marker, following in cases:
                        completed = run('unhandled ' + mode, [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', mode, *common], failure=True, cwd=application)
                        assert completed.stdout.replace(b'\r\n', b'\n') == b'SIGNAL_FATAL|before\n' + following, (mode, completed.stdout)
                        assert b'CLASSIC_SYNTAX' in completed.stderr and code in completed.stderr and b'UNREACHED' not in completed.stdout, (mode, completed.stderr)
                        if marker:
                            line = source_position(data, marker)[1]
                            assert f'conditions-c.crexx:{line}:'.encode() in completed.stderr, (mode, completed.stderr)
                        else:
                            assert f'levelc_signal_event_provider.crexx:{provider_line}:'.encode() in completed.stderr, (mode, completed.stderr)
                    for mode in ('outer-relay', 'outer-untyped'):
                        completed = run('nonresumable ' + mode, [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', mode, *common], failure=True, cwd=application)
                        prior = b'SIGNAL_FATAL|before\n' + (b'SIGNAL_UNHANDLED|provider=1\n' if mode == 'outer-untyped' else b'')
                        assert completed.stdout.replace(b'\r\n', b'\n') == prior, (mode, completed.stdout)
                        assert b'CLASSIC_CONDITION' in completed.stderr and b'INVALID_ARGUMENTS' not in completed.stderr and b'UNREACHED' not in completed.stdout, (mode, completed.stderr)
                        if mode == 'outer-relay':
                            assert f'levelc_signal_event_provider.crexx:{provider_line}:'.encode() in completed.stderr, (mode, completed.stderr)
                        else:
                            host_data = (fixtures / 'conditions-host.crexx').read_bytes()
                            line = source_position(host_data, b'assembler signal "CLASSIC_CONDITION","untyped payload"')[1]
                            assert f'conditions-host.crexx:{line}:'.encode() in completed.stderr and b'untyped payload' in completed.stderr, (mode, completed.stderr)

        if args.phase in ('all', 'refusals'):
            output = application / 'prior.rxas'; prior = b'unchanged prior output\n'; output.write_bytes(prior)
            cases = (('on-empty', 'signal on', 'on', b'25.3'), ('off-empty', 'signal off', 'off', b'25.4'),
                     ('name-empty', 'signal on syntax name', 'name', b'19.3'),
                     ('dead-condition', 'if 0 then signal on bogus', 'bogus', b'25.3'))
            for name, content, token, code in cases:
                source = work / (name + '.crexx'); data = ('options levelc\n' + content + '\n').encode(); source.write_bytes(data)
                completed = compile_c(source, output, failure=True)
                offset, line, column = source_position(content.encode(), token.encode())
                offset += len(b'options levelc\n'); line += 1
                assert code in completed.stdout and f'at line {line}, column {column} (byte {offset})'.encode() in completed.stdout and not completed.stderr, (name, completed.stdout, completed.stderr)
                assert output.read_bytes() == prior and not Path(str(output) + '.llrxc-stage').exists(), 'failed SIGNAL admission preserves previous output'

        if args.phase in ('all', 'contracts'):
            probe = host / 'conditions-contracts-linked.rxbin'; shutil.copy2(backend / probe.name, probe)
            completed = run('tiny whole SIGNAL source/IR contracts', [host_vm, '--no-autoload', probe, '-a', runtime], cwd=host)
            assert re.fullmatch(rb'SIGNAL_CONDITIONS_CONTRACTS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)

        if args.phase in ('all', 'native'):
            prepared = build / 'interpreter/tests_signal_origin.rxbin'
            native = application / prepared.name; shutil.copy2(prepared, native)
            completed = run('native owned payload/origin/continuation', [target_vm, '--no-autoload', native], cwd=application)
            assert completed.stdout.replace(b'\r\n', b'\n') == b'invalid-origin-rejected\ninvalid-name-rejected\nPASS: Signal Origin Payload\n' and not completed.stderr, (completed.stdout, completed.stderr)
    print(f'SIGNAL_CONDITIONS_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
