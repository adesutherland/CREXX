"""Compiled DATE/TIME clause samples and program-wide elapsed/reset lifetime."""
import argparse
import os
import tempfile
import sys
from pathlib import Path
from contract_fixture import ContractFixture

p = argparse.ArgumentParser()
p.add_argument('--bindir', type=Path, required=True)
p.add_argument('--fixture-dir', type=Path)
p.add_argument('--prepare', action='store_true')
p.add_argument('--noopt', action='store_true')
p.add_argument('--linked', action='store_true')
a = p.parse_args()

def exercise(fixture):
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
    expected = b'PASS: compiled Classic clock lifetime\r\n' if os.name == 'nt' else b'PASS: compiled Classic clock lifetime\n'
    fixture.case('clock', source, expected=expected,
                 filename='clock.rexx', noopt=a.noopt, linked=a.linked)

if a.fixture_dir:
    with ContractFixture(a.bindir, a.fixture_dir, [Path(__file__)], prepare=a.prepare) as fixture:
        exercise(fixture)
else:
    if a.prepare:
        p.error('--prepare requires --fixture-dir')
    with tempfile.TemporaryDirectory(prefix='crexx-clock-contract-') as temporary:
        with ContractFixture(a.bindir, temporary, [Path(__file__)], fresh=True) as fixture:
            exercise(fixture)
print(f'PREPARED: clock contract' if a.prepare else f'PASS: clock contract, noopt={a.noopt}, linked={a.linked}')
