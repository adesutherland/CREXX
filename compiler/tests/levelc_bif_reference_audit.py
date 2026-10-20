"""Shared two-mode preparation; unchanged four-cell BIF expected-value audit."""
import argparse
from pathlib import Path
from contract_fixture import ContractFixture

p = argparse.ArgumentParser()
p.add_argument('--bindir', type=Path, required=True)
p.add_argument('--fixture-dir', type=Path, required=True)
p.add_argument('--prepare', action='store_true')
p.add_argument('--noopt', action='store_true')
p.add_argument('--linked', action='store_true')
a = p.parse_args()
source = Path(__file__).parent / 'rexx_src/levelc_bif_reference_audit.rexx'
expected = source.with_suffix('.expected')
with ContractFixture(a.bindir, a.fixture_dir, [Path(__file__), source, expected], prepare=a.prepare) as fixture:
    fixture.case('reference_audit', source.read_text(encoding='utf-8'),
                 expected=expected.read_text(encoding='utf-8'), noopt=a.noopt,
                 linked=a.linked, stderr_empty=True)
print(f'PASS: Level C BIF reference audit, noopt={a.noopt}, linked={a.linked}')
