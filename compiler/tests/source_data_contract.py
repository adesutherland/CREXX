"""Source bytes must survive scanning, literals, imports and source metadata."""
import argparse
import json
import subprocess
import tempfile
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('--bindir', type=Path, required=True)
p.add_argument('--level', choices=('b', 'c', 'g', 'l'), required=True)
p.add_argument('--noopt', action='store_true')
p.add_argument('--linked', action='store_true')
a = p.parse_args()
a.bindir = a.bindir.resolve()
work = Path(tempfile.mkdtemp(prefix='crexx-source-data-'))
commands = []


def run(argv, expected=0):
    log = work / f'command-{len(commands)}.log'
    commands.append([str(x) for x in argv])
    with log.open('wb') as f:
        result = subprocess.run(commands[-1], cwd=work, stdout=f,
                                stderr=subprocess.STDOUT, timeout=300)
    output = log.read_bytes()
    if (result.returncode == 0) != (expected == 0):
        raise AssertionError(f'{commands[-1]}: exit {result.returncode}; {log}\n'
                             + output[-12000:].decode(errors='replace'))
    return output


def compile_run(stem, source):
    path = work / (stem + '.rexx' if a.level == 'c' else stem + '.crexx')
    path.write_bytes(source)
    output = work / stem
    run([a.bindir / 'rxc', *(['-n'] if a.noopt else []), '-i', a.bindir,
         '-o', output, path])
    run([a.bindir / 'rxas', '-o', str(output) + '.rxbin', output])
    image = str(output) + '.rxbin'
    libraries = [a.bindir / (x + '.rxbin') for x in ('library', 'classlib')]
    if a.level == 'c':
        libraries.append(a.bindir / 'rxfnsc.rxbin')
    if stem == 'imported':
        provider = work / 'source_provider'
        run([a.bindir / 'rxc', *(['-n'] if a.noopt else []), '-i', a.bindir,
             *(['--levelc-routine'] if a.level == 'c' else []),
             '-o', provider, work / ('source_provider.rexx' if a.level == 'c' else 'source_provider.crexx')])
        run([a.bindir / 'rxas', '-o', str(provider) + '.rxbin', provider])
        libraries.append(Path(str(provider) + '.rxbin'))
    if a.linked:
        image = str(output) + '_image.rxbin'
        run([a.bindir / 'rxlink', '-o', image, str(output) + '.rxbin', *libraries])
        libraries = []
    return run([a.bindir / 'rxvm', image, *libraries])


try:
    header = f'options level{a.level}\n'.encode()
    # NUL in a leading comment also exercises the options scanner.
    source = (b'/* header\x00comment */\n' + header
              + b"/* nested /* \x00 */ comment */\nsay 'a\x00b\xc3\xa9'\n"
              + b"say 'DONE'\nreturn 0\n")
    assert compile_run('literal', source) == b'a\x00b\xc3\xa9\nDONE\n'
    if a.level != 'c':
        source = header + "值 = '漢🙂'\nsay 值\nreturn 0\n".encode()
        assert compile_run('unicode', source) == '漢🙂\n'.encode()
        provider = work / 'source_provider.crexx'
        provider.write_bytes(header + b'namespace source_provider expose get_text\n'
                             + b'/* provider\x00comment */\nget_text: procedure = .string\n'
                             + b"return 'i\x00m'\n")
        source = header + b"import source_provider\nsay get_text()\nreturn 0\n"
        assert compile_run('imported', source) == b'i\x00m\n'
    else:
        source = header + b"/* \x00 */\nsay sourceline()\nsay sourceline(2)\nreturn 0\n"
        assert compile_run('lines', source) == b'5\n/* \x00 */\n'
        (work / 'source_provider.rexx').write_bytes(
            header + b"/* provider\x00comment */\nreturn 'i\x00m'\n")
        source = header + b'call source_provider\nsay result\nexit\n'
        assert compile_run('imported', source) == b'i\x00m\n'
    # Every physical newline spelling, including a final comment with no EOL.
    for i, eol in enumerate((b'\r\n', b'\r', b'\n')):
        source = eol.join((header.rstrip(b'\n'), b"/* \x00 */", b"say 'TAIL'"))
        source += eol + b'/* trailing\x00comment */'
        assert compile_run(f'newline{i}', source) == b'TAIL\n'
    bad = work / 'bad.crexx'
    bad.write_bytes(header + b"say 'BEFORE'\n\x00\nsay 'AFTER'\n")
    diagnostic = run([a.bindir / 'rxc', '--diagnostics', 'raw', '-i', a.bindir,
                      '-o', work / 'bad', bad], expected=1)
    assert (b'bad.crexx:3:' in diagnostic or b'bad.crexx @ 3:' in diagnostic), diagnostic[-12000:]
    print(f'PASS: source data level {a.level}, noopt={a.noopt}, linked={a.linked}')
finally:
    (work / 'commands.json').write_text(json.dumps(commands, indent=2))
    print(f'Source data evidence: {work}')
