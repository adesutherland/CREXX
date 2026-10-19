#!/usr/bin/env python3
"""Fast repository ownership and disposable-output regression; no builds."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[2]
assert (root/'langtools/rxlang.crexx').is_file()
assert not (root/'langtools/level-l').exists()
assert not (root/'langtools/cli').exists()
assert (root/'langtools/doc/user-guide.md').is_file()
assert (root/'langtools/examples/settings.levell').is_file()
assert (root/'langtools/tests/library/ts_llir.crexx').is_file()
assert 'tinyexpr' not in (root/'lib/rxfnsl/rexx/CMakeLists.txt').read_text()
for old in ['tools','bootstrap','compilers','grammar','doc','examples','tests_functional']:
    p=root/'lib/rxfnsl'/old
    assert not p.exists() or not any(p.iterdir()), str(p)
for owner in ['lib/rxfnsl/rexx','langtools/common','langtools/rexx']:
    for p in (root/owner).glob('*.crexx'):
        imports=re.findall(r'^import\s+(\w+)',p.read_text(),re.M)
        assert not any(re.match(r'll(?:ccompiler|gcompiler|cfrontend|tfrontend|cparse|tparse|test)',x) for x in imports),str(p)
compiler=root/'research/level-l/compilers'
for owner,forbidden in [('levelc',r'll(?:tparse|tfrontend|tir|trxas|gcompiler|ggrammar|test)'),('typed',r'll(?:cparse|cfrontend|cir|crxas|ccompiler|cgrammar|ctarget|test)'),('levelg',r'll(?:cparse|cfrontend|cir|crxas|ccompiler|cgrammar|test)')]:
    for p in (compiler/owner).glob('*.crexx'):
        imports=re.findall(r'^import\s+(\w+)',p.read_text(),re.M)
        assert not any(re.fullmatch(forbidden,x) for x in imports),(str(p),imports)
        assert 'options levelb' in p.read_text(),str(p)
for owner in ['research/level-l','langtools','lib/rxfnsl']:
    for p in (root/owner).rglob('*'):
        if not p.is_file() or '__pycache__' in p.parts:continue
        assert not p.is_symlink(),str(p)
        assert 'evidence' not in p.relative_to(root).parts,str(p)
        assert p.suffix not in ['.log','.stdout','.stderr','.timing'],str(p)
        assert p.name not in ['receipt.json','manifest.json','hashes.json'],str(p)
print('LEVEL_L_LAYOUT|PASS')
