"""Allow focused candidate retries while keeping ordinary Deep QA complete."""
import json
import os
from pathlib import Path


def select(requested, event, ref):
    if event != 'workflow_dispatch':
        requested = 'all'
    rows = json.loads((Path(__file__).resolve().parents[1] / '.github/qa/core-matrix.json').read_text())
    rows = [row for row in rows if requested in ('all', row['selection'])
            or (requested == 'windows' and row['platform'] == 'windows')]
    if not rows:
        raise ValueError('Unknown core QA platform: ' + requested)
    return {'include': rows}, requested == 'all'


def select_jobs(platform, family, event, ref):
    if event != 'workflow_dispatch':
        platform, family = 'all', 'all'
    if family not in ('all', 'comprehensive', 'release', 'books'):
        raise ValueError('Unknown Deep QA job family: ' + family)
    matrix, all_platforms = select(platform, event, ref)
    if family in ('release', 'books') and not all_platforms:
        raise ValueError(f'Deep QA {family} requires platform=all')
    full = family == 'all' and all_platforms
    return matrix, {
        'full_gate': full,
        'run_comprehensive': family in ('all', 'comprehensive'),
        'run_release': family == 'release' or full,
        'run_books': family == 'books' or full,
        'run_stress': full,
    }


if __name__ == '__main__':
    matrix, selection = select_jobs(os.environ.get('SELECTED_PLATFORM', 'all'),
                                    os.environ.get('SELECTED_JOB_FAMILY', 'all'),
                                    os.environ['GITHUB_EVENT_NAME'], os.environ['GITHUB_REF'])
    with open(os.environ['GITHUB_OUTPUT'], 'a') as output:
        output.write('core_matrix=' + json.dumps(matrix) + '\n')
        for name, enabled in selection.items():
            output.write(name + '=' + str(enabled).lower() + '\n')
    with open(os.environ['GITHUB_STEP_SUMMARY'], 'a') as summary:
        summary.write('Core QA: ' + ('complete gate' if selection['full_gate']
                                    else 'selected-family/platform retry only') + '.\n')
