#!/usr/bin/env python3
"""Prepared static statement providers; no builds or source discovery.

Owning B/G preparation is explicit and outside the C request host. One C caller
serves normal, direct-host and fatal checks. Scratch images/logs are disposable;
--inspect retains only active readable RXAS for independent review.
"""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


def called(text, symbol):
    return list(re.finditer(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s+([^,\n]+),§?' +
                           re.escape(symbol) + r'\(\)(?:,|\s*$)', text, re.M | re.I))


def unit(text):
    assert re.findall(r'^llf\d+\(\)', text, re.M) == ['llf1()'], 'one shared C unit body'
    start = re.search(r'^llf1\(\) \.locals=', text, re.M)
    assert start, 'anchored unit definition'
    return text[start.start():text.index('llerror1_1:')]


def executable(text):
    rows = []
    for line in text.splitlines():
        row = line.strip()
        if not row or row.startswith(('.', '/*', '*', '//')):
            continue
        if row.endswith(':') or re.match(r'^\S+\(', row):
            continue
        rows.append(row)
    return rows


def caller_shape(text):
    body = unit(text)
    assert not re.search(r'^\s*exit\s', body, re.M), 'provider calls complete frames'
    targets = ('extfixture.length', 'eoffixture.run',
               'levelc_call_external_bg.typedentry', 'levelc_call_alias.typedentry')
    rows = executable(body); calls = 0
    for index, row in enumerate(rows):
        match = re.match(r'call\s+[^,]+,§?(' + '|'.join(re.escape(name) for name in targets) +
                         r')\(\),(r\d+)$', row, re.I)
        if not match:
            continue
        calls += 1
        count = match[2]; argument = 'r' + str(int(count[1:]) + 1)
        before = rows[max(0, index - 12):index]
        assert 'load ' + count + ',1' in before, ('one counted provider argument', row, before)
        assert 'settp ' + argument + ',768' in before, ('disposable by-value activation', row, before)
        swaps = [re.fullmatch(r'swap ([^,]+),([^,]+)', entry) for entry in before]
        swaps = [entry for entry in swaps if entry and argument in entry.groups()]
        assert swaps, ('caller owns invocation through SWAP', row, before)
        pair = set(swaps[-1].groups())
        after = re.fullmatch(r'swap ([^,]+),([^,]+)', rows[index + 1])
        assert after and set(after.groups()) == pair, ('inverse SWAP immediately recovers completion before NULL/helper', row, rows[index + 1:index + 5])
    assert calls >= 4 and all(called(body, target) for target in targets), 'all selected provider families use direct exports'
    assert not called(body, 'llctarget.status'), 'no status conversion in provider call body'
    assert len(re.findall(r'^\s*sigbrv\s+[^\n]*"FUNCTION_NOT_FOUND"', text, re.M)) == 1, 'external unit installs one core missing-function cleanup relay'
    assert len(re.findall(r'^\s*sighalt\s+"FUNCTION_NOT_FOUND"', text, re.M)) == 1, 'fatal relay detaches core missing-function trap'
    fatal = text[text.index('llerror1:'):]
    restored = called(fatal, 'rexxclassiccontext.rexxclassiccontext.bindpool')
    owners = {match[1].strip() for match in called(body, 'llctarget.poolnew')}
    releases = [match for match in re.finditer(r'^\s*null\s+(\S+)\s*$', fatal, re.M) if match[1] in owners]
    assert restored and releases and restored[0].end() < releases[0].start(), 'fatal external call restores original binding before private owner release'
    assert 'signalorigin' in fatal, 'original fatal event/location is relayed'
    assert not re.search(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s+[^,]+,§?ll(?:binmeta|cbindings|ccompiler)\.', text, re.M), 'static runtime never reads compiler metadata/images'


def provider_shape(text):
    exports = []
    for header in re.findall(r'^\S+\([^\n]*\s\.locals=\d+[^\n]*$', text, re.M):
        exported = re.search(r'\.expose=(\S+)', header)
        if exported:
            exports.append(exported[1])
    assert [item.lower() for item in exports] == ['extfixture.length'], ('sole ordinary wrapper export', exports)
    metadata = re.search(r'^\s*\.meta "extfixture\.length"="b" "\.void" \S+ "([^"\n]+)"\s*$', text, re.M | re.I)
    assert metadata and re.fullmatch(r'[^=,?]+=(?:\.rexxactivationarguments|\.rexxactivation\.\.rexxactivationarguments)', metadata[1], re.I), 'one required by-value activation, void ABI'
    assert not re.search(r'^\s*\.main\b|^\s*exit\s', text, re.M), 'routine image has no main/VM exit'
    assert not called(text, 'llctarget.status'), 'routine wrapper has no command status bridge'
    assert re.search(r'^\s*brtpandt\s+[^,]+,a1,512\s*$', text, re.M), 'wrapper distinguishes named from disposable input'
    assert re.search(r'^\s*copy\s+[^,]+,a1\s*$', text, re.M) and re.search(r'^\s*swap\s+[^,]+,a1\s*$', text, re.M), 'named COPY and disposable SWAP paths are both retained'
    assert called(text, 'rexxactivation.rexxactivationarguments.consumefirstinstruction'), 'wrapper entry0 normalizes PROCEDURE eligibility once'
    assert called(text, 'llctarget.poolnew') and called(text, 'rexxclassicconfig.rexxclassicconfig.§factory'), 'wrapper owns fresh private pool/config'
    installed = called(text, 'rexxactivation.rexxactivationarguments.setreturnvalue')
    absent = called(text, 'rexxactivation.rexxactivationarguments.clearreturnvalue')
    config = called(text, 'rexxclassicconfig.rexxclassicconfig.§factory')[0]
    released = re.search(r'^\s*null\s+' + re.escape(config[1].strip()) + r'\s*$', text[config.end():], re.M)
    assert installed and absent and released, 'wrapper transfers both completion states and releases config'
    release_at = config.end() + released.start()
    assert installed[0].end() < release_at and absent[0].end() < release_at, 'completion copyback precedes provider-local owner release'
    assert 'FUNCTION_NOT_FOUND' not in text, 'wrapper without external operations gains no external-call trap'


def signature_image():
    """Tiny metadata-only negative producers; never executed as providers."""
    rows = ['.globals=0', '.meta "box"="b" ".unknown" .class',
            '.meta "face"="b" ".unknown" .interface',
            '.meta "face"="method final" "ready" ".void" "frame=.rexxactivationarguments" .member']
    entries = (
        ('plain.good', '.void', 'frame=.rexxactivationarguments', 'b'),
        ('plain.unicode', '.void', 'é!?$@#.part=.rexxactivationarguments', 'b'),
        ('scope.ı', '.void', 'frame=.rexxactivationarguments', 'b'),
        ('bad.result', '.int', 'frame=.rexxactivationarguments', 'b'),
        ('bad.arity', '.void', 'frame=.rexxactivationarguments,extra=.int', 'b'),
        ('bad.reference', '.void', 'frame=reference .rexxactivationarguments', 'b'),
        ('bad.optional', '.void', '?frame=.rexxactivationarguments', 'b'),
        ('bad.flags', '.void', 'frame=.rexxactivationarguments', 'c'),
        ('bad.formal', '.void', '7frame=.rexxactivationarguments', 'b'),
        ('box.run', '.void', 'frame=.rexxactivationarguments', 'b'),
        ('box.§factory', '.void', 'frame=.rexxactivationarguments', 'b'),
        ('face.ready', '.void', 'frame=.rexxactivationarguments', 'b'),
    )
    for name, result, arguments, flags in entries:
        rows.extend((f'§{name}() .locals=1 .expose={name}', 'load r0,0', 'ret' if result == '.void' else 'ret r0',
                     f'.meta "{name}"="{flags}" "{result}" §{name}() "{arguments}"'))
    return '\n'.join(rows) + '\n'


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
    repository = fixtures.parents[5]
    backend = build / 'research/level-l/compilers/backend' / args.mode
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    args.work.mkdir(parents=True, exist_ok=True); commands = 0
    with tempfile.TemporaryDirectory(prefix='external-', dir=args.work) as temporary:
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

        host, host_vm = stage('C compiler host', ('rxfs', 'rxcontract'))
        application, target_vm = stage('static application', ('rxfs', 'rxcstream'))
        compiler = host / 'llrxc-c-linked.rxbin'; shutil.copy2(backend / compiler.name, compiler)
        empty_path = work / 'empty PATH'; empty_path.mkdir()
        env = dict(os.environ, PATH=str(empty_path)); env.pop('CREXX_PROVIDER_PATH', None)
        assert not any(shutil.which(name, path=env['PATH']) for name in ('rxc', 'rxc-c'))

        def run(label, command, *, failure=False, silent=False, cwd=work):
            nonlocal commands
            result = subprocess.run([str(item) for item in command], cwd=cwd, env=env,
                                    capture_output=True, timeout=900)
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

        def assemble(assembly):
            run('assemble ' + assembly.name, [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), assembly], silent=True)
            return assembly.with_suffix('.rxbin')

        def compile_b(source, stem, imports=runtime):
            assembly = application / (stem + '.rxas')
            run('owning B/G preparation ' + stem,
                [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import', '-i', imports, '-o', assembly, source], silent=True)
            return assembly, assemble(assembly)

        # Existing provider bodies stay unchanged; only their owning dialect
        # preparation differs. Both resulting images are admitted independently.
        bg_source = repository / 'compiler/tests/rexx_src/levelc_call_external_bg.crexx'
        _, bg_b = compile_b(bg_source, 'bg-b'); shutil.copy2(bg_b, runtime / 'bg-b.rxbin')
        g_source = work / 'bg-g.crexx'
        g_source.write_text(bg_source.read_text().replace('options levelb', 'options levelg', 1), encoding='utf-8')
        _, bg_g = compile_b(g_source, 'bg-g'); shutil.copy2(bg_g, runtime / 'bg-g.rxbin')
        _, alias = compile_b(repository / 'compiler/tests/rexx_src/levelc_call_alias_provider.crexx', 'alias')
        shutil.copy2(alias, runtime / 'alias.rxbin')
        selections = (('c-provider', 'extfixture.LENGTH'), ('c-eof', 'eoffixture.run'),
                      ('bg-b', 'levelc_call_external_bg.typedentry'), ('alias', 'levelc_call_alias.typedentry'))

        def compile_c(source, output, *, bindings=(), routine='', failure=False):
            options = [part for stem, target in bindings for part in ('--provider', stem, target)]
            if routine:
                options += ['--routine', routine]
            return run('C compile ' + source.name,
                       [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []),
                        '-i', runtime, *options, '-o', output, source], failure=failure, silent=not failure, cwd=host)

        provider = application / 'c-provider.rxas'
        compile_c(fixtures / 'provider-c.crexx', provider, routine='extfixture.LENGTH')
        inspect(provider, 'external-provider'); provider_shape(provider.read_text())
        shutil.copy2(assemble(provider), runtime / 'c-provider.rxbin')
        # A separate tiny routine is necessary: the main provider has later
        # private labels and cannot conditionally reach unit EOF.
        eof_source = work / 'eof-provider.crexx'; eof_source.write_text('nop\n')
        eof = application / 'c-eof.rxas'; compile_c(eof_source, eof, routine='eoffixture.run')
        shutil.copy2(assemble(eof), runtime / 'c-eof.rxbin')

        def linked(name, root, inputs):
            output = application / (name + '-linked'); control = work / (name + '.ctl')
            control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) +
                               'ROOT ' + root + '\nOUTPUT ' + str(output) + '\n', encoding='utf-8')
            run(name + ' link', [tool('rxlink'), '-c', control], silent=True)
            return output.with_suffix('.rxbin')

        base = [runtime / (name + '.rxbin') for name in libraries]
        extra = [runtime / (name + '.rxbin') for name in ('c-provider', 'c-eof', 'alias')]
        if args.phase in ('all', 'source', 'host', 'fatal', 'shapes'):
            assembly = application / 'source.rxas'
            compile_c(fixtures / 'source-c.crexx', assembly, bindings=selections)
            text = assembly.read_text(); inspect(assembly, 'external-source'); caller_shape(text)
            assert str(work) not in text and str(build) not in text, 'stable logical C source identities'
            source_binary = assemble(assembly)
            if args.phase in ('all', 'source'):
                expected = b'bif=3\nlocal=BG-LOCAL\nbg=BG-RESULT\nalias=ALIAS-RESULT\n'
                for dialect, bg in (('b', bg_b), ('g', bg_g)):
                    image = linked('source-' + dialect, assembly.stem, [source_binary, *extra, bg, *base])
                    result = run('unchanged owning ' + dialect + ' provider completion', [target_vm, '--no-autoload', image], cwd=application)
                    assert result.stdout.replace(b'\r\n', b'\n') == expected and not result.stderr, (dialect, result.stdout, result.stderr, expected)
            if args.phase in ('all', 'host', 'fatal'):
                exported = application / 'externalfixture.rxas'
                exported_text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=externalfixture.run\2', text, count=1, flags=re.M)
                exported_text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"externalfixture.run"', exported_text, count=1, flags=re.M)
                assert headers == metadata == 1, 'one test-only exported C unit, entry0 remains ineligible'
                exported.write_text(exported_text, encoding='utf-8'); inspect(exported, 'external-exported')
                exported_binary = assemble(exported)
                imports = work / 'B caller imports'; imports.mkdir()
                for library in runtime.glob('*.rxbin'):
                    shutil.copy2(library, imports / library.name)
                shutil.copy2(exported_binary, imports / 'externalfixture.rxbin')
                shutil.copy2(runtime / 'c-provider.rxbin', imports / 'extfixture.rxbin')
                caller, caller_binary = compile_b(fixtures / 'host.crexx', 'external-host', imports)
                if args.phase in ('all', 'host'):
                    image = linked('host', caller.stem, [caller_binary, exported_binary, *extra, bg_b, *base])
                    result = run('owned completion, root/pool isolation and named B ABI', [target_vm, '--no-autoload', image], cwd=application)
                    assert re.fullmatch(rb'direct=COMMAND\r?\nEXTERNAL_HOST\|checks=\d+\|failures=0\r?\n', result.stdout) and not result.stderr, (result.stdout, result.stderr)
                if args.phase in ('all', 'fatal'):
                    full = linked('absent', caller.stem, [caller_binary, exported_binary, *extra, bg_b, *base])
                    missing = linked('unlinked', caller.stem, [caller_binary, exported_binary, runtime / 'c-eof.rxbin', runtime / 'alias.rxbin', bg_b, *base])
                    source = (fixtures / 'source-c.crexx').read_text().splitlines()
                    for mode, image, needle, signal in (
                            ('absent', full, "if MODE = 'absent'", b'CLASSIC_SYNTAX'),
                            ('private-missing', missing, '  call extfixture.LENGTH FATALMARK()', b'FUNCTION_NOT_FOUND')):
                        line = next(i for i, row in enumerate(source, 1) if needle in row)
                        result = run(mode, [target_vm, '--no-autoload', image, '-a', mode], failure=True, cwd=application)
                        assert result.stdout.replace(b'\r\n', b'\n') == b'EXTERNAL_FATAL|before\nactual before missing\n', (mode, result.stdout)
                        assert signal in result.stderr and f'source-c.crexx:{line}:'.encode() in result.stderr, (mode, line, result.stderr)
                        assert (b'43.1' in result.stderr) == (mode == 'absent'), (mode, result.stderr)
                        assert b'UNREACHED' not in result.stdout and b'Assertion' not in result.stderr

        if args.phase in ('all', 'refusals'):
            output = application / 'prior.rxas'; prior = b'unchanged prior output\n'; output.write_bytes(prior)
            startup_cases = (
                ('missing-provider-callable', ('--provider', 'c-provider')),
                ('option-as-provider-callable', ('--provider', 'c-provider', '--routine', 'sample.run')),
                ('missing-routine-export', ('--routine',)),
            )
            for name, options in startup_cases:
                result = run(name, [host_vm, '--no-autoload', compiler, '-a',
                                    '-i', runtime, '-o', output, *options], failure=True, cwd=host)
                diagnostic = result.stdout + result.stderr
                assert diagnostic.startswith(b'LL_COMPILER_OPTIONS: '), (name, diagnostic)
                assert output.read_bytes() == prior, (name, 'invalid options replaced previous output')
            cases = (
                ('duplicate', 'return 0\n', (selections[0], selections[0]), '', b'IMPORT'),
                ('dead-ambiguous', 'if 0 then call TYPEDENTRY\nreturn 0\n', selections, '', b'IMPORT'),
                ('routine-procedure', 'procedure\nreturn\n', (), 'external.bad', b'17.1'),
            )
            for name, data, bindings, routine, cause in cases:
                source = work / (name + '.crexx'); source.write_text(data)
                result = compile_c(source, output, bindings=bindings, routine=routine, failure=True)
                assert cause in result.stdout + result.stderr, (name, cause, result.stdout, result.stderr)
                assert output.read_bytes() == prior, (name, 'failed compile replaced previous output')

        if args.phase in ('all', 'contracts'):
            signatures = application / 'signatures.rxas'; signatures.write_text(signature_image(), encoding='utf-8')
            shutil.copy2(assemble(signatures), runtime / 'signatures.rxbin')
            tiny = application / 'tiny.rxas'
            tiny.write_text('.globals=0\n' + ''.join(
                f'§tiny.{name}() .locals=1 .expose=tiny.{name}\nret\n.meta "tiny.{name}"="b" ".void" §tiny.{name}() "frame=.rexxactivationarguments"\n'
                for name in ('a', 'b')), encoding='utf-8')
            tiny_binary = assemble(tiny)
            for name in ('tiny-a', 'tiny-b'):
                shutil.copy2(tiny_binary, runtime / (name + '.rxbin'))
            # Identical bytes, not a claim about case-distinct file contents.
            # macOS may resolve this spelling to tiny-a itself.
            if not (runtime / 'TINY-A.rxbin').exists():
                shutil.copy2(tiny_binary, runtime / 'TINY-A.rxbin')
            probe = host / 'call-external-contracts-linked.rxbin'; shutil.copy2(backend / probe.name, probe)
            result = run('independent prepared provider contracts', [host_vm, '--no-autoload', probe, '-a', runtime], cwd=host)
            assert re.fullmatch(rb'EXTERNAL_CONTRACTS\|checks=\d+\|failures=0\r?\n', result.stdout) and not result.stderr, (result.stdout, result.stderr)
    print(f'EXTERNAL_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
