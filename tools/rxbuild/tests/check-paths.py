#!/usr/bin/env python3
"""One bounded rxbuild failure path; it never reaches a compiler invocation."""
import argparse
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--rxbuild', required=True, type=Path)
parser.add_argument('--case', required=True, choices=('missing', 'blocked'))
args = parser.parse_args()
root = Path(__file__).resolve().parents[3]
with tempfile.TemporaryDirectory(prefix='rxbuild paths ') as temp:
    temp = Path(temp)
    parent = temp/'new parent'
    if args.case == 'blocked':
        parent.write_text('must survive')
    output = parent/'nested output'
    plan = temp/'plan.build'
    plan.write_text(f'''COMPILE GRAMMAR OUTPUT_DIR = "{output.as_posix()}" SOURCE = "tools/rxbuild/no-such-grammar.levell" OUTPUT = "grammar";
COMPILE READER OUTPUT_DIR = "{output.as_posix()}" SOURCE = "missing.crexx" DEPENDS_ON = "grammar" OUTPUT = "reader";
RUN WORK_DIR = "{output.as_posix()}" PROGRAM = "reader" STDIN = "";
''')
    result = subprocess.run([str(args.rxbuild.resolve()), str(plan)], cwd=root/'tools/rxbuild',
                            capture_output=True, text=True, timeout=60)
    text = result.stdout+result.stderr
    assert result.returncode != 0, text
    assert len(text) < 20000, text[-1500:]  # no runaway parent-path recursion
    if args.case == 'blocked':
        assert parent.read_text() == 'must survive'
        assert 'RXBUILD: cannot create directory' in text, text
    else:
        assert output.is_dir(), text[-1500:]
        source = next(line.split(': ', 1)[1] for line in text.splitlines() if line.startswith('BUILD GRAMMAR: '))
        assert Path(source).resolve() == root/'tools/rxbuild/no-such-grammar.levell', text
        assert 'BUILD GRAMMAR:' in text, text
    print('PASS: rxbuild '+args.case+' output directory and error propagation')
