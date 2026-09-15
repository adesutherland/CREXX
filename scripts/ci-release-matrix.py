"""Select diagnostic lanes without allowing partial publication matrices."""
import json
import os
from pathlib import Path


def select(lane, event, ref):
    matrix = json.loads((Path(__file__).resolve().parents[1] /
                         '.github/llama/release-matrix.json').read_text())
    if event != 'workflow_dispatch' or not ref.startswith('refs/heads/temp/llama-release-'):
        lane = 'all'
    if lane != 'all':
        matrix['include'] = [item for item in matrix['include'] if
                             (lane == 'base' and not item['cuda']) or
                             (lane == 'cuda' and item['cuda']) or item['id'] == lane]
    if not matrix['include']:
        raise ValueError('Unknown release lane: ' + lane)
    return matrix


if __name__ == '__main__':
    selected = select(os.environ.get('CREXX_CANDIDATE_LANE', 'all'),
                      os.environ['GITHUB_EVENT_NAME'], os.environ['GITHUB_REF'])
    encoded = json.dumps(selected, separators=(',', ':'))
    with open(os.environ['GITHUB_OUTPUT'], 'a') as f:
        f.write('matrix=' + encoded + '\n')
    print(encoded)
