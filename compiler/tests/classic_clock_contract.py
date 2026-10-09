"""Compiled DATE/TIME clause samples and program-wide elapsed/reset lifetime."""
import argparse
import json
import subprocess
import sys
import tempfile
import os
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('--bindir', type=Path, required=True)
p.add_argument('--noopt', action='store_true')
p.add_argument('--linked', action='store_true')
a = p.parse_args()
a.bindir = a.bindir.resolve()
work = Path(tempfile.mkdtemp(prefix='crexx-clock-contract-'))
commands = []


def run(argv):
    log = work / f'command-{len(commands)}.log'
    commands.append([str(x) for x in argv])
    with log.open('wb') as f:
        result = subprocess.run(commands[-1], cwd=work, stdout=f,
                                stderr=subprocess.STDOUT, timeout=300)
    output = log.read_bytes()
    if result.returncode:
        raise AssertionError(f'exit {result.returncode}; {log}\n'
                             + output[-12000:].decode(errors='replace'))
    return output.replace(b"\r\n", b"\n") if os.name == "nt" else output


try:
    # Existing SYSTEM transport, with fixed platform commands and no new host API.
    pause = ('powershell -NoProfile -NonInteractive -Command "Start-Sleep -Milliseconds 1100"'
             if sys.platform == 'win32' else 'sleep 1')
    source = """options levelc
start=time('E')
if start <> 0 then exit 81
pair=time('L') || '|' || time('L')
parse var pair first '|' second
if first \\== second then exit 82
before=time('L')
address SYSTEM 'PAUSE'
if rc <> 0 then exit 83
if before == time('L') then exit 84
if time('E') < 0.5 then exit 85
if shared() < 0.5 then exit 86
if private() < 0.5 then exit 87
pair=time('L') || '|' || delayed() || '|' || time('L')
parse var pair first '|' middle '|' last
if first \\== last | first == middle then exit 88
pair=date('B') || '|' || time('L') || '|' || date('B')
parse var pair first '|' middle '|' last
if first \\== last then exit 89
pair=time('R') || '|' || time('E')
parse var pair elapsed '|' restarted
if elapsed < 0.5 | restarted <> 0 then exit 90
-- Condition sampling repeats even when a loop body has no statements.
start=time('E')
do while time('E') < 0.05
end
say 'PASS: compiled Classic clock lifetime'
exit
shared:
return time('E')
private: procedure
return time('E')
delayed: procedure
address SYSTEM 'PAUSE'
if rc <> 0 then exit 91
return time('L')
""".replace('PAUSE', pause).replace('-- Condition sampling repeats even when a loop body has no statements.',
                                    '/* Condition sampling repeats for an empty loop body. */')
    path = work / 'clock.rexx'
    path.write_text(source, encoding="utf-8")
    output = work / 'clock'
    run([a.bindir / 'rxc', *(['-n'] if a.noopt else []), '-i', a.bindir,
         '-o', output, path])
    run([a.bindir / 'rxas', '-o', str(output) + '.rxbin', output])
    libraries = [a.bindir / (x + '.rxbin') for x in ('library', 'classlib', 'rxfnsc')]
    image = str(output) + '.rxbin'
    if a.linked:
        image = str(output) + '_image.rxbin'
        run([a.bindir / 'rxlink', '-o', image, str(output) + '.rxbin', *libraries])
        libraries = []
    assert run([a.bindir / 'rxvm', image, *libraries]) == b'PASS: compiled Classic clock lifetime\n'
    print(f'PASS: clock contract, noopt={a.noopt}, linked={a.linked}')
finally:
    (work / 'commands.json').write_text(json.dumps(commands, indent=2), encoding="utf-8")
    print(f'Clock evidence: {work}')
