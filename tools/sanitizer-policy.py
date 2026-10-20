#!/usr/bin/env python3
"""Exact sanitizer membership, conservative cadence and qualified result scope."""
import argparse
import hashlib
import importlib.util
import json
import os
import platform
from pathlib import Path
import re
import subprocess
import sys
import xml.etree.ElementTree as ET

MOVED = {
    'expression': ('classic_expression_contract_opt', 'classic_expression_contract_linked_noopt'),
    'bif': ('levelc_bif_reference_audit_opt', 'levelc_bif_reference_audit_linked_noopt'),
    'structured-flow': tuple(f'ts_llstructuredflow_{backend}_noopt_{engine}'
                             for backend in ('lr1', 'packrat', 'earley')
                             for engine in ('rxbvm', 'rxtvm')),
}
DAILY_MOVED = frozenset(name for family in MOVED.values() for name in family)
BROAD_PROFILES = {'full', 'weekly', 'native', 'affected'}
PROFILES = sorted(BROAD_PROFILES | {'daily'})


def fixture_tools():
    spec = importlib.util.spec_from_file_location('qa_fixtures', Path(__file__).with_name('sanitizer-fixtures.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def excluded(profile):
    return '^(' + '|'.join(map(re.escape, sorted(DAILY_MOVED))) + ')$' if profile == 'daily' else ''


def decide(event, activated, weekday, changed=None, requested='full'):
    if event == 'tag':
        return {'profile': 'full', 'reason': 'version tags retain original broad qualification'}
    if event == 'manual':
        return {'profile': requested, 'reason': 'explicit manual scope; default is full'}
    if not activated:
        return {'profile': 'full', 'reason': 'daily activation is disabled pending qualification/master decision'}
    if weekday == 7:
        return {'profile': 'weekly', 'reason': 'weekly broad safety net with original instrumented producers'}
    if changed is None:
        return {'profile': 'full', 'reason': 'missing qualification history or unavailable comparison'}
    # Uncertain/shared/native changes restore every family and original producer
    # scope. This conservative first tranche does not infer narrow native impact.
    known_roots = ('lib/', 'langtools/', 'research/level-l/', 'compiler/tests/rexx_src/',
                   'tests/', 'examples/', 'docs/')
    for name in changed:
        if (Path(name).name in {'CMakeLists.txt', 'AGENTS.md'} or
                any(family in name for family in ('classic_expression_contract',
                                                  'levelc_bif_reference_audit', 'ts_llstructuredflow'))):
            return {'profile': 'full', 'reason': f'build/policy or affected moved-family contract: {name}'}
        allowed_suffix = Path(name).suffix.lower() in {'.crexx', '.rexx', '.expected', '.md', '.tex'}
        allowed_suffix = allowed_suffix or (Path(name).suffix.lower() == '.txt' and '/unicode/data/' in name)
        if (not name.startswith(known_roots) and name not in ('README.md', 'AGENTS.md') or
                not allowed_suffix or
                name.startswith(('lib/rxfnsl/', 'langtools/'))):
            return {'profile': 'full', 'reason': f'native/shared/uncertain affected scope: {name}'}
    return {'profile': 'daily', 'reason': 'known source/document-only scope; unknown and new CTests stay included'}


def changed_files(source, base, head):
    if not base:
        return None
    test = subprocess.run(['git', '-C', str(source), 'merge-base', '--is-ancestor', base, head],
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if test.returncode:
        return None
    result = subprocess.run(['git', '-C', str(source), 'diff', '--name-only', '-z', base, head],
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return result.stdout.decode().rstrip('\0').split('\0') if result.returncode == 0 and result.stdout else ([] if result.returncode == 0 else None)


def ctest_inventory(args):
    result = subprocess.run(['ctest', '--show-only=json-v1', *args], text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
    value = json.loads(result.stdout)
    names = [test['name'] for test in value['tests']]
    if len(names) != len(set(names)):
        raise ValueError('duplicate CTest names cannot qualify a profile')
    return value, set(names)


def result_identity(build):
    tools = fixture_tools()
    config = tools.json_read(Path(build) / tools.CONFIG)
    value = {
        'source': tools.source_identity(config['CMAKE_HOME_DIRECTORY']),
        'configuration': config,
        'dependencies': tools.dependency_identity(config),
        'preparation': 'external-ordinary-fixtures' if config.get('CREXX_QA_FIXTURE_TREE') else 'original-producers',
        'execution': {key: os.environ.get(key, '') for key in (
            'ASAN_OPTIONS', 'LSAN_OPTIONS', 'CREXX_QA_BUILD_LEAKS', 'CREXX_QA_PHASE',
            'ImageVersion', 'ImageOS', 'RUNNER_ARCH', 'RUNNER_OS')},
        'host': {'system': platform.system(), 'machine': platform.machine(),
                 'release': platform.release(), 'mac_version': platform.mac_ver()[0]},
        'routing': {key: hashlib.sha256(os.environ.get(key, '').encode()).hexdigest()
                    for key in ('PATH', 'CREXX_HOME', 'CREXX_PROVIDER_PATH', 'CREXX_LIBRARY_PATH',
                                'LD_LIBRARY_PATH', 'DYLD_LIBRARY_PATH', 'CC', 'CXX', 'CFLAGS', 'CXXFLAGS', 'LDFLAGS')},
    }
    native_list = Path(build) / 'qa-native-files.txt'
    if native_list.exists():
        value['native_products'] = {
            str(Path(name).relative_to(Path(build).resolve())): tools.digest(name)
            for name in native_list.read_text().splitlines()
            if name and Path(name).is_file()
        }
    if config.get('CREXX_QA_FIXTURE_TREE'):
        value['donor_seal'] = tools.digest(Path(config['CREXX_QA_FIXTURE_TREE']) / tools.MANIFEST)
    return value


def canonical_input(build, profile, execution=None, native_only=False):
    """Reusable assurance inputs, separate from exact observed binary receipts.

    Generated build timestamp/commit/dirty decorations are presentation metadata.
    Source contents, semantic version/channel, tool binaries, flags, dependencies
    and runtime policy remain part of the key. Tag/manual runs remain unconditional.
    """
    tools = fixture_tools()
    original_build = str(Path(build).absolute())
    build = Path(build).resolve()
    config = tools.json_read(build / tools.CONFIG)
    source = Path(config['CMAKE_HOME_DIRECTORY']).resolve()
    replacements = [(str(build), '<BUILD>'), (original_build, '<BUILD>'),
                    (config.get('CMAKE_BINARY_DIR', str(build)), '<BUILD>'),
                    (str(source), '<SOURCE>'), (config['CMAKE_HOME_DIRECTORY'], '<SOURCE>')]
    for name, label in (('CREXX_QA_FIXTURE_TREE', '<DONOR>'), ('DSLSH_SOURCE_DIR', '<DSLSH>')):
        if config.get(name):
            replacements.extend([(str(Path(config[name]).resolve()), label), (config[name], label)])
    if os.environ.get('RUNNER_TEMP'):
        replacements.append((os.environ['RUNNER_TEMP'], '<RUNNER_TEMP>'))
    replacements.sort(key=lambda item: len(item[0]), reverse=True)
    def normalize(value):
        for path, label in replacements:
            value = value.replace(path, label)
        return value
    derived = {'CREXX_BUILD_TIMESTAMP', 'CREXX_BUILD_COMMIT', 'CREXX_BUILD_DIRTY',
               'CREXX_VERSION', 'CREXX_VERSION_SEMVER', 'CREXX_VERSION_DISPLAY',
               'CREXX_VERSION_BUILD_METADATA_EXTRA'}
    configuration = {key: normalize(value) for key, value in config.items()
                     if key not in derived and key not in {'CREXX_QA_FIXTURE_TREE', 'DSLSH_SOURCE_DIR'}}
    native_tools = {}
    for name in ('CMAKE_C_COMPILER', 'CMAKE_CXX_COMPILER'):
        path = config.get(name)
        if path:
            native_tools[name] = tools.digest(path)
    if config.get('CMAKE_MAKE_PROGRAM'):
        native_tools['build_executor'] = tools.digest(config['CMAKE_MAKE_PROGRAM'])
    for name, arguments in (('cmake', ['--version']),):
        result = subprocess.run([name, *arguments], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
        native_tools[name] = result.stdout.decode().splitlines()[0]
    native_tools['python'] = platform.python_version()
    execution = execution or {key: os.environ.get(key, '') for key in
                              ('ASAN_OPTIONS', 'LSAN_OPTIONS', 'CREXX_QA_BUILD_LEAKS', 'CREXX_QA_PHASE')}
    value = {'schema': 1, 'profile': 'broad' if profile in BROAD_PROFILES else profile,
             'preparation': 'external-ordinary-fixtures' if config.get('CREXX_QA_FIXTURE_TREE') else 'original-producers',
             'source': tools.source_identity(source), 'configuration': configuration,
             'dependencies': tools.dependency_identity(config), 'tools': native_tools,
             'execution': execution,
             'host': {'system': platform.system(), 'machine': platform.machine(),
                      'release': platform.release(), 'mac_version': platform.mac_ver()[0],
                      'image_version': os.environ.get('ImageVersion', ''), 'image_os': os.environ.get('ImageOS', '')},
             'routing': {key: normalize(os.environ.get(key, '')) for key in
                         ('PATH', 'CREXX_HOME', 'CREXX_PROVIDER_PATH', 'CREXX_LIBRARY_PATH',
                          'LD_LIBRARY_PATH', 'DYLD_LIBRARY_PATH', 'CC', 'CXX', 'CFLAGS', 'CXXFLAGS', 'LDFLAGS')}}
    if native_only:
        for key in ('source', 'profile', 'preparation'):
            value.pop(key)
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()


def platform_profile(requested, scheduled, expected, current):
    return 'full' if requested == 'daily' and scheduled and (not expected or expected != current) else requested


def inventory(build, profile, args, output):
    tools = fixture_tools()
    _, universe = ctest_inventory(['--test-dir', str(build), '-LE', '^performance-measurement$'])
    definitions, selected = ctest_inventory(args)
    if not selected:
        raise ValueError('empty sanitizer selection')
    if profile == 'daily' and not DAILY_MOVED <= universe:
        raise ValueError('approved daily family names are missing; review the profile')
    tools.json_write(output, {'profile': profile, 'universe': sorted(universe),
                             'selected': sorted(selected), 'identity': result_identity(build),
                             'selected_definitions': definitions['tests'],
                             'reusable_input_key': canonical_input(build, profile),
                             'native_configuration_key': canonical_input(build, 'full', native_only=True)})
    print(f'{profile}: selected {len(selected)} of {len(universe)} non-measurement CTests')


def complete(build, selection, junit, output):
    tools = fixture_tools()
    before = tools.json_read(selection)
    tools.validate(build, artifacts=True)
    after_identity = result_identity(build)
    before_products = before['identity'].get('native_products', {})
    after_products = after_identity.get('native_products', {})
    before_inputs = {key: value for key, value in before['identity'].items() if key != 'native_products'}
    after_inputs = {key: value for key, value in after_identity.items() if key != 'native_products'}
    if before_inputs != after_inputs or any(after_products.get(name) != value for name, value in before_products.items()):
        raise ValueError('source/configuration/dependency/preparation/native inputs changed during qualification')
    # CTest may legitimately build additional native targets. Full scope audits
    # them after tests; record their observed identities with the final result.

    root = ET.parse(junit).getroot()
    cases = root.findall('.//testcase')
    names = [case.attrib['name'] for case in cases]
    if len(names) != len(set(names)) or set(names) != set(before['selected']):
        raise ValueError('incomplete CTest execution does not match the selected names')
    if any(case.find('failure') is not None or case.find('error') is not None or
           case.find('skipped') is not None for case in cases):
        raise ValueError('failed or unexecuted tests cannot qualify sanitizer assurance')
    full = set(before['selected']) == set(before['universe'])
    daily = set(before['selected']) == set(before['universe']) - DAILY_MOVED
    own = before['identity']['preparation'] == 'original-producers'
    config = before['identity']['configuration']
    mode = config.get('CMAKE_BUILD_TYPE', '').upper()
    asan = '-fsanitize=address' in ' '.join(config.get(key, '') for key in (
        'CMAKE_C_FLAGS', 'CMAKE_CXX_FLAGS', 'CMAKE_C_FLAGS_' + mode, 'CMAKE_CXX_FLAGS_' + mode))
    execution = before['identity']['execution']
    options = execution['ASAN_OPTIONS'] + ':' + execution['LSAN_OPTIONS']
    supported = (asan and execution['CREXX_QA_PHASE'] == 'full' and
                 not any(value in options for value in ('halt_on_error=0', 'halt_on_error=false', 'exitcode=0', 'suppressions=', 'leak_check_at_exit=0', 'leak_check_at_exit=false')) and
                 (config.get('CMAKE_SYSTEM_NAME') == 'Darwin' or
                  (config.get('CMAKE_SYSTEM_NAME') == 'Linux' and 'detect_leaks=1' in execution['ASAN_OPTIONS'] and
                   not any(value in execution['LSAN_OPTIONS'] for value in ('detect_leaks=0', 'detect_leaks=false')) and
                   execution['CREXX_QA_BUILD_LEAKS'] == 'on')))
    scope = ('broad' if full and own and supported and before['profile'] in BROAD_PROFILES else
             'daily' if daily and supported and before['profile'] == 'daily' else 'focused')
    key = hashlib.sha256(json.dumps({'profile': before['profile'], 'identity': after_identity,
                                    'definitions': before.get('selected_definitions', [])}, sort_keys=True).encode()).hexdigest()
    tools.json_write(output, {**before, 'identity': after_identity, 'scope': scope, 'complete': True,
                             'executed': len(names), 'qualified_input_identity': key})
    print(f'Qualified {scope} scope: {len(names)} executed; preparation={before["identity"]["preparation"]}')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('command', choices=('exclude', 'decide', 'inventory', 'complete', 'key', 'native-key'))
    p.add_argument('--profile', choices=PROFILES, default='full')
    p.add_argument('--build-dir', type=Path)
    p.add_argument('--output', type=Path)
    p.add_argument('--selection', type=Path)
    p.add_argument('--junit', type=Path)
    p.add_argument('--leaks', choices=('on', 'off'), default='on')
    p.add_argument('--build-leaks', choices=('on', 'off'), default='on')
    p.add_argument('--event', choices=('schedule', 'manual', 'tag'), default='manual')
    p.add_argument('--activated', choices=('on', 'off'), default='off')
    p.add_argument('--weekday', type=int, choices=range(1, 8), default=1)
    p.add_argument('--source', type=Path, default=Path(__file__).resolve().parents[1])
    p.add_argument('--base', default='')
    p.add_argument('--head', default='HEAD')
    a, extra = p.parse_known_args()
    if extra and extra[0] == '--':
        extra.pop(0)
    if a.command in ('key', 'native-key'):
        options = ':'.join(part for part in os.environ.get('ASAN_OPTIONS', '').split(':')
                           if part and not part.startswith('detect_leaks='))
        execution = {'ASAN_OPTIONS': 'detect_leaks=' + ('1' if a.leaks == 'on' else '0') + (':' + options if options else ''),
                     'LSAN_OPTIONS': os.environ.get('LSAN_OPTIONS', ''),
                     'CREXX_QA_BUILD_LEAKS': a.build_leaks, 'CREXX_QA_PHASE': 'full'}
        print(canonical_input(a.build_dir, a.profile, execution, native_only=a.command == 'native-key'))
    elif a.command == 'exclude':
        print(excluded(a.profile))
    elif a.command == 'decide':
        print(json.dumps(decide(a.event, a.activated == 'on', a.weekday,
                               changed_files(a.source, a.base, a.head), a.profile)))
    elif a.command == 'inventory':
        inventory(a.build_dir, a.profile, extra, a.output)
    else:
        complete(a.build_dir, a.selection, a.junit, a.output)


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError, ET.ParseError) as error:
        print(f'sanitizer-policy: {error}', file=sys.stderr)
        raise SystemExit(1)
