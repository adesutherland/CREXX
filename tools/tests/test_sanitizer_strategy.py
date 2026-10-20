"""Behavioral policy, CMake argument, owner-copy and shared-cache contracts."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET
from unittest.mock import patch

ROOT = Path(os.environ.get('CREXX_TEST_ROOT', Path(__file__).resolve().parents[2]))
def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value
F = module('fixtures', ROOT / 'tools/sanitizer-fixtures.py')
P = module('policy', ROOT / 'tools/sanitizer-policy.py')
C = module('contracts', ROOT / 'compiler/tests/contract_fixture.py')


def command(*argv, success=True):
    result = subprocess.run(list(map(str, argv)), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if (result.returncode == 0) != success:
        raise AssertionError(result.stdout[-12000:])
    return result


class Strategy(unittest.TestCase):
    def setUp(self):
        # Default-policy assertions are independent of the enclosing affected
        # workflow; individual cases explicitly exercise nonempty restoration.
        environment = patch.dict(os.environ, {'CREXX_QA_AFFECTED_TESTS':'[]'})
        environment.start()
        self.addCleanup(environment.stop)
        self.temporary = tempfile.TemporaryDirectory(prefix='crexx-strategy-')
        self.root = Path(self.temporary.name)
        self.addCleanup(self.temporary.cleanup)

    def test_cadence_and_exact_default_keep(self):
        self.assertEqual(len(P.DAILY_MOVED), 2010)
        with patch.object(Path, 'read_text', return_value=json.dumps({'schema':1,'weekly_only':{'one':['duplicate','duplicate']}})):
            with self.assertRaisesRegex(ValueError, 'duplicate'):
                P.weekly_groups()
        for name in P.DAILY_MOVED:
            self.assertRegex(name, P.excluded('daily'))
        for name in ('future_native_contract', 'arg_semantics_scalar_run_opt',
                     'ts_unicode_normalization_rxtvm_opt', 'san002_wide_decimal_extract_rxbvm_opt',
                     'llist_test_noopt', 'rxvm_memory_contract'):
            self.assertNotRegex(name, P.excluded('daily'))
        for profile in P.BROAD_PROFILES:
            self.assertEqual(P.excluded(profile), '')
        decide = lambda **kw: P.decide(kw.get('event', 'schedule'), kw.get('active', True),
                                       kw.get('day', 2), kw.get('changes', []), kw.get('requested', 'full'))['profile']
        self.assertEqual(decide(active=False), 'full')
        self.assertEqual(decide(day=7), 'weekly')
        self.assertEqual(decide(changes=None), 'full')
        self.assertEqual(decide(changes=['lib/example.crexx']), 'daily')
        self.assertEqual(decide(event='manual'), 'full')
        self.assertEqual(decide(event='manual', requested='daily'), 'daily')
        self.assertEqual(decide(event='tag', requested='daily'), 'full')
        self.assertEqual(P.platform_profile('daily', True, '', 'new'), 'full')
        self.assertEqual(P.platform_profile('daily', True, 'old', 'new'), 'full')
        self.assertEqual(P.platform_profile('daily', True, 'same', 'same'), 'daily')
        self.assertEqual(P.platform_profile('daily', False, '', 'new'), 'daily')
        for name in ('lib/CMakeLists.txt', 'lib/plugins/vector/CMakeLists.txt',
                     'tests/rxpa/CMakeLists.txt', 'compiler/rxcp.c', 'new/control.unknown',
                     'compiler/tests/rexx_src/san003_inline_scoped_array_actual.crexx', 'cmake/new.cmake'):
            self.assertEqual(decide(changes=[name]), 'full', name)

    def test_affected_source_ownership_and_platform_absence(self):
        def decision(name):
            value = P.decide('schedule', True, 2, [name])
            self.assertEqual(value['profile'], 'daily', name)
            return set(value['affected_tests'])
        for name in ('langtools/rexx/some_source.crexx', 'lib/rxfnsl/rexx/llast.crexx',
                     'research/level-l/compilers/src/compiler.crexx', 'lib/rxfnsg/rexx/general.crexx'):
            self.assertEqual(decision(name), set(), name)
        for name in ('lib/rxfnsl/unicode/algorithm.crexx', 'lib/rxfnsg/unicode/data/UnicodeData.txt',
                     'lib/rxfnsg/rexx/unicode_case_mapping.crexx.in'):
            self.assertEqual(decision(name), P.WEEKLY_GROUPS['unicode-runtime-matrix'])
        self.assertEqual(decision('lib/rxfnsl/rexx/llir.crexx'), P.WEEKLY_GROUPS['level-l-workload-matrix'])
        self.assertEqual(decision('langtools/rexx/llgenerate.crexx'),
                         {'levell_services_noopt_rxbvm', 'levell_services_noopt_rxtvm'})
        structured=decision('langtools/tests/ts_llstructuredflow.crexx')
        self.assertEqual(len(structured), 14)
        self.assertTrue(all(name.startswith('ts_llstructuredflow_') for name in structured))
        self.assertEqual(len(decision('compiler/tests/rexx_src/levelc_bif_reference_audit.expected')), 5)  # four owners + shared setup
        # A platform can omit known reference names; new tests remain default keep.
        universe={'future_native_contract','binary_global_import_types','arg_semantics_scalar_run_opt'}
        self.assertEqual(universe-P.DAILY_MOVED, {'future_native_contract','arg_semantics_scalar_run_opt'})
        restored='binary_global_import_types'
        with patch.dict(os.environ, {'CREXX_QA_AFFECTED_TESTS':json.dumps([restored])}):
            self.assertNotRegex(restored, P.excluded('daily'))
            self.assertRegex('arg_semantics_scalar_opt', P.excluded('daily'))
        with patch.dict(os.environ, {'CREXX_QA_AFFECTED_TESTS':'["unknown"]'}):
            with self.assertRaisesRegex(ValueError, 'unaudited'):
                P.excluded('daily')

    @unittest.skipUnless(shutil.which('ninja'), 'Independent CTest fixture requires Ninja')
    def test_real_ctest_daily_fixture_closure(self):
        source=self.root/'fixture-source'; source.mkdir()
        (source/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.24)
project(DailyFixture NONE)
enable_testing()
add_test(NAME levelc_bif_reference_audit_prepare COMMAND "${CMAKE_COMMAND}" -E true)
set_tests_properties(levelc_bif_reference_audit_prepare PROPERTIES FIXTURES_SETUP required-owner)
add_test(NAME future_native_contract COMMAND "${CMAKE_COMMAND}" -E true)
set_tests_properties(future_native_contract PROPERTIES FIXTURES_REQUIRED required-owner)
add_test(NAME arg_semantics_scalar_opt COMMAND "${CMAKE_COMMAND}" -E true)
add_test(NAME arg_semantics_scalar_opt_future COMMAND "${CMAKE_COMMAND}" -E true)
''')
        build=self.root/'fixture-build'
        command('cmake','-S',source,'-B',build,'-G','Ninja')
        pattern=P.excluded('daily',build)
        # Keep the full expression out of CreateProcess's Windows argument
        # limit. CTest's in-process script handler still compiles/applies it.
        pattern_file=self.root/'exclude.txt'; pattern_file.write_text(pattern)
        script=self.root/'select.cmake'; junit=self.root/'selected.xml'
        script.write_text('''set(CTEST_SOURCE_DIRECTORY "@SOURCE@")
set(CTEST_BINARY_DIRECTORY "@BUILD@")
set(CTEST_CMAKE_GENERATOR "Ninja")
ctest_start(Experimental)
file(READ "@PATTERN@" pattern)
ctest_test(EXCLUDE "${pattern}" OUTPUT_JUNIT "@JUNIT@" RETURN_VALUE status)
if(NOT status EQUAL 0)
 message(FATAL_ERROR "Miniature selected tests failed")
endif()
'''.replace('@SOURCE@',source.as_posix()).replace('@BUILD@',build.as_posix()).replace('@PATTERN@',pattern_file.as_posix()).replace('@JUNIT@',junit.as_posix()))
        command('ctest','-S',script,'--output-on-failure')
        actual={case.attrib['name'] for case in ET.parse(junit).getroot().findall('.//testcase')}
        expected={'future_native_contract','levelc_bif_reference_audit_prepare','arg_semantics_scalar_opt_future'}
        self.assertEqual(actual,expected)
        self.assertEqual(P.profile_names(build,'daily'),expected)
        for name in P.DAILY_MOVED:
            if name!='levelc_bif_reference_audit_prepare': self.assertRegex(name,pattern)
        self.assertNotRegex('arg_semantics_scalar_opt_future',pattern)

    def mini_project(self):
        source = self.root / 'source with spaces'
        source.mkdir()
        command('git', 'init', '-q', source)
        (source / 'emit.py').write_text('import json,sys\nfrom pathlib import Path\nPath(sys.argv[1]).write_text(json.dumps(sys.argv[2:]))\n')
        cmake = '''cmake_minimum_required(VERSION 3.24)
project(FixtureContract C)
file(WRITE "${CMAKE_BINARY_DIR}/native.c" "int main(void) { return 0; }\n")
add_executable(contract_native "${CMAKE_BINARY_DIR}/native.c")
include(CTest)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
include("@MODULE@")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(WIN32)
 set(suffix ".exe")
endif()
foreach(tool IN ITEMS rxc rxas rxlink)
 file(WRITE "${CMAKE_BINARY_DIR}/bin/${tool}${suffix}" "ordinary producer ${tool}")
endforeach()
foreach(kind IN ITEMS original wrapped)
 set(out "${CMAKE_BINARY_DIR}/${kind}.rxbin")
 set(byproduct "${CMAKE_BINARY_DIR}/${kind}.rxas")
 set(args OUTPUT "${out}" BYPRODUCTS "${byproduct}"
   COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/emit.py" "${out}"
     "space argument" "one;two" "escaped\\;semi" "$<IF:$<BOOL:1>,yes,no>"
   COMMAND "${CMAKE_COMMAND}" -E touch "${byproduct}" VERBATIM)
 # The explicit calls intentionally exercise quoted argument boundaries.
 if(kind STREQUAL "wrapped")
  crexx_add_fixture_command(OUTPUT "${out}" BYPRODUCTS "${byproduct}"
   COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/emit.py" "${out}"
     "space argument" "one;two" "escaped\\;semi" "$<IF:$<BOOL:1>,yes,no>"
   COMMAND "${CMAKE_COMMAND}" -E touch "${byproduct}" VERBATIM)
 else()
  add_custom_command(OUTPUT "${out}" BYPRODUCTS "${byproduct}"
   COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/emit.py" "${out}"
     "space argument" "one;two" "escaped\\;semi" "$<IF:$<BOOL:1>,yes,no>"
   COMMAND "${CMAKE_COMMAND}" -E touch "${byproduct}" VERBATIM)
 endif()
 add_custom_target(${kind} ALL DEPENDS "${out}")
endforeach()
add_custom_target(consumer ALL COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_BINARY_DIR}/consumer.txt" DEPENDS wrapped)
set_property(GLOBAL APPEND PROPERTY CREXX_QA_NATIVE_OUTPUTS "${CMAKE_BINARY_DIR}/$<CONFIG>/native")
crexx_finalize_qa_fixtures()
'''.replace('@MODULE@', (ROOT / 'cmake/CrexxQaFixtures.cmake').as_posix())
        (source / 'CMakeLists.txt').write_text(cmake)
        return source

    @unittest.skipUnless(shutil.which('ninja') and shutil.which('git'), 'External fixture mode requires Ninja; other contracts still run')
    def test_cmake_forwarding_and_external_owner_failure(self):
        source = self.mini_project()
        donor, native = self.root / 'ordinary', self.root / 'native'
        multi=self.root/'multi'
        command('cmake','-S',source,'-B',multi,'-G','Ninja Multi-Config')
        self.assertTrue((multi/'qa-native-files-Debug.txt').is_file())
        self.assertTrue((multi/'qa-native-files-Release.txt').is_file())
        self.assertIn('/Debug/contract_native', (multi/'qa-build-targets-Debug.tsv').read_text().replace('.exe',''))
        self.assertIn('/Release/contract_native', (multi/'qa-build-targets-Release.tsv').read_text().replace('.exe',''))
        command('cmake', '-S', source, '-B', donor, '-G', 'Ninja')
        command('cmake', '--build', donor)
        self.assertEqual(json.loads((donor/'original.rxbin').read_text()),
                         json.loads((donor/'wrapped.rxbin').read_text()))
        self.assertEqual(json.loads((donor/'wrapped.rxbin').read_text()),
                         ['space argument', 'one;two', 'escaped\\;semi', 'yes'])
        F.seal(donor)
        command('cmake', '-S', source, '-B', native, '-G', 'Ninja', f'-DCREXX_QA_FIXTURE_TREE={donor}')
        command('cmake', '--build', native)
        self.assertTrue((native/'consumer.txt').exists())
        F.validate(native, artifacts=True)
        self.assertEqual((donor/'wrapped.rxbin').read_bytes(), (native/'wrapped.rxbin').read_bytes())
        (native/'wrapped.rxas').unlink()
        with self.assertRaisesRegex(ValueError, 'owner member'):
            F.validate(native, artifacts=True)
        F.copy_files(native, ['wrapped.rxbin', 'wrapped.rxas'])
        (donor/'wrapped.rxas').write_text('partial donor')
        with self.assertRaisesRegex(ValueError, 'partial or changed'):
            F.copy_files(native, ['wrapped.rxbin', 'wrapped.rxas'])
        self.assertFalse((native/'wrapped.rxbin').exists())
        self.assertFalse((native/'wrapped.rxas').exists())
        (native/'consumer.txt').unlink()
        command('cmake', '--build', native, success=False)
        self.assertFalse((native/'consumer.txt').exists())
        (source/'emit.py').write_text('changed producer input')
        with self.assertRaisesRegex(ValueError, 'mismatch'):
            F.validate(native)

    def test_runtime_paths_reject_ordinary_vm_and_provider_routes(self):
        native=self.root/'native'; donor=self.root/'ordinary'
        for tree in (native,donor):
            (tree/'bin').mkdir(parents=True)
            (tree/'bin/rxvm').write_text('native entry')
            (tree/'image.rxbin').write_text('owned image')
        with patch.object(F,'donor',return_value=(donor,{})), patch.dict(os.environ,{'CREXX_HOME':'','CREXX_PROVIDER_PATH':''}):
            F.runtime_paths(native,native/'bin/rxvm',[native/'image.rxbin',native/'bin'])
            with self.assertRaisesRegex(ValueError,'ordinary donor'):
                F.runtime_paths(native,donor/'bin/rxvm',[native/'image.rxbin'])
            for variable in ('CREXX_HOME','CREXX_PROVIDER_PATH'):
                with patch.dict(os.environ,{variable:str(donor/'bin')}):
                    with self.assertRaisesRegex(ValueError,'ordinary donor'):
                        F.runtime_paths(native,native/'bin/rxvm',[native/'image.rxbin'])
            with self.assertRaisesRegex(ValueError,'outside its instrumented tree'):
                F.runtime_paths(native,Path(sys.executable),[native/'image.rxbin'])

    def test_selected_preparation_fails_unknown_owner_and_retains_native_children(self):
        build=self.root/'prepared'; build.mkdir()
        baseline=('rxc','rxas','rxlink','rxvm','rxbvm','rxtvm','crexx','library','classlib','rxfnsc','rxfnsg')
        registry=[name+'\tUTILITY\t' for name in baseline]
        registry.extend(['owned_host\tEXECUTABLE\t'+str(build/'host'),
                         'ordinary_image\tUTILITY\t', 'unused_weekly_image\tUTILITY\t',
                         'implicit_provider\tMODULE_LIBRARY\t'+str(build/'provider.rxplugin')])
        (build/'qa-build-targets.tsv').write_text('\n'.join(registry)+'\n')
        tests=[{'name':'host','command':[str(build/'host')], 'properties':[]},
               {'name':'wrapper','command':['cmake','-P','runtime.cmake'], 'properties':[
                   {'name':'CREXX_PREP_TARGETS','value':'unused_weekly_image'},
                   {'name':'CREXX_DAILY_PREP_TARGETS','value':'ordinary_image;rxvm'}]},
               {'name':'qa_control','command':['python','contract.py'], 'properties':[
                   {'name':'CREXX_PREP_NONE','value':True}]}]
        with patch.object(P,'ctest_inventory',return_value=({'tests':tests},set(t['name'] for t in tests))), patch.object(F,'identity',return_value={'source':'same'}), patch.object(P,'fixture_tools',return_value=F):
            value=P.preparation_plan(build,'daily')
            self.assertIn('owned_host',value['targets'])
            self.assertIn('ordinary_image',value['targets'])
            self.assertIn('implicit_provider',value['targets'])
            self.assertNotIn('unused_weekly_image',value['targets'])
            self.assertFalse(set(value['targets']) & {'all','ALL','qa-prep'})
            tests.append({'name':'future_wrapper','command':['cmake','-P','new.cmake'],'properties':[]})
            with self.assertRaisesRegex(ValueError,'future_wrapper'):P.preparation_plan(build,'daily')
            tests.pop()
            tests[2]['properties'][0]['value']='FALSE'
            with self.assertRaisesRegex(ValueError,'qa_control'):P.preparation_plan(build,'daily')
            tests[2]['properties'][0]['value']=True
            tests[1]['properties'][1]['value']='missing_fixture_target'
            with self.assertRaisesRegex(ValueError,'missing_fixture_target'):P.preparation_plan(build,'daily')

    @unittest.skipUnless(shutil.which('git'), 'Source-identity contracts require Git')
    def test_missing_history_and_test_consumed_docs(self):
        source = self.root/'src'; source.mkdir(); command('git', 'init', '-q', source)
        (source/'sample.c').write_text('a')
        (source/'docs').mkdir(); document=source/'docs'/'oracle.md'; document.write_text('one')
        ordinary, result = F.source_identity(source, False), F.source_identity(source)
        document.write_text('two')
        self.assertEqual(ordinary, F.source_identity(source, False))
        self.assertNotEqual(result, F.source_identity(source))
        self.assertIsNone(P.changed_files(source, '', 'HEAD'))
        self.assertIsNone(P.changed_files(source, 'missing', 'HEAD'))
        (source/'__pycache__').mkdir(); (source/'__pycache__/ignored.pyc').write_bytes(b'cache')
        hints = F.changed_git_paths(source, include_docs=False, limit=1)
        self.assertEqual(hints, {'count': 1, 'paths': ['sample.c'], 'truncated': False})
        (source/'second.c').write_text('second')
        hints = F.changed_git_paths(source, include_docs=False, limit=1)
        self.assertEqual(hints['count'], 2)
        self.assertTrue(hints['truncated'])

    @unittest.skipUnless(shutil.which('git'), 'Source-identity contracts require Git')
    def test_donor_actual_dependency_and_configuration_drift(self):
        source=self.root/'src'; source.mkdir(); command('git','init','-q',source)
        (source/'source.crexx').write_text('source')
        dependency=self.root/'dependency'; dependency.mkdir(); command('git','init','-q',dependency)
        (dependency/'provider.c').write_text('dependency')
        command('git','-C',dependency,'add','provider.c')
        command('git','-C',dependency,'-c','user.name=Fixture','-c','user.email=fixture@example.invalid','commit','-qm','fixture')
        donor=self.root/'ordinary'; native=self.root/'native'
        common={'CMAKE_HOME_DIRECTORY':str(source),'DSLSH_SOURCE_DIR':str(dependency),'CMAKE_BUILD_TYPE':'Debug'}
        for build in (donor,native):
            (build/'bin').mkdir(parents=True)
            for name in ('rxc','rxas','rxlink'):
                (build/'bin'/(name+('.exe' if os.name=='nt' else ''))).write_text('owned test producer')
            (build/'qa-fixture-files.txt').write_text('case.rxbin\n')
            (build/'case.rxbin').write_text('owned fixture')
        F.json_write(donor/F.CONFIG,common)
        F.json_write(native/F.CONFIG,{**common,'CREXX_QA_FIXTURE_TREE':str(donor)})
        F.seal(donor); F.validate(native)
        F.json_write(native/F.CONFIG,{**common,'CREXX_QA_FIXTURE_TREE':str(donor),'CMAKE_C_FLAGS':'-DCHANGED=1'})
        with self.assertRaisesRegex(ValueError,'configuration keys=CMAKE_C_FLAGS'): F.validate(native)
        F.json_write(native/F.CONFIG,{**common,'CREXX_QA_FIXTURE_TREE':str(donor)})
        (dependency/'provider.c').write_text('moving dependency branch changed contents')
        with self.assertRaisesRegex(ValueError,'dependency DSLSH_SOURCE_DIR fields=contents paths=.*provider.c'): F.validate(native)
        (dependency/'provider.c').write_text('dependency')
        (source/'source.crexx').write_text('changed source')
        with self.assertRaisesRegex(ValueError,'components=source; source paths=.*source.crexx'): F.validate(native)

    @unittest.skipUnless(sys.platform in ('linux','darwin') and shutil.which('cc'), 'Maintained native closure hosts require a C compiler')
    def test_native_provider_own_flags_and_product_entrypoint(self):
        build=self.root/'native'; (build/'bin').mkdir(parents=True)
        source=build/'native.c'; source.write_text('int main(void){return 0;}\n')
        obj=build/'native.o'; executable=build/'bin'/'rxbvm'
        cc=shutil.which('cc')
        args=[cc,'-fsanitize=address','-c',str(source),'-o',str(obj)]
        for invocation in (args,[cc,'-fsanitize=address',str(obj),'-o',str(executable)]):
            probe=subprocess.run(invocation,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
            if probe.returncode:
                if os.environ.get('CREXX_QA_REQUIRE_ASAN')=='TRUE':
                    self.fail(probe.stdout[-12000:])
                self.skipTest('Host compiler lacks the ASan facility required for this control fixture')
        for name in ('crexx_native_cflags','crexx_native_ldflags'):
            (build/'bin'/name).write_text('-fsanitize=address')
        products=[]
        for name in ('rxc','rxas','rxlink','rxvm','rxbvm','rxtvm','crexx'):
            target=build/'bin'/name
            if target!=executable: shutil.copyfile(executable,target)
            products.append(str(target))
        F.json_write(build/F.CONFIG,{'CMAKE_BUILD_TYPE':'Debug','CMAKE_C_FLAGS':'-fsanitize=address',
                                    'CMAKE_SYSTEM_NAME':'Darwin' if sys.platform=='darwin' else 'Linux'})
        F.json_write(build/'compile_commands.json',[{'file':str(source),'directory':str(build),'arguments':args}])
        (build/'qa-native-files.txt').write_text('\n'.join(products))
        child=build/'child-program'; shutil.copyfile(executable,child)
        self.assertIn(child,F.owned_native_files(build))
        self.assertNotIn(obj,F.owned_native_files(build))
        F.native_closure(build)
        nested=build/'sdk'; nested.mkdir()
        F.json_write(nested/'compile_commands.json',[{'file':str(nested/'provider.c'),'directory':str(build),
                     'arguments':[cc,'-c',str(source),'-o',str(obj)]}])
        with self.assertRaisesRegex(ValueError,'source lacks effective ASan'):
            F.native_closure(build)
        (nested/'compile_commands.json').unlink()
        (build/'bin/rxvm').write_text('wrong ordinary product entrypoint')
        with self.assertRaisesRegex(ValueError,'product rxvm'):
            F.native_closure(build)

    def test_shared_cache_identity_unowned_link_and_failure(self):
        bindir=self.root/'bin'; bindir.mkdir()
        for name in ('rxc', 'rxas', 'rxlink', 'rxvm', 'library.rxbin', 'classlib.rxbin', 'rxfnsc.rxbin'):
            suffix='.exe' if os.name=='nt' and '.' not in name else ''
            (bindir/(name+suffix)).write_text(name)
        contract=self.root/'contract.py'; contract.write_text('oracle')
        cache=self.root/'cache'; calls=[]
        def native(argv, cwd, **kwargs):
            name=Path(argv[0]).stem; calls.append(name)
            if name=='rxc':
                if kwargs.get('expected')==2: return b'RXC-LC-01.1'
                Path(str(argv[argv.index('-o')+1])+'.rxas').write_text('assembly')
            elif name in ('rxas', 'rxlink'):
                Path(argv[argv.index('-o')+1]).write_text('linked' if name=='rxlink' else 'bytecode')
            return b'okay\r\n' if name=='rxvm' else b''
        def exercise(prepare=False, linked=False):
            with C.ContractFixture(bindir, cache, [contract], prepare=prepare) as value:
                value.case('sample', 'source', expected='okay\n', linked=linked)
                value.invalid('bad', 'bad input', '01.1')
        with patch.object(C, 'run', side_effect=native):
            exercise(prepare=True)
            self.assertEqual(calls.count('rxc'), 3)
            self.assertEqual(calls.count('rxas'), 2)
            self.assertNotIn('rxlink', calls)
            image=cache/'prepared/opt/sample/program_image.rxbin'
            image.write_text('unowned interrupted image')
            exercise(linked=True)
            self.assertEqual(image.read_text(), 'linked')
            self.assertEqual(calls.count('rxlink'), 1)
            exercise(linked=True)
            self.assertEqual(calls.count('rxlink'), 1)
            image.unlink()
            exercise(prepare=True)
            with patch.object(C.ContractFixture, 'publish_manifest', side_effect=OSError('interrupted')):
                with self.assertRaises(OSError): exercise(linked=True)
            self.assertFalse(image.exists())
            exercise(prepare=True)
            exercise(linked=True)
            (bindir/('rxc.exe' if os.name=='nt' else 'rxc')).write_text('changed native producer')
            with self.assertRaisesRegex(AssertionError, 'Missing/stale'): exercise()

    def test_interprocess_preparation_lock(self):
        script=self.root/'lock.py'
        script.write_text("""import importlib.util, pathlib, sys, time
spec=importlib.util.spec_from_file_location('contract',sys.argv[1]); m=importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
with m.locked(pathlib.Path(sys.argv[2])):
 with open(sys.argv[3],'a') as f: f.write('enter\\n'); f.flush()
 time.sleep(.15)
 with open(sys.argv[3],'a') as f: f.write('exit\\n'); f.flush()
""")
        argv=[sys.executable, str(script), str(ROOT/'compiler/tests/contract_fixture.py'),
              str(self.root/'lock'), str(self.root/'events')]
        processes=[subprocess.Popen(argv, stdout=subprocess.PIPE, stderr=subprocess.PIPE) for _ in range(2)]
        for process in processes:
            out,err=process.communicate(timeout=15)
            self.assertEqual(process.returncode, 0, err.decode())
        self.assertEqual((self.root/'events').read_text().splitlines(), ['enter','exit','enter','exit'])

    @unittest.skipUnless(shutil.which('git'), 'Source-identity contracts require Git')
    def test_reuse_key_relocation_history_metadata_and_boundaries(self):
        roots=[]
        for index in (1, 2):
            source=self.root/f'source{index}'; source.mkdir(); command('git', 'init', '-q', source)
            (source/'contract.c').write_text('same source')
            build=self.root/f'build{index}'; build.mkdir()
            F.json_write(build/F.CONFIG, {'CMAKE_HOME_DIRECTORY':str(source), 'CMAKE_BUILD_TYPE':'Debug',
                'CMAKE_SYSTEM_NAME':'Linux', 'CMAKE_C_FLAGS':'-fsanitize=address',
                'CMAKE_C_COMPILER':sys.executable, 'CREXX_BUILD_TIMESTAMP':str(index),
                'CREXX_BUILD_COMMIT':str(index)*40, 'CREXX_BUILD_DIRTY':str(index),
                'CREXX_VERSION':'1.0+local.g'+str(index), 'CREXX_VERSION_CORE':'1.0',
                'CREXX_VERSION_OVERRIDE':'', 'CREXX_QA_FIXTURE_TREE':'',
                'CREXX_VERSION_FILE_PATH':str(build/'VERSION')})
            roots.append(build)
        environment={'ASAN_OPTIONS':'detect_leaks=1','LSAN_OPTIONS':'',
                     'CREXX_QA_BUILD_LEAKS':'on','CREXX_QA_PHASE':'full'}
        a,b=roots
        key=P.canonical_input(a, 'full', environment)
        self.assertEqual(key, P.canonical_input(b, 'full', environment))
        self.assertEqual(key, P.canonical_input(b, 'weekly', environment))
        self.assertNotEqual(key, P.canonical_input(b, 'daily', environment))
        config=F.json_read(b/F.CONFIG)
        for field,value in [('CMAKE_C_FLAGS','-fsanitize=address -DCHANGED=1'),
                            ('CREXX_VERSION_OVERRIDE','2.0'), ('CREXX_QA_FIXTURE_TREE',str(self.root/'donor'))]:
            F.json_write(b/F.CONFIG, {**config,field:value})
            self.assertNotEqual(key, P.canonical_input(b, 'full', environment), field)
        F.json_write(b/F.CONFIG, config)
        with patch.object(P.platform, 'machine', return_value='other-architecture'):
            self.assertNotEqual(key, P.canonical_input(b, 'full', environment))
        self.assertNotEqual(key, P.canonical_input(b, 'full', {**environment,'ASAN_OPTIONS':'detect_leaks=0'}))
        restored=['binary_global_import_types','ts_unicode_normalization_rxbvm_noopt']
        pretty={**environment,'CREXX_QA_AFFECTED_TESTS':json.dumps(list(reversed(restored)),indent=2)}
        with patch.dict(os.environ,pretty):
            direct=P.canonical_input(a,'daily')
            self.assertEqual(direct,P.canonical_input(a,'daily',{**environment,'CREXX_QA_AFFECTED_TESTS':json.dumps(restored)}))
            cli=command(sys.executable,ROOT/'tools/sanitizer-policy.py','key','--build-dir',a,'--profile','daily').stdout.strip()
            self.assertEqual(cli,direct)
            output=self.root/'inventory.json'
            with patch.object(P,'ctest_inventory',return_value=({'tests':[{'name':'future_test','properties':[]}]},{'future_test'})):
                P.inventory(a,'daily',[],output)
            self.assertEqual(F.json_read(output)['reusable_input_key'],cli)
        with patch.dict(os.environ,{**environment,'CREXX_QA_AFFECTED_TESTS':''}):
            self.assertEqual(P.canonical_input(a,'daily'),P.canonical_input(a,'daily',{**environment,'CREXX_QA_AFFECTED_TESTS':'[]'}))

    @unittest.skipIf(os.name=='nt', 'The maintained shell runner is exercised on Linux and Apple')
    def test_runner_failed_post_audit_cannot_publish(self):
        bindir=self.root/'commands'; bindir.mkdir()
        shim="#!"+sys.executable+"\n"+r"""import json,os,pathlib,sys
root=pathlib.Path(os.environ['QA_MOCK_ROOT']); name=pathlib.Path(sys.argv[0]).name; args=sys.argv[1:]
if name=='cmake': raise SystemExit(0)
if name=='ctest':
 (root/'executed').touch(); pathlib.Path(args[args.index('--output-junit')+1]).write_text('<testsuite><testcase name="one"/></testsuite>'); raise SystemExit(0)
script=pathlib.Path(args[0]).name; operation=args[1]
if script=='sanitizer-fixtures.py':
 if operation=='native':
  count=root/'audits'; value=int(count.read_text())+1 if count.exists() else 1; count.write_text(str(value)); raise SystemExit(0 if value==1 else 23)
 raise SystemExit(0)
if operation=='exclude': raise SystemExit(0)
if operation=='inventory': pathlib.Path(args[args.index('--output')+1]).write_text('{}'); raise SystemExit(0)
if operation=='complete': (root/'unexpected-complete').touch(); raise SystemExit(0)
raise SystemExit('unexpected control command')
"""
        for name in ('python3','cmake','ctest'):
            path=bindir/name; path.write_text(shim); path.chmod(0o755)
        environment={**os.environ,'PATH':str(bindir)+os.pathsep+os.environ.get('PATH',''), 'QA_MOCK_ROOT':str(self.root)}
        result=subprocess.run([str(ROOT/'tools/asan-run.sh'),'--build-dir',str(self.root/'build'),
                               '--log-root',str(self.root/'logs'),'--phase','full','--profile','full',
                               '--leaks','on','--build-leaks','on','--no-live-tail'],
                              env=environment,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=30)
        self.assertNotEqual(result.returncode,0,result.stdout)
        self.assertEqual((self.root/'audits').read_text(),'2',result.stdout)
        self.assertTrue((self.root/'executed').exists())
        self.assertFalse((self.root/'unexpected-complete').exists())
        self.assertEqual(list((self.root/'logs').glob('*/result.json')),[])

    def test_late_sanitizer_diagnostic_cannot_be_masked_by_success_text(self):
        source=self.root/'late-exit-source'; source.mkdir()
        build=self.root/'late-exit-build'
        (source/'late.py').write_text('import sys\nprint("SUSTAINED_VM|failures=0", flush=True)\nprint("==123==ERROR: LeakSanitizer: detected memory leaks", file=sys.stderr, flush=True)\nsys.exit(9)\n')
        (source/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.24)
project(LateExit NONE)
enable_testing()
add_test(NAME late COMMAND "@PYTHON@" "${CMAKE_CURRENT_SOURCE_DIR}/late.py")
if(SEMANTIC_REGEX)
 set_tests_properties(late PROPERTIES PASS_REGULAR_EXPRESSION "SUSTAINED_VM[|]failures=0")
endif()
set_tests_properties(late PROPERTIES FAIL_REGULAR_EXPRESSION "OWNING_FAILURE_ORACLE")
include("@HELPER@")
crexx_apply_sanitizer_failure_guards()
'''.replace('@PYTHON@',Path(sys.executable).as_posix()).replace('@HELPER@',(ROOT/'cmake/CrexxQaFixtures.cmake').as_posix()))
        command('cmake','-S',source,'-B',build,'-DSEMANTIC_REGEX=OFF')
        junit=build/'ctest.xml'
        command('ctest','--test-dir',build,'--output-junit',junit,success=False)
        # Existing semantic PASS expressions intentionally remain; the central
        # completion check must reject their otherwise green late diagnostic.
        command('cmake','-S',source,'-B',build,'-DSEMANTIC_REGEX=ON')
        command('ctest','--test-dir',build,'--output-junit',junit)
        identity={'preparation':'original-producers','native_products':{}}
        selection=build/'selection.json'; result=build/'result.json'
        F.json_write(selection,{'identity':identity,'selected':['late']})
        with patch.object(F,'validate'), patch.object(P,'fixture_tools',return_value=F), patch.object(P,'result_identity',return_value=identity):
            with self.assertRaisesRegex(ValueError,'sanitizer diagnostic in CTest output: late'):
                P.complete(build,selection,junit,result)
            for fatal in ('==123==LeakSanitizer has encountered a fatal error',
                          'AddressSanitizer: CHECK failed: internal condition'):
                junit.write_text('<testsuite><testcase name="late"><system-out>'+fatal+'</system-out></testcase></testsuite>')
                with self.assertRaisesRegex(ValueError,'sanitizer diagnostic in CTest output: late'):
                    P.complete(build,selection,junit,result)
        self.assertFalse(result.exists())
        command('cmake','-S',source,'-B',build,'-DCMAKE_C_FLAGS=-fsanitize=address')
        definitions,_=P.ctest_inventory(['--test-dir',str(build)])
        properties={item['name']:item['value'] for item in definitions['tests'][0]['properties']}
        self.assertIn('OWNING_FAILURE_ORACLE',properties['FAIL_REGULAR_EXPRESSION'])
        command('ctest','--test-dir',build,'--output-junit',junit,success=False)

    def test_complete_scope_rejects_false_broad(self):
        build=self.root/'build'; build.mkdir()
        names=['one', 'two']
        identity={'preparation':'original-producers',
                  'configuration':{'CMAKE_SYSTEM_NAME':'Linux', 'CMAKE_BUILD_TYPE':'Debug',
                                   'CMAKE_C_FLAGS':'-fsanitize=address'},
                  'execution':{'ASAN_OPTIONS':'detect_leaks=1', 'LSAN_OPTIONS':'',
                               'CREXX_QA_BUILD_LEAKS':'on', 'CREXX_QA_PHASE':'full'}}
        selection=build/'selection.json'; junit=build/'ctest.xml'; result=build/'result.json'
        def execute(value, selected=names, xml=None, after=None, profile='full', preparation='prepared'):
            F.json_write(selection, {'profile':profile, 'universe':names, 'selected':selected,
                                     'profile_selected':names, 'identity':value, 'selected_definitions':[], 'daily_preparation':preparation})
            junit.write_text(xml or '<testsuite><testcase name="one"/><testcase name="two"/></testsuite>')
            result.unlink(missing_ok=True)
            with patch.object(P, 'result_identity', return_value=after or value), patch.object(P, 'profile_names', return_value=set(names)), patch.object(F, 'validate'), patch.object(P, 'fixture_tools', return_value=F), patch.object(P, 'preparation_plan', return_value='prepared'):
                P.complete(build, selection, junit, result)
            return F.json_read(result)['scope']
        self.assertEqual(execute(identity), 'broad')
        self.assertEqual(execute(identity,profile='daily'),'daily')
        with self.assertRaisesRegex(ValueError, 'exact completed selected prerequisite'):
            execute(identity, profile='daily', preparation=None)
        self.assertEqual(execute(identity,['one'],'<testsuite><testcase name="one"/></testsuite>',profile='daily'),'focused')
        original={**identity,'native_products':{'original':'same'}}
        self.assertEqual(execute(original,after={**identity,'native_products':{'original':'same','new':'owned'}}),'broad')
        with self.assertRaisesRegex(ValueError,'native inputs changed'):
            execute(original,after={**identity,'native_products':{'original':'changed'}})
        self.assertFalse(result.exists())
        with self.assertRaisesRegex(ValueError, 'configuration keys=CMAKE_C_FLAGS'):
            execute(identity, after={**identity, 'configuration':{**identity['configuration'], 'CMAKE_C_FLAGS':'changed'}})
        self.assertFalse(result.exists())
        for change in ('ordinary', 'leaks', 'build-leaks', 'phase', 'external', 'unsafe'):
            value=json.loads(json.dumps(identity))
            if change=='ordinary': value['configuration']['CMAKE_C_FLAGS']=''
            elif change=='leaks': value['execution']['ASAN_OPTIONS']='detect_leaks=0'
            elif change=='build-leaks': value['execution']['CREXX_QA_BUILD_LEAKS']='off'
            elif change=='phase': value['execution']['CREXX_QA_PHASE']='ctest'
            elif change=='external': value['preparation']='external-ordinary-fixtures'
            else: value['execution']['ASAN_OPTIONS']+=':halt_on_error=0'
            self.assertEqual(execute(value), 'focused', change)
        self.assertEqual(execute(identity, ['one'], '<testsuite><testcase name="one"/></testsuite>'), 'focused')
        for xml in ('<testsuite/>', '<testsuite><testcase name="one"/><testcase name="two"><skipped/></testcase></testsuite>'):
            with self.assertRaises(ValueError): execute(identity, xml=xml)


if __name__ == '__main__':
    unittest.main()
