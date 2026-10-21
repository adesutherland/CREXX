#!/usr/bin/env python3
"""Two complementary CLAUSE1 cells over prepared products; never builds.

Each cell compiles one C image and runs it once through its B owner. Four
portable one-second commands distinguish equality/freshness, with no speed
threshold. Shared deterministic DATE/TIME conversion tests remain their owner.
"""
import argparse
import os
from pathlib import Path
import re
import runpy
import shlex
import shutil
import subprocess
import sys
import tempfile


ADDRESS = runpy.run_path(str(Path(__file__).resolve().parents[1] / 'address' / 'check-address.py'))
called = ADDRESS['called']
unit = ADDRESS['unit']
source_position = ADDRESS['source_position']
BEGIN = 'rexxactivation.rexxactivationarguments.beginclausetime'
ACTIVATION = 'rexxclassiccontext.rexxclassiccontext.activation'
TIME = 'rexxclassicbiftime.rexxclassicbif_time'
DATE = 'rexxclassicbifdate.rexxclassicbif_date'


def mapped_rows(body):
    """Keep actual source directives separate from executable instructions."""
    current = None
    for row in body.splitlines():
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            current = (int(fields[2]), int(fields[3]), fields[4], int(fields[5]),
                       int(fields[6]), int(fields[7]), fields[8])
        yield row, current


def mapped_calls(body, symbol):
    return [(row, at) for row, at in mapped_rows(body) if called(row, symbol)]


def no_repeated_source(body):
    previous = None
    for row in body.splitlines():
        if re.fullmatch(r'llb\d+:\s*', row):
            previous = None  # A real control entry may restore the same mapping.
        elif row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            key = tuple(fields[2:])  # Origin IDs can differ for one source span.
            assert previous != key, 'no mechanically repeated unchanged source record inside one block'
            previous = key


def shapes(text, source, target):
    body = unit(text); data = source.read_bytes()
    begins = mapped_calls(body, BEGIN)
    assert len(begins) == 8, 'one lazy reset per selected ordinary/loop occurrence'
    assert len(called(body, DATE)) == 2 and len(called(body, TIME)) == 11, 'quoted DATE/TIME bind shared entries once per authored expression'
    assert not re.search(r'^\s*(?:time|mtime|xtime|date)\s', body, re.M), 'C caller never samples the platform clock eagerly'
    assert not re.search(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s+[^\n]*llc(?:clause|procedure|do)protocol', body, re.M | re.I), 'proofs and owner indices have no runtime representation'
    assert not called(body, 'rexxtimedate.rexxdatetimestate.beginclause'), 'caller reuses activation lazy invalidation'

    expected = {
        b"PAIR = 'DATE'": 1,
        b"LATER = 'TIME'": 1,
        b"CHILD_BEFORE = 'TIME'": 1,
        b"CHILD_AFTER = 'TIME'": 1,
        b"do I = 'TIME'": 2,
        b"do U = 'TIME'": 2,
    }
    for needle, count in expected.items():
        _, line, _ = source_position(data, needle)
        selected = [at for _, at in begins if at and at[2:4] == ('source-c.crexx', line)]
        assert len(selected) == count, ('authored clock boundary occurrences', needle, len(selected), count)
        assert all(at[1] in (17, 65) and at[4] == 1 and at[5] > at[4] and at[6].startswith(needle.decode()) for at in selected), 'clock boundaries keep exact nonempty authored head ranges/provenance'

    # The proof-only begin id does not prescribe TRACE clause identities for
    # expression children. Keep their truthful authored source subspans.
    reset_lines = {at[3] for _, at in begins if at and at[2] == 'source-c.crexx'}
    for row, at in mapped_calls(body, TIME) + mapped_calls(body, DATE):
        assert at and at[1] in (17, 65) and at[2] == 'source-c.crexx' and at[3] in reset_lines and at[5] > at[4], ('clock observation retains its authored subspan', row, at)

    # An ordinary local call named TIME/DATE has no selected clock binding.
    _, shadow_line, _ = source_position(data, b"say 'CLAUSE|shadow='")
    assert not any(at and at[2:4] == ('source-c.crexx', shadow_line) for _, at in begins), 'local DATE/TIME shadows do not request a reset'
    assert not called(body, 'llctarget.status') and not re.search(r'^\s*exit\s', body, re.M), 'clock frame returns its owned packet without a status bridge'

    # Each selected begin borrows the live activation reference; it neither
    # allocates another context nor copies an activation aggregate.
    previous_end = 0
    for begin in called(body, BEGIN):
        prefix = body[previous_end:begin.start()]
        getters = called(prefix, ACTIVATION)
        assert getters, 'lazy reset obtains current activation from canonical context'
        getter = getters[-1]; reference = getter[1].strip(); tail = prefix[getter.end():]
        linked = re.search(r'^\s*linkref\s+([^,\s]+),' + re.escape(reference) + r'\s*$', tail, re.M)
        assert linked, 'begin borrows the returned activation reference'
        assert not re.search(r'^\s*copy\s+[^,\n]+,' + re.escape(reference) + r'\s*$', tail, re.M), 'no full activation copy for a clause begin'
        following = body[begin.end():]
        assert re.search(r'^\s*unlink\s+' + re.escape(linked[1]) + r'\s*$', following, re.M), 'scoped activation borrow ends after reset'
        previous_end = begin.end()

    # Retain one shared WHILE predicate and place the repeat marker before
    # Body->Ready advance. The initial reset precedes constructor/captures.
    for needle in (b"do I = 'TIME'", b"do U = 'TIME'"):
        _, line, _ = source_position(data, needle)
        rows = [(row, at) for row, at in mapped_rows(body) if at and at[2:4] == ('source-c.crexx', line)]
        listing = '\n'.join(row for row, _ in rows)
        reset = called(listing, BEGIN); constructor = called(listing, 'llctarget.donew')
        assert len(reset) == 2 and len(constructor) == 1 and reset[0].start() < constructor[0].start(), 'initial boundary precedes once-only DO construction/captures'
    assert len(called(body, 'llctarget.whiletruth')) == 1, 'initial/repeated WHILE reuse one semantic predicate'
    assert len(called(body, 'llctarget.untiltruth')) == 1, 'first/repeated UNTIL reuse one semantic predicate'
    no_repeated_source(body)
    definitions = re.findall(r'^\S+\([^\n]*\s\.locals=\d+[^\n]*\.expose=(llctarget\.[^\s]+)\s*$', target, re.M)
    assert len(definitions) == len(set(definitions)) == 36, 'no new target export or clock helper'


def normalized_mapping(text):
    """Compare erased-marker output without unstable IR origin/clause numbers."""
    rows = []
    for row in unit(text).splitlines():
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            # Preserve zero/nonzero, source, flags, text and record multiplicity.
            fields[1:3] = [str(int(fields[1]) != 0), str(int(fields[2]) != 0)]
            row = repr(fields)
        rows.append(row)
    return rows


def raw_shapes(directory):
    marker = (directory / 'owner-2.rxas').read_text()
    plain = (directory / 'owner-3.rxas').read_text()
    assert normalized_mapping(marker) == normalized_mapping(plain), 'K-free begin plus exclusive C load have no executable/getter/COPY/source/diagnostic side effect'
    for name, count in (('owner-1', 1), ('owner-2', 0), ('owner-3', 0),
                        ('changed-1', 1), ('changed-11', 0), ('loop-1', 2),
                        ('loop-3', 2), ('call-continuation', 1)):
        body = unit((directory / (name + '.rxas')).read_text())
        assert len(called(body, BEGIN)) == count, ('raw selected marker/reset count', name, count)
        if count == 0:
            assert not called(body, TIME) and not called(body, DATE), ('whole owner is K-free after pruning', name)
        no_repeated_source(body)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
    parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
    parser.add_argument('--ir', choices=('on', 'off'), default='on')
    parser.add_argument('--phase', choices=('all', 'source', 'host', 'contracts', 'shapes'), default='all')
    parser.add_argument('--inspect', type=Path)
    args = parser.parse_args()
    fixtures = Path(__file__).resolve().parent; build = args.build.resolve()
    backend = build / 'research/level-l/compilers/backend' / args.mode
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    args.work.mkdir(parents=True, exist_ok=True); commands = 0
    with tempfile.TemporaryDirectory(prefix='clause-', dir=args.work) as temporary:
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
                shutil.copy2(build / 'bin' / (plugin + '.rxplugin'), providers / (plugin + '.rxplugin'))
            return directory, vm

        host, host_vm = stage('C compiler host', ('rxfs',))
        application, target_vm = stage('static application', ('rxfs', 'rxcstream'))
        compiler = host / 'llrxc-c-linked.rxbin'; shutil.copy2(backend / compiler.name, compiler)
        empty_path = work / 'empty PATH'; empty_path.mkdir()
        env = dict(os.environ, PATH=str(empty_path)); env.pop('CREXX_PROVIDER_PATH', None)
        assert not any(shutil.which(name, path=env['PATH']) for name in ('rxc', 'rxc-c'))

        def run(label, command, *, silent=False, cwd=work):
            nonlocal commands
            completed = subprocess.run([str(item) for item in command], cwd=cwd, env=env, capture_output=True, timeout=900)
            commands += 1; diagnostic = completed.stdout + completed.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
            assert completed.returncode == 0, (label, completed.returncode, diagnostic[-6000:])
            if silent:
                assert not diagnostic, (label, diagnostic[-4000:])
            return completed

        def inspect(path, name):
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, args.inspect / f'{name}-{args.mode}-{args.ir}-{args.vm}.rxas')

        if args.phase in ('all', 'source', 'host', 'shapes'):
            source = work / 'source-c.crexx'; data = (fixtures / source.name).read_bytes()
            source.write_bytes(data if args.mode == 'opt' else data.replace(b'\n', b'\r\n'))
            assembly = application / 'source.rxas'
            run('one C source compile', [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []),
                                        '-i', runtime, '-o', assembly, source], silent=True, cwd=host)
            inspect(assembly, 'clause-source'); text = assembly.read_text()
            assert str(work) not in text and str(build) not in text, 'stable logical source identity'
            shapes(text, source, (backend / 'llctarget.rxas').read_text())
            if args.phase != 'shapes':
                exported = application / 'source-exported.rxas'
                text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=clausefixture.run\2', text, count=1, flags=re.M)
                text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"clausefixture.run"', text, count=1, flags=re.M)
                assert headers == metadata == 1, 'one test-only exported entry0, no production ABI change'
                exported.write_text(text, encoding='utf-8'); inspect(exported, 'clause-exported')
                run('exported source assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), exported], silent=True)
                imports = work / 'B caller imports'; imports.mkdir()
                for provider in runtime.glob('*.rxbin'):
                    shutil.copy2(provider, imports / provider.name)
                shutil.copy2(exported.with_suffix('.rxbin'), imports / 'clausefixture.rxbin')
                caller = application / 'clause-host'
                run('B host compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import', '-i', imports, '-o', caller, fixtures / 'host.crexx'], silent=True)
                run('B host assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), caller], silent=True)
                linked = application / 'clause-host-linked'; control = work / 'host.ctl'
                inputs = [caller.with_suffix('.rxbin'), exported.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) + 'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run('B host link', [tool('rxlink'), '-c', control], silent=True)
                pause_script = work / 'portable pause command.py'
                pause_script.write_text('import time\ntime.sleep(1)\n', encoding='utf-8')
                pieces = [sys.executable, str(pause_script)]
                pause = subprocess.list2cmdline(pieces) if os.name == 'nt' else shlex.join(pieces)
                completed = run('one four-pause clock execution', [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', pause], cwd=application)
                expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
                actual = completed.stdout.replace(b'\r\n', b'\n')
                assert actual.startswith(expected), (actual, expected)
                assert re.fullmatch(rb'CLAUSE_HOST\|checks=\d+\|failures=0\n', actual[len(expected):]) and not completed.stderr, (completed.stdout, completed.stderr)

        if args.phase in ('all', 'contracts'):
            probe = host / 'clause-contracts-linked.rxbin'; shutil.copy2(backend / probe.name, probe)
            raw = work / 'raw RXAS'; raw.mkdir()
            completed = run('tiny raw owner/proof controls', [host_vm, '--no-autoload', probe, '-a', runtime, raw], cwd=host)
            assert re.fullmatch(rb'CLAUSE_CONTRACTS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
            raw_shapes(raw)
            for path in raw.glob('*.rxas'):
                inspect(path, 'clause-raw-' + path.stem)
    print(f'CLAUSE_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
