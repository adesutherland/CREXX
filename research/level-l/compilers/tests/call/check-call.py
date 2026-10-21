#!/usr/bin/env python3
"""Independent ordinary CALL checks over prepared B/L compiler products.

No shared build is launched. C requests run with native compilers absent from
PATH. Scratch products are temporary; optional readable RXAS copies are active
review inputs and must be removed after review.
"""
import argparse
import ast
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


def routine(text, symbol):
    match = re.search(r'^\S+\(.*\.locals=.*\.expose=' + re.escape(symbol) +
                      r'\s*$([\s\S]*?)(?=^\S+\(.*\s\.expose=|\Z)', text, re.M)
    assert match, ('missing emitted helper', symbol)
    return match.group()


def unit(text):
    names = re.findall(r'^llf\d+\(\)', text, re.M)
    assert names == ['llf1()'], ('one shared source-unit body', names)
    assert 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int' in text
    return text.split('llerror1_1:', 1)[0]


def named_call(text, name):
    return re.search(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s+[^,\n]+,' +
                     r'§?' + re.escape(name) + r'\(\)(?:,|\s*$)', text, re.M)


def transport_clause(text, source):
    """Re-entry may repeat records; the authored MARK span keeps its clause."""
    data = source.read_bytes()
    expression = b"MARK('a')"
    offset = data.index(expression)
    line = data[:offset].count(b'\n') + 1
    line_start = data.rfind(b'\n', 0, offset) + 1
    column = len(data[line_start:offset].decode('utf-8')) + 1
    ending = column + len(expression.decode('utf-8'))
    pattern = (r'^\s*\.srcstep\s+\d+\s+(\d+)\s+17\s+"' + re.escape(source.name) +
               rf'"\s+{line}\s+{column}\s+{ending}\s+"')
    clauses = [int(value) for value in re.findall(pattern, unit(text), re.M)]
    assert clauses and clauses[0] > 0 and len(set(clauses)) == 1, ('nested actual transport retains its admitted exact-span clause', clauses)


def shapes(text, helper_text):
    body = unit(text)
    assert named_call(body, 'llctarget.lengthvalue'), 'admitted one-argument LENGTH specialization'
    assert named_call(routine(helper_text, 'llctarget.lengthvalue'),
                      'rexxclassicbiflength.rexxclassicbif_length'), 'LENGTH helper uses shared named entry'
    for name in ('rexxclassicbifsubstr.rexxclassicbif_substr',
                 'rexxclassicbifunicode.rexxclassicbif_graphemecount',
                 'rexxclassicbifvalue.rexxclassicbif_value',
                 'rexxclassicbifarg.rexxclassicbif_arg'):
        assert named_call(body, name), ('selected shared named BIF', name)
    assert 'rexxclassicbif_call' not in text, 'no legacy runtime text dispatcher'
    assert named_call(body, 'rexxclassicbifs.rexxclassicbif_checked'), 'shared checked-error builder'
    assert named_call(body, 'llctarget.bifarguments'), 'one shared BIF operand/context transport'
    transport = routine(helper_text, 'llctarget.bifarguments')
    for kind, spelling in (('RexxValue[]', r'reference \.rexxvalue(?:\.\.rexxvalue)?\[\*\]'),
                           ('int[]', r'reference \.int\[\*\]')):
        parameters = re.findall(r'^\s*\.meta "llctarget\.bifarguments\.[^"]+"="b" "' +
                                spelling + r'" (a\d+)\s*$', transport, re.M)
        assert len(parameters) == 1, ('raw vector reference argument', kind, parameters)
        assert re.search(r'^\s*linkref\s+[^,\n]+,' + parameters[0] + r'\s*$', transport, re.M), ('scoped raw vector alias', kind)
    assert not re.search(r'^\s*\.meta "llctarget\.bifarguments\.[^"]+"="b" '
                         r'"reference \.rexxactivation[^"\n]*" a\d+\s*$', transport, re.M), 'no full packet reference formal'
    vector_links = re.findall(r'^\s*linkattr1\s+([^,\n]+),([^,\n]+),(\d+)\s*\n'
                             r'\s*mkref\s+[^,\n]+,\1\s*\n\s*unlink\s+\1\s*$', body, re.M)
    assert any(packet == other_packet and int(index) > 0 and int(other_index) > 0 and index != other_index
               for _, packet, index in vector_links for _, other_packet, other_index in vector_links), 'distinct raw vector attributes are borrowed and detached before transport'
    for forbidden in ('rexxpool.rexxvariablepool.§factory', 'rexxclassicconfig.rexxclassicconfig.§factory'):
        assert not named_call(transport, forbidden), ('no unused BIF owner allocation', forbidden)
    assert '45.1' in routine(helper_text, 'llctarget.emptyreturn')
    assert '44.1' in routine(helper_text, 'llctarget.requirevalue')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
    parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
    parser.add_argument('--ir', choices=('on', 'off'), default='on')
    parser.add_argument('--phase', choices=('all', 'source', 'fatal', 'refusals', 'contracts', 'shapes'), default='all')
    parser.add_argument('--inspect', type=Path)
    args = parser.parse_args()
    build = args.build.resolve()
    backend = build / 'research/level-l/compilers/backend' / args.mode
    fixtures = Path(__file__).resolve().parent
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    args.work.mkdir(parents=True, exist_ok=True)
    commands = 0
    with tempfile.TemporaryDirectory(prefix='call-', dir=args.work) as temporary:
        work = Path(temporary)
        runtime = work / 'runtime'; runtime.mkdir()
        for name in libraries:
            provider = backend if name == 'llctarget' else build / 'bin'
            shutil.copy2(provider / (name + '.rxbin'), runtime / (name + '.rxbin'))
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

        def run(label, command, *, failure=False, silent=False, cwd=work):
            nonlocal commands
            result = subprocess.run([str(item) for item in command], cwd=cwd, env=env,
                                    capture_output=True, timeout=900)
            commands += 1
            diagnostic = result.stdout + result.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
            assert (result.returncode != 0 if failure else result.returncode == 0), (label, result.returncode, diagnostic[-6000:])
            if silent:
                assert not diagnostic, (label, diagnostic[-4000:])
            return result

        def compile_c(source, output, *, failure=False):
            return run('compile ' + source.name,
                       [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []),
                        '-i', runtime, '-o', output, source], failure=failure, silent=not failure, cwd=host)

        def inspect(path, name):
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, args.inspect / f'{name}-{args.mode}-{args.ir}-{args.vm}.rxas')

        def image(assembly, name):
            run(name + ' assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), assembly], silent=True)
            output = application / (name + '-linked')
            run(name + ' link', [tool('rxlink'), '-o', output, assembly.with_suffix('.rxbin'),
                                *(runtime / (library + '.rxbin') for library in libraries)], silent=True)
            return output.with_suffix('.rxbin')

        if args.phase in ('all', 'source', 'shapes'):
            assembly = application / 'source.rxas'
            compile_c(fixtures / 'source-c.crexx', assembly)
            inspect(assembly, 'call-source')
            text = assembly.read_text()
            assert str(work) not in text and str(build) not in text, 'stable logical source names'
            transport_clause(text, fixtures / 'source-c.crexx')
            shapes(text, (backend / 'llctarget.rxas').read_text())
            if args.phase != 'shapes':
                linked = image(assembly, 'source')
                result = run('source execute', [target_vm, '--no-autoload', linked], cwd=application)
                expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
                assert result.stdout.replace(b'\r\n', b'\n') == expected and not result.stderr, ('independent source output', result.stdout, result.stderr, expected)

        if args.phase in ('all', 'fatal'):
            assembly = application / 'fatal.rxas'
            compile_c(fixtures / 'fatal-c.crexx', assembly)
            inspect(assembly, 'call-fatal')
            literals = re.findall(r'^\s*load\s+r\d+,("(?:\\.|[^"\\])*")\s*$', unit(assembly.read_text()), re.M)
            assert any(ast.literal_eval(literal) == 'A\x00B' for literal in literals), 'caller retains the exact decoded NUL target independently of diagnostic display'
            linked = image(assembly, 'fatal')
            source = (fixtures / 'fatal-c.crexx').read_text().splitlines()
            cases = (
                ('missing', b'mark M1\nmark M2\n', b'43.1', "when MODE = 'missing'", b'ABSENT'),
                ('quoted-lower', b'mark Q\n', b'43.1', "when MODE = 'quoted-lower'", b'length'),
                ('quoted-constant', b'mark N\n', b'43.1', "when MODE = 'quoted-constant'", b'7'),
                ('quoted-policy', b'mark O\n', b'43.1', "when MODE = 'quoted-policy'", b'ON'),
                ('quoted-empty', b'mark E\n', b'43.1', "when MODE = 'quoted-empty'", None),
                ('quoted-nul', b'mark Z\n', b'43.1', "when MODE = 'quoted-nul'", b'A\\x00B'),
                ('unicode-missing', b'mark U\n', b'43.1', "when MODE = 'unicode-missing'", 'Μ'.encode()),
                ('dotted-missing', b'mark D\n', b'43.1', "when MODE = 'dotted-missing'", b'P.Q'),
                ('too-few', b'', b'40.3', "when MODE = 'too-few'", None),
                ('omitted', b'', b'40.5', "when MODE = 'omitted'", None),
                ('raw-slots', b'', b'40.4', "when MODE = 'raw-slots'", None),
                ('value-invalid-name', b'', b'40.26', "when MODE = 'value-invalid-name'", None),
                ('bare-function', b'', b'45.1', None, None),
                ('eof-function', b'', b'44.1', "when MODE = 'eof-function'", b'TRAILING'),
            )
            for mode, expected, code, clause, target in cases:
                line = source.index('EMPTY:') + 3 if clause is None else next(i for i, text in enumerate(source, 1) if clause in text)
                result = run('fatal ' + mode, [target_vm, '--no-autoload', linked, '-a', mode], failure=True, cwd=application)
                diagnostic = result.stderr
                assert result.stdout.replace(b'\r\n', b'\n') == expected, (mode, result.stdout, expected)
                assert code in diagnostic and b'CLASSIC_SYNTAX' in diagnostic, (mode, diagnostic)
                assert f'fatal-c.crexx:{line}:'.encode() in diagnostic, (mode, line, diagnostic)
                if target:
                    assert target in diagnostic, (mode, target, diagnostic)
                assert b'UNREACHED' not in result.stdout and b'Assertion' not in diagnostic

        if args.phase in ('all', 'refusals'):
            cases = (
                ('bare', 'call', b'19.2'),
                ('dead-malformed', 'if 0 then call +', b'19.2'),
                ('policy-on', 'call on error', b'PROFILE'),
                ('policy-off', 'call off error', b'PROFILE'),
                ('grouped-target', 'do\nINNER:\nnop\nend\ncall INNER', b'16.3'),
            )
            output = application / 'prior.rxas'; prior = b'unchanged prior output\n'; output.write_bytes(prior)
            for name, text, code in cases:
                source = work / (name + '.crexx'); source.write_text(text + '\n')
                result = compile_c(source, output, failure=True)
                diagnostic = result.stdout + result.stderr
                assert code in diagnostic, (name, code, diagnostic)
                assert output.read_bytes() == prior, (name, 'failed source request replaced prior output')

        if args.phase in ('all', 'contracts'):
            probe = host / 'call-contracts-linked.rxbin'
            shutil.copy2(backend / probe.name, probe)
            forged = work / 'forged runtime'; forged.mkdir()
            shutil.copy2(runtime / 'llctarget.rxbin', forged / 'llctarget.rxbin')
            result = run('independent contracts', [host_vm, '--no-autoload', probe, '-a', runtime, forged], cwd=host)
            assert re.fullmatch(rb'CALL_CONTRACTS\|checks=\d+\|failures=0\r?\n', result.stdout) and not result.stderr, (result.stdout, result.stderr)
    print(f'CALL_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
