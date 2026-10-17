#!/usr/bin/env python3
"""Exercise installer publication gates without signing or Apple service access."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    helper = Path(__file__).with_name('staple-macos-installer.sh')
    with tempfile.TemporaryDirectory(prefix='crexx-stapling-') as directory:
        work = Path(directory)
        xcrun = work / 'xcrun'
        xcrun.write_text('#!' + sys.executable + '\n' + '''
import json, os, sys
from pathlib import Path
state = Path(os.environ['STAPLING_FIXTURE_STATE'])
record = json.loads(state.read_text())
record['commands'].append(sys.argv[1:])
result = record[sys.argv[2]].pop(0)
state.write_text(json.dumps(record))
print('fixture diagnostic: ' + str(result))
sys.exit(result)
''', encoding='utf-8')
        xcrun.chmod(0o755)
        sleep = work / 'sleep'
        sleep.write_text('#!' + sys.executable + '\n' + '''
import json, os, sys
from pathlib import Path
state = Path(os.environ['STAPLING_FIXTURE_STATE'])
record = json.loads(state.read_text())
record['delays'].append(sys.argv[1:])
state.write_text(json.dumps(record))
''', encoding='utf-8')
        sleep.chmod(0o755)
        package = str(work / 'installer with spaces.pkg')
        state = work / 'state.json'
        env = dict(os.environ, PATH=str(work) + os.pathsep + os.environ['PATH'],
                   STAPLING_FIXTURE_STATE=str(state))
        cases = [
            ('immediate success', [0], [0], 0, []),
            ('temporary staple recovery', [68, 68, 0], [0], 0, ['15', '30']),
            ('permanent staple failure', [65], [], 65, []),
            ('staple refusal after service failure', [68, 65], [], 65, ['15']),
            ('staple retry exhaustion', [68] * 4, [], 68, ['15', '30', '45']),
            ('validation refusal', [0], [65], 65, []),
            ('validation service recovery', [0], [68, 0], 0, ['15']),
            ('last validation attempt succeeds', [0], [68, 68, 68, 0], 0, ['15', '30', '45']),
            ('validation retry exhaustion', [0], [68] * 4, 68, ['15', '30', '45']),
            ('validation refusal after service failure', [0], [68, 65], 65, ['15']),
            ('both actions recover', [68, 0], [68, 0], 0, ['15', '15']),
            ('independent retry caps', [68, 68, 68, 0], [68, 68, 68, 0], 0,
             ['15', '30', '45', '15', '30', '45']),
        ]
        for name, staple, validate, expected, delays in cases:
            state.write_text(json.dumps(dict(staple=staple, validate=validate,
                                             commands=[], delays=[])))
            result = subprocess.run(['bash', str(helper), package], env=env,
                                    capture_output=True, text=True, timeout=30)
            record = json.loads(state.read_text())
            assert result.returncode == expected, (name, result)
            expected_commands = ([['stapler', 'staple', package]] * len(staple) +
                                 [['stapler', 'validate', package]] * len(validate))
            assert record['commands'] == expected_commands, record
            assert record['delays'] == [[delay] for delay in delays], record
            assert result.stdout.splitlines() == [
                'fixture diagnostic: ' + str(code) for code in staple + validate], result
            print('PASS: ' + name)
        for arguments in ([], [package, package]):
            state.write_text(json.dumps(dict(commands=[], delays=[])))
            result = subprocess.run(['bash', str(helper), *arguments], env=env,
                                    capture_output=True, text=True, timeout=30)
            assert result.returncode == 64 and 'Usage:' in result.stderr, result
            assert json.loads(state.read_text()) == dict(commands=[], delays=[])
            print('PASS: invalid argument count ' + str(len(arguments)))


if __name__ == '__main__':
    main()
