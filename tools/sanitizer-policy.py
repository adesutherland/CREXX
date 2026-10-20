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

def weekly_groups():
    # A reviewed snapshot of exact names, never a live cwd/label/name heuristic.
    value = json.loads(Path(__file__).with_name('sanitizer-weekly-tests.json').read_text())
    if value.get('schema') != 1:
        raise ValueError('unsupported sanitizer membership policy')
    raw = value['weekly_only']
    if not isinstance(raw, dict) or any(not isinstance(tests, list) for tests in raw.values()):
        raise ValueError('invalid weekly sanitizer groups')
    flat = [test for tests in raw.values() for test in tests]
    if any(not isinstance(test, str) or not test for test in flat) or len(flat) != len(set(flat)):
        raise ValueError('duplicate/invalid weekly sanitizer name')
    return {name: frozenset(tests) for name, tests in raw.items()}


WEEKLY_GROUPS = weekly_groups()
DAILY_MOVED = frozenset(test for tests in WEEKLY_GROUPS.values() for test in tests)


def parse_affected(value):
    value = json.loads(value or '[]')
    if not isinstance(value, list) or any(not isinstance(name, str) for name in value):
        raise ValueError('affected sanitizer selection must be a list of exact names')
    if len(value) != len(set(value)) or not set(value) <= DAILY_MOVED:
        raise ValueError('affected selection contains a duplicate or unaudited weekly name')
    return frozenset(value)


def affected_tests():
    return parse_affected(os.environ.get('CREXX_QA_AFFECTED_TESTS', '[]'))


BROAD_PROFILES = {'full', 'weekly', 'native', 'affected'}
PROFILES = sorted(BROAD_PROFILES | {'daily'})


def fixture_tools():
    spec = importlib.util.spec_from_file_location('qa_fixtures', Path(__file__).with_name('sanitizer-fixtures.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def excluded(profile, build=None):
    if profile != 'daily':
        return ''
    names = set(DAILY_MOVED - affected_tests())
    if build is not None:
        names.difference_update(profile_names(build, profile))
    # CMake's regex engine has a compiled-size limit and only nine capture
    # groups. Factor only exact admitted mode pairs: every generated leaf is
    # still one reviewed name, with no wildcard/default exclusion of new names.
    pairs = {name[:-6] for name in names
             if name.endswith('_noopt') and name[:-6] + '_opt' in names}
    rest = names - {base + suffix for base in pairs for suffix in ('_noopt', '_opt')}
    branches = list(map(re.escape, sorted(rest)))
    if pairs:
        branches.append('(' + '|'.join(map(re.escape, sorted(pairs))) + ')_(noopt|opt)')
    return '^(' + '|'.join(branches) + ')$'


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
    # Native/build/control changes retain conservative original-producer broad
    # fallback. Source language ownership alone does not force that campaign.
    known_roots = ('lib/', 'langtools/', 'research/level-l/', 'compiler/tests/rexx_src/',
                   'tests/', 'examples/', 'docs/')
    restore = set()
    for name in changed:
        if Path(name).name in {'CMakeLists.txt', 'AGENTS.md'}:
            return {'profile': 'full', 'reason': f'build/policy control changed: {name}'}
        suffix = Path(name).suffix.lower()
        allowed = suffix in {'.crexx', '.rexx', '.expected', '.rxas', '.levell', '.rxl', '.md', '.tex'} or name.endswith(('.crexx.in', '.rexx.in'))
        allowed = allowed or (suffix == '.txt' and '/unicode/data/' in name)
        if (not name.startswith(known_roots) and name != 'README.md') or not allowed:
            return {'profile': 'full', 'reason': f'native/shared/uncertain affected scope: {name}'}
        # Closed producer SAN trigger edits retain original instrumented scope.
        if any(trigger in name for trigger in ('san001', 'san003', 'san004', 'san006', 'san007', 'san008',
                                               'san010', 'san011', 'san012', 'binary_global_import_types',
                                               'frame_control_ast', 'compiler_contract_builder')):
            return {'profile': 'full', 'reason': f'producer SAN trigger changed: {name}'}
        if suffix in {'.md', '.tex'}:
            continue
        # These named contracts alter the retained panel's native allocation or
        # request/graph histories. Other source-only edits use ordinary full
        # semantics plus the fixed daily panel, not every Level L permutation.
        unicode_owner = (name.startswith(('lib/rxfnsg/unicode/', 'lib/rxfnsl/unicode/')) or
                         name.startswith('lib/rxfnsg/rexx/unicode') or
                         name.startswith('lib/rxfnsb/rexx/rxunicode'))
        if unicode_owner:
            restore.update(WEEKLY_GROUPS['unicode-runtime-matrix'])
        if name in {'lib/rxfnsl/rexx/llir.crexx', 'lib/rxfnsl/rexx/llcfg.crexx',
                    'lib/rxfnsl/rexx/llflow.crexx', 'langtools/rexx/llfront.crexx'}:
            # Shared graph/front-end storage feeds all configured backends.
            restore.update(WEEKLY_GROUPS['level-l-workload-matrix'])
        if name in {'langtools/rexx/llgenerate.crexx', 'langtools/tests/services.crexx'}:
            restore.update(test for test in WEEKLY_GROUPS['level-l-workload-matrix']
                           if test.startswith('levell_services_'))
        if name.startswith('langtools/tests/'):
            stem = Path(name).stem
            prefix = 'ast_' if stem == 'ast-probe' else stem + '_'
            restore.update(test for test in WEEKLY_GROUPS['level-l-workload-matrix']
                           if test.startswith(prefix))
        if name.startswith('compiler/tests/rexx_src/'):
            stem = Path(name).stem
            restore.update(test for test in DAILY_MOVED
                           if test == stem or test.startswith(stem + '_'))
    return {'profile': 'daily', 'affected_tests': sorted(restore),
            'reason': 'VM-priority daily scope plus exact affected source-family names; new tests default keep'}


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
            'ImageVersion', 'ImageOS', 'RUNNER_ARCH', 'RUNNER_OS', 'CREXX_QA_AFFECTED_TESTS')},
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
    execution = dict(execution or {key: os.environ.get(key, '') for key in
                              ('ASAN_OPTIONS', 'LSAN_OPTIONS', 'CREXX_QA_BUILD_LEAKS', 'CREXX_QA_PHASE', 'CREXX_QA_AFFECTED_TESTS')})
    execution['CREXX_QA_AFFECTED_TESTS'] = json.dumps(sorted(parse_affected(execution.get('CREXX_QA_AFFECTED_TESTS', '[]'))))
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
    if native_only or profile in BROAD_PROFILES:
        value['execution'].pop('CREXX_QA_AFFECTED_TESTS', None)
    if native_only:
        for key in ('source', 'profile', 'preparation'):
            value.pop(key)
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()


def platform_profile(requested, scheduled, expected, current):
    return 'full' if requested == 'daily' and scheduled and (not expected or expected != current) else requested


def profile_names(build, profile, definitions=None):
    if definitions is None:
        definitions, _ = ctest_inventory(['--test-dir', str(build), '-LE', '^performance-measurement$'])
    tests = {test['name']: test for test in definitions['tests']}
    kept = set(tests) if profile != 'daily' else set(tests) - (DAILY_MOVED - affected_tests())
    # Explicit -E also suppresses setup tests in CTest. Preserve every fixture
    # owner needed by the default-kept or restored consumers, including future
    # consumers of an existing weekly fixture. This only adds prerequisites.
    properties = {name: {item['name']: item['value'] for item in test['properties']}
                  for name, test in tests.items()}
    while True:
        required = {fixture for name in kept
                    for fixture in properties[name].get('FIXTURES_REQUIRED', [])}
        owners = {name for name, props in properties.items()
                  if required.intersection(props.get('FIXTURES_SETUP', []) + props.get('FIXTURES_CLEANUP', []))}
        if owners <= kept:
            return kept
        kept.update(owners)


def preparation_plan(build, profile):
    """Selected fixture/native target graph; an unknown wrapper fails closed."""
    if profile != "daily":
        raise ValueError("selective preparation is reserved for daily runtime scope")
    build = Path(build).resolve()
    definitions, _ = ctest_inventory(['--test-dir', str(build), '-LE', '^performance-measurement$'])
    selected = profile_names(build, profile, definitions)
    targets = {}
    for line in (build / 'qa-build-targets.tsv').read_text().splitlines():
        name, kind, path = line.split('\t')
        if name in targets:
            raise ValueError('duplicate configured build target: ' + name)
        targets[name] = kind
    required = set()
    missing = []
    for test in definitions['tests']:
        name = test['name']
        if name not in selected:
            continue
        properties = {item['name']: item['value'] for item in test['properties']}
        declared = properties.get('CREXX_DAILY_PREP_TARGETS', properties.get('CREXX_PREP_TARGETS', []))
        if isinstance(declared, str):
            declared = declared.split(';') if declared else []
        # CTest can omit command[] until an executable exists. Ownership must
        # be declarative in both cold and warm trees; paths are not a fallback.
        if not declared and properties.get('CREXX_PREP_NONE') not in (True, 1, 'TRUE', 'ON', '1'):
            missing.append(name)
        required.update(declared)
    if missing:
        raise ValueError('selected tests have no declared prerequisite owner: ' + ', '.join(sorted(missing)))
    # All runtime autoload/native provider packages remain daily. Their native
    # objects and implicit provider paths are not inferable from command[0].
    required.update(name for name, kind in targets.items()
                    if kind in ('MODULE_LIBRARY', 'SHARED_LIBRARY'))
    required.update(('rxc', 'rxas', 'rxlink', 'rxvm', 'rxbvm', 'rxtvm', 'crexx',
                     'library', 'classlib', 'rxfnsc', 'rxfnsg'))
    unknown = required - targets.keys()
    if unknown or required.intersection(('all', 'ALL', 'qa-prep', 'qa-prep-all')):
        raise ValueError('invalid selected prerequisite targets: ' + ', '.join(sorted(unknown)))
    return {'profile': profile, 'selected': sorted(selected), 'targets': sorted(required),
            'configuration': fixture_tools().identity(build)}


def inventory(build, profile, args, output, preparation=None):
    tools = fixture_tools()
    universe_definitions, universe = ctest_inventory(['--test-dir', str(build), '-LE', '^performance-measurement$'])
    definitions, selected = ctest_inventory(args)
    if not selected:
        raise ValueError('empty sanitizer selection')
    prepared = None
    if preparation is not None:
        prepared = tools.json_read(preparation)
        if profile != 'daily' or prepared != preparation_plan(build, profile):
            raise ValueError('selected prerequisite build does not match current daily inputs')
    # Platform-absent known names are legitimate; newly registered names remain
    # selected. Never require the Apple reference inventory on another platform.
    tools.json_write(output, {'profile': profile, 'universe': sorted(universe),
                             'selected': sorted(selected), 'profile_selected': sorted(profile_names(build, profile, universe_definitions)), 'affected_tests': sorted(affected_tests()) if profile == 'daily' else [], 'identity': result_identity(build),
                             'selected_definitions': definitions['tests'], 'daily_preparation': prepared,
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
        raise ValueError('source/configuration/dependency/preparation/native inputs changed during qualification; '
                         + tools.identity_mismatch_details(build, before['identity'], after_identity, include_docs=True))
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
    # CTest PASS_REGULAR_EXPRESSION can admit a nonzero child exit after its
    # semantic success text. A late process-exit LSan report must still block
    # assurance, including reports in otherwise passing test output.
    diagnostic = re.compile(r'(?:^|\n)\s*(?:==[0-9]+==)?(?:ERROR: (?:AddressSanitizer|LeakSanitizer):|SUMMARY: (?:AddressSanitizer|LeakSanitizer):|AddressSanitizer:DEADLYSIGNAL|AddressSanitizer: CHECK failed|LeakSanitizer has encountered a fatal error)', re.MULTILINE)
    for case in cases:
        if diagnostic.search('\n'.join(case.itertext())):
            raise ValueError('sanitizer diagnostic in CTest output: ' + case.attrib['name'])
    full = set(before['selected']) == set(before['universe'])
    restored = set(before.get('affected_tests', []))
    declared = parse_affected(before['identity']['execution'].get('CREXX_QA_AFFECTED_TESTS', '[]'))
    if before['profile'] == 'daily' and (not restored <= DAILY_MOVED or restored != declared):
        raise ValueError('affected daily names are not bound to the execution policy')
    expected_daily = profile_names(build, 'daily') if before['profile'] == 'daily' else set()
    if before['profile'] == 'daily' and set(before.get('profile_selected', [])) != expected_daily:
        raise ValueError('canonical daily fixture scope changed during qualification')
    daily = set(before['selected']) == expected_daily
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
    if daily and supported and before['profile'] == 'daily':
        if before.get('daily_preparation') != preparation_plan(build, 'daily'):
            raise ValueError('daily assurance requires the exact completed selected prerequisite build')
    scope = ('broad' if full and own and supported and before['profile'] in BROAD_PROFILES else
             'daily' if daily and supported and before['profile'] == 'daily' else 'focused')
    key = hashlib.sha256(json.dumps({'profile': before['profile'], 'identity': after_identity,
                                    'definitions': before.get('selected_definitions', [])}, sort_keys=True).encode()).hexdigest()
    tools.json_write(output, {**before, 'identity': after_identity, 'scope': scope, 'complete': True,
                             'executed': len(names), 'qualified_input_identity': key})
    print(f'Qualified {scope} scope: {len(names)} executed; preparation={before["identity"]["preparation"]}')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('command', choices=('exclude', 'decide', 'inventory', 'complete', 'key', 'native-key', 'prep'))
    p.add_argument('--profile', choices=PROFILES, default='full')
    p.add_argument('--build-dir', type=Path)
    p.add_argument('--output', type=Path)
    p.add_argument('--selection', type=Path)
    p.add_argument('--verify-plan', type=Path)
    p.add_argument('--preparation-plan', type=Path)
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
                     'CREXX_QA_BUILD_LEAKS': a.build_leaks, 'CREXX_QA_PHASE': 'full',
                     'CREXX_QA_AFFECTED_TESTS': json.dumps(sorted(affected_tests()))}
        print(canonical_input(a.build_dir, a.profile, execution, native_only=a.command == 'native-key'))
    elif a.command == 'exclude':
        print(excluded(a.profile, a.build_dir))
    elif a.command == 'decide':
        print(json.dumps(decide(a.event, a.activated == 'on', a.weekday,
                               changed_files(a.source, a.base, a.head), a.profile)))
    elif a.command == 'prep':
        value = preparation_plan(a.build_dir, a.profile)
        if a.verify_plan and value != fixture_tools().json_read(a.verify_plan):
            raise ValueError('selected preparation inputs changed during build')
        fixture_tools().json_write(a.output, value)
        print('Selected prerequisites: ' + str(len(value['targets'])) + ' targets for ' + str(len(value['selected'])) + ' tests')
    elif a.command == 'inventory':
        inventory(a.build_dir, a.profile, extra, a.output, a.preparation_plan)
    else:
        complete(a.build_dir, a.selection, a.junit, a.output)


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError, ET.ParseError) as error:
        print(f'sanitizer-policy: {error}', file=sys.stderr)
        raise SystemExit(1)
