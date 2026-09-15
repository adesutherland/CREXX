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


def cores_for(matrix):
    """Each selected provider consumes one of these independently built cores."""
    cores = {}
    for row in matrix['include']:
        platform = row['core_platform']
        if platform in cores:
            continue
        args = ''
        if row['toolchain'] == 'msvc':
            args = '-DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DENABLE_PARSER_MODE=OFF'
        elif row['platform'] == 'macos':
            args = '-DCMAKE_OSX_ARCHITECTURES=' + platform.removeprefix('macos-')
        cores[platform] = dict(row, artifact_name='CREXX-' + platform,
            name='Core ' + platform, cuda=False, backends='', cmake_args=args,
            preferred_vm='rxtvm' if row['platform'] == 'linux' else 'rxbvm')
    return dict(include=list(cores.values()))


if __name__ == '__main__':
    selected = select(os.environ.get('CREXX_CANDIDATE_LANE', 'all'),
                      os.environ['GITHUB_EVENT_NAME'], os.environ['GITHUB_REF'])
    encoded = json.dumps(selected, separators=(',', ':'))
    with open(os.environ['GITHUB_OUTPUT'], 'a') as f:
        f.write('matrix=' + encoded + '\n')
        f.write('core_matrix=' + json.dumps(cores_for(selected), separators=(',', ':')) + '\n')
    print(encoded)
