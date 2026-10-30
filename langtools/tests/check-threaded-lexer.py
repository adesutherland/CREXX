#!/usr/bin/env python3
"""One-VM threaded lexer parity, independent token spans and atomic publication.

All generated programs and advisory capsules live in an owned temporary root.
The raw worker/service contracts are exercised by threaded-lexer.crexx.
Unicode identifier properties are intersected with ASCII and five selected
scalars; the oracle reads the pinned UCD and independently applies that domain.
The full-property membership regression remains check-unicode-lexer.py.
Selected real grammars compare one worker mode with supplied same-source serial
RXAS/capsule products at the maintained dfa.states=16384 construction budget.
"""
import argparse
import bisect
import hashlib
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import time

p = argparse.ArgumentParser()
p.add_argument('--bin', type=Path, required=True)
p.add_argument('--probe', type=Path, help='prepared threaded-lexer-linked.rxbin')
p.add_argument('--grammar', type=Path, help='selected grammar for one prepared-baseline comparison')
p.add_argument('--serial-rxas', type=Path, help='prepared serial RXAS from the same selected grammar')
p.add_argument('--serial-lexer', type=Path, help='prepared serial capsule from the same selected grammar')
p.add_argument('--grammar-workers', type=int, choices=(1, 2, 4, 8), default=4)
p.add_argument('--case', choices=('all', 'small', 'unicode', 'grammar', 'contracts', 'tiles'), default='all')
p.add_argument('--cmake', default='cmake')
p.add_argument('--timeout', type=int, default=900)
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
binary = a.bin.resolve()
suffix = '.exe' if os.name == 'nt' else ''
workers = (0, 1, 2, 4, 8)
commands = 0
started = time.monotonic()


def run(command, *, success=True):
    global commands
    commands += 1
    command = list(map(str, command))
    result = subprocess.run(command, capture_output=True, timeout=a.timeout)
    data = result.stdout + result.stderr
    assert b'Warning in ' not in data and b'PANIC:' not in data, (command, data[-2000:])
    if success:
        assert result.returncode == 0, (command, result.returncode, data[-2000:])
    else:
        assert result.returncode != 0, (command, 'unexpected success')
        assert b'.globals=' not in result.stdout, (command, 'partial RXAS publication')
    return result


def tool(name):
    return binary / (name + suffix)


def digest(data):
    return hashlib.sha256(data).hexdigest()


sets = {'XID_Start': [], 'XID_Continue': []}
unicode_sample = ((0, 0x7f), (0xe9, 0xe9), (0x301, 0x301),
                  (0x3bb, 0x3bb), (0x5b57, 0x5b57), (0x10400, 0x10400))
raw = root / 'lib/rxfnsb/unicode/data/unicode-17.0.0/DerivedCoreProperties.txt'
for line in raw.read_text(encoding='utf-8').splitlines():
    body = line.split('#', 1)[0].strip()
    if not body:
        continue
    points, name, *_ = map(str.strip, body.split(';'))
    if name in sets:
        bounds = points.split('..')
        sets[name].append((int(bounds[0], 16), int(bounds[-1], 16)))


def contains(name, point):
    spans = sets[name]
    index = bisect.bisect_right(spans, (point, 0x110000)) - 1
    return index >= 0 and point <= spans[index][1]


def sampled(point):
    return any(low <= point <= high for low, high in unicode_sample)


def tokens(source, unicode):
    """Independent scalar membership and ordered maximal-munch oracle."""
    text = source.decode('utf-8')
    offsets = [0]
    for scalar in text:
        offsets.append(offsets[-1] + len(scalar.encode('utf-8')))
    result = []
    i = 0
    first = lambda c: c == '_' or (sampled(ord(c)) and contains('XID_Start', ord(c))
                                   if unicode else 'a' <= c <= 'z')
    rest = lambda c: c == '_' or (sampled(ord(c)) and contains('XID_Continue', ord(c))
                                  if unicode else 'a' <= c <= 'z' or '0' <= c <= '9')
    while i < len(text):
        start = i
        if text[i] in ' \t\r\n':
            i += 1
            while i < len(text) and text[i] in ' \t\r\n':
                i += 1
            continue
        if first(text[i]):
            i += 1
            while i < len(text) and rest(text[i]):
                i += 1
            symbol = 2 if text[start:i] == 'if' else 1
        elif text.startswith('->', i):
            symbol = 3
            i += 2
        else:
            symbol = 4 if text[i] == '-' else 5
            i += 1
        result.append((symbol, offsets[start], offsets[i] - offsets[start]))
    return result


with tempfile.TemporaryDirectory(prefix='crexx-threaded-lexer-') as temporary:
    work = Path(temporary)

    def parity(label, grammar):
        capsule = work / (label + '-0.lexer')
        serial = run([tool('rxlang'), grammar, '--rxas', '--lexer-workers', 0,
                      '--lexer-out', capsule]).stdout
        baseline = capsule.read_bytes()
        assert serial and baseline, (label, 'empty successful product')
        for count in workers[1:]:
            candidate = work / f'{label}-{count}.lexer'
            observed = run([tool('rxlang'), grammar, '--rxas', '--lexer-workers', count,
                            '--lexer-out', candidate]).stdout
            assert observed == serial, (label, count, 'RXAS', digest(serial), digest(observed))
            assert candidate.read_bytes() == baseline, (label, count, 'capsule mismatch')
        print(f'THREADED_LEXER_PARITY|{label}|modes=0,1,2,4,8|failures=0', flush=True)
        return serial

    def runtime(label, generated, unicode):
        target = work / label
        target.mkdir()
        (target / 'probe_language.rxas').write_bytes(generated)
        cases = [b'', b'if iffy if2 _ a0 -> - -- ->>', b'if\r\n_\t9\x00',
                 'é é0 ifé λ ifλ a\u0301 \u0301 字 𐐀_ 🙂'.encode('utf-8'),
                 'à β 東 𐐁 aβ ifβ'.encode('utf-8')]
        # Domain and encoding boundaries plus UCD edges outside the sample.
        points = {0, 0x7f, 0x80, 0x7ff, 0x800, 0xd7ff, 0xe000, 0xffff, 0x10000, 0x10ffff}
        for low, high in unicode_sample:
            points.update((low - 1, low, high, high + 1))
        for spans in sets.values():
            for low, high in spans[::max(1, len(spans) // 16)]:
                points.update((low - 1, low, high, high + 1))
        for point in sorted(points):
            if 0 <= point <= 0x10ffff and not 0xd800 <= point <= 0xdfff:
                value = chr(point).encode('utf-8')
                cases.extend((value, b'a' + value))
        payload = struct.pack('<I', len(cases)) + b''.join(struct.pack('<I', len(s)) + s for s in cases)
        (target / 'cases.bin').write_bytes(payload)
        run([tool('rxas'), target / 'probe_language'])
        run([tool('rxc'), '-x', '--no-exe-import', '-i', f'{target};{binary}',
             '-o', target / 'probe', root / 'langtools/tests/bootstrap-probe.crexx'])
        run([tool('rxas'), target / 'probe'])
        run([tool('rxlink'), '-s', '-o', target / 'linked', target / 'probe.rxbin',
             target / 'probe_language.rxbin', binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
        result = run([tool('rxbvm'), target / 'linked.rxbin', '-a', target / 'cases.bin']).stdout
        observed = []
        for line in result.decode('utf-8').splitlines():
            fields = line.split('|')
            if fields[0] == 'CASE':
                assert fields[2] == '0', (label, line)
                observed.append([])
            else:
                assert fields[0] == 'TOKEN' and observed, (label, line)
                observed[-1].append(tuple(map(int, fields[1:])))
        expected = [tokens(source, unicode) for source in cases]
        assert observed == expected, (label, 'independent token kind/byte-span mismatch')
        print(f'THREADED_LEXER_TOKENS|{label}|inputs={len(cases)}|failures=0', flush=True)

    def selected_grammar():
        assert a.grammar and a.serial_rxas and a.serial_lexer, (
            'selected grammar requires --grammar, --serial-rxas and --serial-lexer')
        grammar = a.grammar.resolve()
        source = grammar.read_bytes()
        serial = a.serial_rxas.read_bytes()
        baseline = a.serial_lexer.read_bytes()
        assert serial and baseline, 'empty prepared serial grammar products'
        candidate = work / 'selected-grammar.lexer'
        observed = run([tool('rxlang'), grammar, '--rxas', '--lexer-workers', a.grammar_workers,
                        '--limit', 'dfa.states=16384', '--lexer-out', candidate]).stdout
        assert grammar.read_bytes() == source, 'selected grammar changed during comparison'
        assert observed == serial, ('selected-grammar', a.grammar_workers, 'RXAS',
                                     digest(serial), digest(observed))
        assert candidate.read_bytes() == baseline, ('selected-grammar', a.grammar_workers, 'capsule mismatch')
        print(f'THREADED_LEXER_PARITY|selected-grammar|prepared=0|worker={a.grammar_workers}|failures=0', flush=True)

    if a.case in ('all', 'small', 'unicode'):
        names = ('small', 'unicode') if a.case == 'all' else (a.case,)
        for name in names:
            grammar = root / f'langtools/tests/grammars/threaded-lexer-{name}.levell'
            image = parity(name, grammar)
            runtime(name, image, name == 'unicode')

    if a.grammar and a.case in ('all', 'grammar'):
        selected_grammar()
    if a.case == 'grammar':
        assert a.grammar, '--case grammar requires --grammar'

    if a.case in ('all', 'small'):
        grammar = root / 'langtools/tests/grammars/threaded-lexer-small.levell'
        errors = []
        for count in workers:
            failed = run([tool('rxlang'), grammar, '--rxas', '--lexer-workers', count,
                          '--limit', 'dfa.states=1'], success=False)
            data = failed.stdout + failed.stderr
            assert b'LL_RESOURCE_LIMIT: dfa.states budget 1, required at least 2' in data, data[-2000:]
            errors.append(data)
        assert all(data == errors[0] for data in errors), 'worker count changed first resource diagnostic'
        for options in (('--lexer-workers', '3'), ('--lexer-workers', '-1'),
                        ('--lexer-workers',), ('--lexer-workers', '1', '--lexer-workers', '2')):
            run([tool('rxlang'), grammar, '--rxas', *options], success=False)
        output = work / 'published.rxas'
        wrapper = root / 'langtools/cmake/generate.cmake'
        command = [a.cmake, f'-DLEVEL_L={tool("rxlang")}', f'-DSPEC={grammar}',
                   f'-DOUTPUT={output}', '-DFORMAT=rxas', '-DREUSE_LEXER=ON',
                   '-DLEXER_WORKERS=4']
        run([*command, '-P', wrapper])
        owned = [output, Path(str(output) + '.lexer'), Path(str(output) + '.lexer.manifest')]
        saved = [(path.read_bytes(), path.stat().st_mtime_ns) for path in owned]
        run([*command, '-DGENERATOR_LIMITS=dfa.states=1', '-P', wrapper], success=False)
        assert saved == [(path.read_bytes(), path.stat().st_mtime_ns) for path in owned], 'failure replaced last-good publication'
        assert not list(work.glob('published.rxas.stage-*')), 'failed worker frontier retained staging'

    if a.probe and a.case in ('all', 'contracts', 'tiles'):
        selectors = ('tiles',) if a.case == 'tiles' else ('regular', 'transport')
        for selector in selectors:
            result = run([tool('rxbvm'), '--no-autoload', a.probe.resolve(), '-a', selector]).stdout
            assert b'|failures=0' in result and b'FAIL|' not in result, result[-2000:]
    if a.probe and a.case in ('all', 'contracts'):
        result = run([tool('rxbvm'), '--no-autoload', a.probe.resolve(), '-a', 'service',
                      root / 'langtools/tests/grammars/threaded-lexer-small.levell']).stdout
        assert b'|failures=0' in result and b'FAIL|' not in result, result[-2000:]
    if a.case in ('contracts', 'tiles'):
        assert a.probe, f'--case {a.case} requires --probe'

print(f'THREADED_LEXER|commands={commands}|elapsed={time.monotonic()-started:.3f}|failures=0')
