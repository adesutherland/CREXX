"""Source bytes survive scanning, literals, imports and source metadata."""
import argparse
import os
import tempfile
from pathlib import Path
from contract_fixture import ContractFixture, ProviderSource

p = argparse.ArgumentParser()
p.add_argument('--bindir', type=Path, required=True)
p.add_argument('--fixture-dir', type=Path)
p.add_argument('--level', choices=('b', 'c', 'g', 'l'), required=True)
p.add_argument('--prepare', action='store_true')
p.add_argument('--noopt', action='store_true')
p.add_argument('--linked', action='store_true')
a = p.parse_args()


def check(fixture, stem, source, expected, providers=()):
    if os.name == 'nt':
        expected = expected.replace(b'\n', b'\r\n')
    fixture.case(stem, source, expected=expected, classic=a.level == 'c',
                 filename=stem + ('.rexx' if a.level == 'c' else '.crexx'),
                 providers=providers, noopt=a.noopt, linked=a.linked)


def exercise(fixture):
    header = f'options level{a.level}\n'.encode()
    # NUL in a leading comment also exercises the options scanner.
    source = (b'/* header\x00comment */\n' + header
              + b"/* nested /* \x00 */ comment */\nsay 'a\x00b\xc3\xa9'\n"
              + b"say 'DONE'\nreturn 0\n")
    check(fixture, 'literal', source, b'a\x00b\xc3\xa9\nDONE\n')
    if a.level != 'c':
        source = header + "值 = '漢🙂'\nsay 值\nreturn 0\n".encode()
        check(fixture, 'unicode', source, '漢🙂\n'.encode())
        # L routine consumers import a B provider: authored L sources are
        # reserved for grammars. The provider bytes/result remain the oracle.
        provider_header = b'options levelb\n' if a.level == 'l' else header
        provider = ProviderSource('source_provider.crexx',
                                  provider_header + b'namespace source_provider expose get_text\n'
                                  + b'/* provider\x00comment */\nget_text: procedure = .string\n'
                                  + b"return 'i\x00m'\n")
        source = header + b"import source_provider\nsay get_text()\nreturn 0\n"
        check(fixture, 'imported', source, b'i\x00m\n', providers=(provider,))
    else:
        source = header + b"/* \x00 */\nsay sourceline()\nsay sourceline(2)\nreturn 0\n"
        check(fixture, 'lines', source, b'5\n/* \x00 */\n')
        provider = ProviderSource('source_provider.rexx',
                                  header + b"/* provider\x00comment */\nreturn 'i\x00m'\n",
                                  ('--levelc-routine',))
        source = header + b'call source_provider\nsay result\nexit\n'
        check(fixture, 'imported', source, b'i\x00m\n', providers=(provider,))
    # Every physical newline spelling, including a final comment with no EOL.
    for i, eol in enumerate((b'\r\n', b'\r', b'\n')):
        source = eol.join((header.rstrip(b'\n'), b"/* \x00 */", b"say 'TAIL'"))
        source += eol + b'/* trailing\x00comment */'
        check(fixture, f'newline{i}', source, b'TAIL\n')
    fixture.invalid_source('bad', header + b"say 'BEFORE'\n\x00\nsay 'AFTER'\n",
                           ('bad.crexx:3:', 'bad.crexx @ 3:'), filename='bad.crexx')

if a.fixture_dir:
    with ContractFixture(a.bindir, a.fixture_dir, [Path(__file__)], prepare=a.prepare) as fixture:
        exercise(fixture)
else:
    if a.prepare:
        p.error('--prepare requires --fixture-dir')
    # The manual/instrumented route still invokes each original producer per
    # case and selected mode. It uses this same generator, without cache reuse.
    with tempfile.TemporaryDirectory(prefix='crexx-source-data-') as temporary:
        with ContractFixture(a.bindir, temporary, [Path(__file__)], fresh=True) as fixture:
            exercise(fixture)
print(f'PREPARED: source data level {a.level}' if a.prepare else f'PASS: source data level {a.level}, noopt={a.noopt}, linked={a.linked}')
