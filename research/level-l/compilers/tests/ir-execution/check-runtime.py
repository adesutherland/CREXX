#!/usr/bin/env python3
"""One VM, one frozen semantic graph, independent exact runtime expectations."""
import argparse
from pathlib import Path
import subprocess
import tempfile


EXPECTED = [
    'SET|4100C3A9',
    'OBS|4954454D',
    'OBS|3432',
    'OBS|33',
    'OBS|00FF',
    'PAUSE|4100C3A9',
    'OBS|4100C3A9',
    'RETURN|33',
    'STATE|1|1|0|0|4100C3A9',
]


def run(argv, cwd):
    completed = subprocess.run([str(value) for value in argv], cwd=cwd,
                               capture_output=True, text=True, timeout=120)
    if completed.returncode or completed.stderr:
        raise AssertionError((argv, completed.returncode, completed.stdout,
                              completed.stderr))
    return completed.stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--rxas-out', type=Path)
    args = parser.parse_args()
    build = args.build.resolve()
    products = build / 'research/level-l/compilers/ir-execution'
    tools = build / 'bin'
    with tempfile.TemporaryDirectory(prefix='crexx-ir-runtime-') as temporary:
        work = Path(temporary)
        rxas = work / 'semantic-graph.rxas'
        output = run([tools / 'rxbvm', '--no-autoload',
                      products / 'runtime-contracts-linked.rxbin', '-a', rxas], work)
        assert 'IR_EXECUTION_RUNTIME|PASS' in output, output
        observed = [line.removeprefix('RUNNER|') for line in output.splitlines()
                    if line.startswith('RUNNER|')]
        assert observed == EXPECTED, ('runner independent oracle', observed)
        text = rxas.read_text(encoding='utf-8')
        assert text.count(',classicbridge.targetnew(),') == 1
        assert text.count(',classicbridge.targetfinish(),') == 1
        assert 'call ' in text and ',fixture2(),' in text
        assert '0x4100C3A9' in text and '0x00FF' in text
        assert 'mkref r1,r0' in text
        if args.rxas_out:
            args.rxas_out.parent.mkdir(parents=True, exist_ok=True)
            args.rxas_out.write_text(text, encoding='utf-8')
        run([tools / 'rxas', '-o', work / 'semantic-graph', rxas], work)
        modules = [work / 'semantic-graph.rxbin', products / 'classicbridge.rxbin']
        modules += [tools / name for name in (
            'library.rxbin', 'rxfnsc.rxbin', 'classlib.rxbin',
            'rxunicode-core.rxbin', 'rxfnsl.rxbin',
            'rxfnsl-analysis.rxbin', 'rxfnsl-execute.rxbin')]
        target = work / 'semantic-graph-linked'
        run([tools / 'rxlink', '-s', '-o', target, *modules], work)
        executed = run([tools / 'rxbvm', '--no-autoload',
                        target.with_suffix('.rxbin')], work)
        assert executed.splitlines() == EXPECTED, ('RXAS independent oracle', executed)
    print('IR_EXECUTION_RUNTIME|PASS')


if __name__ == '__main__':
    main()
