"""Allow focused candidate retries while keeping ordinary Deep QA complete."""
import json
import os
from pathlib import Path


def select(requested, event, ref):
    if event != 'workflow_dispatch' or not ref.startswith('refs/heads/temp/llama-release-'):
        requested = 'all'
    rows = json.loads((Path(__file__).resolve().parents[1] / '.github/qa/core-matrix.json').read_text())
    rows = [row for row in rows if requested in ('all', row['selection'])]
    if not rows:
        raise ValueError('Unknown core QA platform: ' + requested)
    return {'include': rows}, requested == 'all'


if __name__ == '__main__':
    matrix, full = select(os.environ.get('SELECTED_PLATFORM', 'all'),
                          os.environ['GITHUB_EVENT_NAME'], os.environ['GITHUB_REF'])
    with open(os.environ['GITHUB_OUTPUT'], 'a') as output:
        output.write('core_matrix=' + json.dumps(matrix) + '\n')
        output.write('full_gate=' + str(full).lower() + '\n')
    with open(os.environ['GITHUB_STEP_SUMMARY'], 'a') as summary:
        summary.write('Core QA: ' + ('complete gate' if full else 'selected-platform retry only') + '.\n')
