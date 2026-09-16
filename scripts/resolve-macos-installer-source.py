"""Require successful packaging and matching product before fresh-host offline QA."""
import json
import os
from pathlib import Path
import subprocess


def main():
    run_id = str(int(os.environ['MAC_PACKAGE_RUN']))
    repo = os.environ['GITHUB_REPOSITORY']
    platform = os.environ['PLATFORM']
    run = json.loads(subprocess.check_output(['gh', 'api', f'repos/{repo}/actions/runs/{run_id}']))
    if run['status'] != 'completed' or run['path'] != '.github/workflows/llama-installer-qa.yml':
        raise ValueError('Not a completed installer QA source')
    jobs = json.loads(subprocess.check_output(['gh', 'api', f'repos/{repo}/actions/runs/{run_id}/jobs']))['jobs']
    job = next(j for j in jobs if j['name'] == 'Installer lifecycle ' + platform)
    if not any(s['name'] == 'Repackage and notarize retained signed Mac plugin' and
               s['conclusion'] == 'success' for s in job['steps']):
        raise ValueError('Source signed packaging did not pass')
    proof = Path(os.environ['RUNNER_TEMP']) / 'proof' / 'packaging-source'
    subprocess.run(['gh', 'run', 'download', run_id, '--repo', repo,
        '--name', 'installer-qa-' + platform + '-' + run['head_sha'], '--dir', str(proof)], check=True)
    product = json.loads((proof / 'source.json').read_text())
    if product['head_sha'] != os.environ['PRODUCT_COMMIT']:
        raise ValueError('Signed package source differs from qualified core')
    (proof / 'packaging-job.json').write_text(json.dumps(job, indent=2) + '\n')
    with open(os.environ['GITHUB_OUTPUT'], 'a') as stream:
        stream.write('revision=' + run['head_sha'] + '\n')


if __name__ == '__main__':
    main()
