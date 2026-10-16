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
result = record['staple'].pop(0) if sys.argv[2] == 'staple' else record['validate']
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
            ('immediate success', [0], 0, 0, 1, []),
            ('temporary service recovery', [68, 68, 0], 0, 0, 3, ['15', '30']),
            ('permanent failure', [65], 0, 65, 1, []),
            ('retry exhaustion', [68] * 4, 0, 68, 4, ['15', '30', '45']),
            ('validation refusal', [0], 65, 65, 1, []),
            ('validation service failure', [0], 68, 68, 1, []),
        ]
        for name, staple, validate, expected, attempts, delays in cases:
            state.write_text(json.dumps(dict(staple=staple, validate=validate,
                                             commands=[], delays=[])))
            result = subprocess.run(['bash', str(helper), package], env=env,
                                    capture_output=True, text=True, timeout=30)
            record = json.loads(state.read_text())
            assert result.returncode == expected, (name, result)
            assert record['commands'][:attempts] == [['stapler', 'staple', package]] * attempts, record
            expected_tail = [['stapler', 'validate', package]] if staple[-1] == 0 else []
            assert record['commands'][attempts:] == expected_tail, record
            assert record['delays'] == [[delay] for delay in delays], record
            assert 'fixture diagnostic:' in result.stdout, result
            print('PASS: ' + name)


if __name__ == '__main__':
    main()
