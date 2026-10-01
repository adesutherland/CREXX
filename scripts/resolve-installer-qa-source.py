"""Validate retained Build inputs, allowing only an explicit Mac packaging retry."""
import argparse
import json
import os
from pathlib import Path
import subprocess

MAC_INSTALLER_STEP = 'Package, notarize and staple optional macOS plugin installer'
MAC_JOBS = {'Plugin macOS arm64 Metal', 'Plugin macOS x86_64 CPU'}
REQUIRED_JOBS = MAC_JOBS | {
    'Core linux-x64', 'Core windows-x64', 'Core macos-arm64', 'Core macos-x86_64',
    'Windows MinGW core quality gate (no binary delivery) / Core windows-mingw (mingw)',
    'Plugin Linux x64 Vulkan', 'Plugin Linux x64 CUDA',
    'Plugin Windows x64 Vulkan (MSVC)', 'Plugin Windows x64 CUDA (MSVC)',
}


def validate(run, jobs, allow_retry=False):
    if run['status'] != 'completed' or run['path'] != '.github/workflows/build.yml':
        raise ValueError('Source is not a completed Build run')
    if run['conclusion'] == 'success':
        return
    if not allow_retry or run['conclusion'] != 'failure':
        raise ValueError('Source Build did not succeed')
    if not REQUIRED_JOBS <= {job['name'] for job in jobs}:
        raise ValueError('Incomplete source job matrix')
    for job in jobs:
        if job['name'] in REQUIRED_JOBS and job['conclusion'] == 'skipped':
            raise ValueError('Required source job skipped')
        if job['conclusion'] in ('success', 'skipped'):
            continue
        failures = [step['name'] for step in job['steps'] if step['conclusion'] == 'failure']
        passed = {step['name'] for step in job['steps'] if step['conclusion'] == 'success'}
        if (job['status'] != 'completed' or job['conclusion'] != 'failure' or
                job['name'] not in MAC_JOBS or failures != [MAC_INSTALLER_STEP] or
                not {'Smoke combined core and plugin downloads',
                     'Upload qualified optional plugin', 'Notarize macOS ZIP asset'} <= passed):
            raise ValueError('Source has a failure outside Mac installer packaging: ' + job['name'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--run-id', type=int, required=True)
    parser.add_argument('--allow-mac-installer-retry', action='store_true')
    args = parser.parse_args()
    base = f"repos/{os.environ['GITHUB_REPOSITORY']}/actions/runs/{args.run_id}"
    run = json.loads(subprocess.check_output(['gh', 'api', base]))
    pages = json.loads(subprocess.check_output(['gh', 'api', base + '/jobs?filter=latest&per_page=100',
                                               '--paginate', '--slurp']))
    jobs = [job for page in pages for job in page['jobs']]
    root = Path(os.environ['RUNNER_TEMP'])
    (root / 'source.json').write_text(json.dumps(run, indent=2) + '\n')
    (root / 'source-jobs.json').write_text(json.dumps(jobs, indent=2) + '\n')
    validate(run, jobs, args.allow_mac_installer_retry)
    with open(os.environ['GITHUB_OUTPUT'], 'a') as stream:
        stream.write('commit=' + run['head_sha'] + '\n')
    print(run['head_sha'])


if __name__ == '__main__':
    main()
