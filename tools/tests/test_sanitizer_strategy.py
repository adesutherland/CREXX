"""Behavioral policy, CMake argument, owner-copy and shared-cache contracts."""
import argparse
from collections import Counter
import importlib.util
import json
import os
from pathlib import Path, PurePosixPath, PureWindowsPath
import re
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
        # Keep the native temporary spelling: Windows may supply DOS short
        # names, which fixture containment must handle without a test workaround.
        self.root = Path(self.temporary.name)
        self.addCleanup(self.temporary.cleanup)

    def test_component_memory_cadence_and_regressions(self):
        self.assertIn('sustained_vm_values_rxtvm', P.MEMORY_TESTS)
        for name in ('future_functional_contract', 'classic_expression_contract_opt',
                     'sustained_vm_levell_rxbvm', 'levell_services_opt_rxtvm'):
            self.assertNotIn(name, P.MEMORY_TESTS)
        for names in P.MEMORY_REGRESSIONS.values():
            self.assertTrue(set(names) <= P.MEMORY_TESTS)
        known=set(re.findall(r'^#{2,3} (SAN-[0-9]+)\b',
                            (ROOT/'docs/SANITIZER-WORKLIST.md').read_text(encoding='utf-8'),re.MULTILINE))-{'SAN-009'}
        self.assertTrue(known <= P.MEMORY_REGRESSIONS.keys())
        invalid={'schema':1,'panels':{'one':{'boundary':'owned','tests':['duplicate','duplicate']}},'regressions':{}}
        with patch.object(Path, 'read_text', return_value=json.dumps(invalid)):
            with self.assertRaisesRegex(ValueError, 'duplicate'):
                P.memory_policy()
        for profile in P.BROAD_PROFILES:
            self.assertEqual(P.included(profile), '')
        for day in range(1, 8):
            self.assertEqual(P.decide('schedule', True, day)['profile'], 'daily')
            # An explicit manual choice remains independent of the weekday.
            for profile in ('full', 'daily'):
                self.assertEqual(P.decide('manual', False, day, profile)['profile'], profile)
            self.assertEqual(P.decide('tag', False, day, 'daily')['profile'], 'full')
        with self.assertRaisesRegex(ValueError, 'disabled'):
            P.decide('schedule', False, 2)

    def test_explicit_affected_scope_and_platform_absence(self):
        definitions={'tests':[{'name':'memory_compiler_lifecycle','properties':[]},
                              {'name':'future_functional_contract','properties':[]}]}
        self.assertEqual(P.profile_names(self.root,'daily',definitions), {'memory_compiler_lifecycle'})
        with patch.dict(os.environ, {'CREXX_QA_AFFECTED_TESTS':'["unknown"]'}):
            with self.assertRaisesRegex(ValueError, 'cannot be expanded'):
                P.affected_tests()

    def test_missing_known_memory_regression_cannot_qualify(self):
        definitions={'tests':[{'name':'one','properties':[]}]}
        with patch.object(P,'ctest_inventory',return_value=(definitions,{'one'})):
            with self.assertRaisesRegex(ValueError,'required memory contracts are missing'):
                P.inventory(self.root,'daily',[],self.root/'result.json')
        self.assertFalse((self.root/'result.json').exists())

    @unittest.skipUnless(os.name != 'nt', 'Stage boundary fixture uses executable scripts')
    def test_tool_memory_boundaries_and_expected_error_diagnostics(self):
        bindir=self.root/'stage-tools'; bindir.mkdir()
        log=self.root/'stage-calls'
        stub='''#!@PYTHON@
import os,sys
from pathlib import Path
args=sys.argv[1:]
with open(os.environ['STAGE_LOG'],'a') as stream: stream.write(Path(sys.argv[0]).name+'\\n')
if 'invalid.rexx' in ' '.join(args) or 'invalid.rxas' in ' '.join(args):
 print('SYNTAX ERROR',file=sys.stderr)
 if os.environ.get('STAGE_DIAGNOSTIC'): print('==123==ERROR: LeakSanitizer: detected memory leaks',file=sys.stderr)
 if os.environ.get('STAGE_ABNORMAL'): os.kill(os.getpid(),15)
 raise SystemExit(8)
output=args[args.index('-o')+1]
name=Path(sys.argv[0]).name
if name=='rxlink' and output in ('duplicate','invalid'):
 print('Duplicate providers' if output=='duplicate' else 'signature mismatch',file=sys.stderr); raise SystemExit(8)
if 'missing.rxpp' in ' '.join(args):
 print('RXPP_SOURCE_MISSING: Source file is missing',file=sys.stderr); raise SystemExit(8)
suffix='.rxas' if name in ('rxc','rxc-c') else '.rxbin' if name in ('rxas','rxlink') else ''
Path(output+suffix).write_text('owned output')
'''.replace('@PYTHON@',sys.executable)
        for name in ('rxc','rxc-c','rxas','rxlink','rxdas','rxpp'):
            tool=bindir/name; tool.write_text(stub); tool.chmod(0o755)
        trap=bindir/'rxvm'
        trap.write_text('#!/bin/sh\nprintf "rxvm\\n" >> "$STAGE_LOG"\nexit 37\n'); trap.chmod(0o755)
        for stage,name in (('compiler','rxc'),('classic','rxc-c'),('assembler','rxas'),
                           ('linker','rxlink'),('disassembler','rxdas'),('precomp','rxpp')):
            log.unlink(missing_ok=True)
            args=['cmake','-DSTAGE='+stage,'-DTOOL='+str(bindir/name),'-DSOURCE='+str(ROOT),
                  '-DBIN='+str(bindir),'-DINPUT='+str(bindir),'-DWORK='+str(self.root/stage),'-P',str(ROOT/'tests/memory/tool_lifecycle.cmake')]
            environment={**os.environ,'STAGE_LOG':str(log),'PATH':str(bindir)+os.pathsep+os.environ['PATH']}
            result=subprocess.run(args,env=environment,text=True,capture_output=True)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertEqual(set(log.read_text().splitlines()),{name})
            if stage in ('compiler','classic','assembler','disassembler'):
                result=subprocess.run(args,env={**environment,'STAGE_DIAGNOSTIC':'1'},text=True,capture_output=True)
                self.assertNotEqual(result.returncode,0)
                self.assertIn('reported a memory failure',result.stderr)
                result=subprocess.run(args,env={**environment,'STAGE_ABNORMAL':'1'},text=True,capture_output=True)
                self.assertNotEqual(result.returncode,0)
                self.assertIn('terminated abnormally',result.stderr)

    def test_tool_exit_statuses_preserve_error_and_crash_oracles(self):
        # Exercise the real CMake harness with Windows process results on every
        # host. The native Windows smoke tests additionally use the real tools.
        wrapper=self.root/'process-results.cmake'
        wrapper.write_text(r'''
set(WIN32 "${SIMULATE_WINDOWS}")
function(execute_process)
 cmake_parse_arguments(PARSE_ARGV 0 call ""
   "WORKING_DIRECTORY;RESULT_VARIABLE;OUTPUT_VARIABLE;ERROR_VARIABLE" "COMMAND")
 string(JOIN " " invocation ${call_COMMAND})
 if(invocation MATCHES "invalid\\.(rxas|rxbin)")
  set(result "${INJECT_STATUS}")
  set(error "${INJECT_DIAGNOSTIC}")
 else()
  set(result "${SUCCESS_STATUS}")
  list(FIND call_COMMAND -o index)
  math(EXPR index "${index}+1")
  list(GET call_COMMAND ${index} output)
  if(STAGE STREQUAL assembler)
   set(output "${WORK}/${output}.rxbin")
  endif()
  file(WRITE "${output}" "fresh component output")
 endif()
 set(${call_RESULT_VARIABLE} "${result}" PARENT_SCOPE)
 set(${call_OUTPUT_VARIABLE} "" PARENT_SCOPE)
 set(${call_ERROR_VARIABLE} "${error}" PARENT_SCOPE)
endfunction()
include("@HARNESS@")
'''.replace('@HARNESS@',(ROOT/'tests/memory/tool_lifecycle.cmake').as_posix()),encoding='utf-8')
        cases=[('TRUE','-1','SYNTAX ERROR',True),
               ('TRUE','255','SYNTAX ERROR',True),
               ('FALSE','255','SYNTAX ERROR',True),
               ('FALSE','-1','SYNTAX ERROR',False),
               ('TRUE','-1073741819','SYNTAX ERROR',False),
               ('TRUE','3221225477','SYNTAX ERROR',False),
               ('TRUE','1073741845','SYNTAX ERROR',False),
               ('TRUE','Subprocess aborted','SYNTAX ERROR',False),
               ('TRUE','-1','unrelated output',False),
               ('TRUE','-1','ERROR: LeakSanitizer: detected memory leaks',False),
               ('TRUE','0','SYNTAX ERROR',False)]
        for stage in ('assembler','disassembler'):
            args=['cmake','-DSTAGE='+stage,'-DTOOL=controlled-process-result',
                  '-DSOURCE='+str(ROOT),'-DINPUT='+str(self.root),
                  '-DWORK='+str(self.root/'process-work')]
            for windows,status,diagnostic,accepted in cases:
                with self.subTest(stage=stage,windows=windows,status=status,diagnostic=diagnostic):
                    command(*args,'-DSIMULATE_WINDOWS='+windows,'-DSUCCESS_STATUS=0',
                            '-DINJECT_STATUS='+status,'-DINJECT_DIAGNOSTIC='+diagnostic,
                            '-P',wrapper,success=accepted)
            # A valid output cannot turn a nonzero success-path exit into success.
            command(*args,'-DSIMULATE_WINDOWS=TRUE','-DSUCCESS_STATUS=-1',
                    '-DINJECT_STATUS=-1','-DINJECT_DIAGNOSTIC=SYNTAX ERROR',
                    '-P',wrapper,success=False)

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
        with patch.object(P,'MEMORY_TESTS',frozenset({'future_native_contract'})):
            pattern=P.included('daily',build)
            self.assertEqual(P.profile_names(build,'daily'),
                             {'future_native_contract','levelc_bif_reference_audit_prepare'})
        # Keep the full expression out of CreateProcess's Windows argument
        # limit. CTest's in-process script handler still compiles/applies it.
        pattern_file=self.root/'exclude.txt'; pattern_file.write_text(pattern)
        script=self.root/'select.cmake'; junit=self.root/'selected.xml'
        script.write_text('''set(CTEST_SOURCE_DIRECTORY "@SOURCE@")
set(CTEST_BINARY_DIRECTORY "@BUILD@")
set(CTEST_CMAKE_GENERATOR "Ninja")
ctest_start(Experimental)
file(READ "@PATTERN@" pattern)
ctest_test(INCLUDE "${pattern}" OUTPUT_JUNIT "@JUNIT@" RETURN_VALUE status)
if(NOT status EQUAL 0)
 message(FATAL_ERROR "Miniature selected tests failed")
endif()
'''.replace('@SOURCE@',source.as_posix()).replace('@BUILD@',build.as_posix()).replace('@PATTERN@',pattern_file.as_posix()).replace('@JUNIT@',junit.as_posix()))
        command('ctest','-S',script,'--output-on-failure')
        actual={case.attrib['name'] for case in ET.parse(junit).getroot().findall('.//testcase')}
        expected={'future_native_contract','levelc_bif_reference_audit_prepare'}
        self.assertEqual(actual,expected)
        self.assertNotRegex('arg_semantics_scalar_opt_future',pattern)

    @unittest.skipUnless(os.name != 'nt' and shutil.which('ninja'), 'Cold producer fixture requires executable scripts and Ninja')
    def test_split_runtime_cold_build_prepares_certified_exits(self):
        source = self.root / 'split source'
        source.mkdir()
        build = self.root / 'split build'
        producer = '''#!@PYTHON@
from pathlib import Path
import sys
tool = Path(sys.argv[0]).name
if tool == 'rxc' and not Path('@EXITS@').is_file():
    sys.exit("INTERNAL EXIT ERROR: Failed to load certified exit module 'rxcexits'")
output = sys.argv[sys.argv.index('-o') + 1]
Path(output + ('.rxas' if tool == 'rxc' else '')).write_text('owned fixture')
'''.replace('@PYTHON@', sys.executable).replace('@EXITS@', (build/'bin/rxcexits.rxbin').as_posix())
        for tool in ('rxc', 'rxas', 'rxlink'):
            executable = source / tool
            executable.write_text(producer)
            executable.chmod(0o755)
        (source/'sample.crexx').write_text('fixture source')
        (source/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.24)
project(SplitRuntimeContract NONE)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(tool IN ITEMS rxc rxas rxlink)
 add_executable(${tool} IMPORTED)
 set_target_properties(${tool} PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/${tool}")
endforeach()
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
 COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin")
add_custom_target(compiler_exit_bin DEPENDS "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin")
file(WRITE "${CMAKE_BINARY_DIR}/bin/library.rxbin" "fixture library")
add_custom_target(library)
function(crexx_add_fixture_command)
 add_custom_command(${ARGV})
endfunction()
include("@HELPER@")
foreach(mode IN ITEMS noopt opt)
 crexx_add_qa_runtime_case(sample "${CMAKE_SOURCE_DIR}/sample.crexx" ${mode} script target)
endforeach()
'''.replace('@HELPER@', (ROOT/'cmake/CrexxQaRuntimeCase.cmake').as_posix()))
        command('cmake', '-S', source, '-B', build, '-G', 'Ninja')
        command('cmake', '--build', build, '--target', 'qa-runtime-sample-noopt', 'qa-runtime-sample-opt', '--parallel', '4')
        self.assertTrue((build/'bin/rxcexits.rxbin').is_file())
        for mode, suffix in (('noopt', ''), ('opt', '-linked')):
            self.assertTrue((build/f'qa-runtime/sample_{mode}/sample_run_{mode}{suffix}.rxbin').is_file())
        # Losing the generated module must rebuild its owner before producers.
        (build/'bin/rxcexits.rxbin').unlink()
        command('cmake', '--build', build, '--target', 'qa-runtime-sample-noopt', 'qa-runtime-sample-opt')
        self.assertTrue((build/'bin/rxcexits.rxbin').is_file())

    @unittest.skipUnless(os.name != 'nt' and shutil.which('ninja'), 'Producer freshness fixture requires executable scripts and Ninja')
    def test_runtime_producers_track_published_certified_exits(self):
        source = self.root / 'exits source'; source.mkdir()
        build = self.root / 'exits build'
        # Use the maintained matrix helper and exact worker producer rule.
        # The fake tools expose which published module each recipe observed.
        (source / 'cmake').symlink_to(ROOT / 'cmake', target_is_directory=True)
        definitions = (ROOT / 'interpreter/CMakeLists.txt').read_text()
        worker = re.search(r'add_custom_command\(\s*OUTPUT test_persistent_worker_executor\.rxbin\s*'
                           r'COMMAND \$<TARGET_FILE:rxc>.*?COMMENT "[^"]*"\)',
                           definitions, re.S)
        self.assertIsNotNone(worker)
        producer = '''#!@PYTHON@
from pathlib import Path
import sys
tool = Path(sys.argv[0]).name
output = sys.argv[sys.argv.index('-o') + 1] if '-o' in sys.argv else sys.argv[-1]
suffix = '.rxas' if tool == 'rxc' else '.rxbin'
if not output.endswith(suffix):
    output += suffix
Path(output).write_bytes(Path('@BUILD@/bin/rxcexits.rxbin').read_bytes())
with Path('@BUILD@/calls.txt').open('a') as calls:
    calls.write(tool + '\\n')
'''.replace('@PYTHON@', sys.executable).replace('@BUILD@', build.as_posix())
        for tool in ('rxc', 'rxas', 'rxlink', 'rxvm', '_e5native'):
            executable = source / tool
            executable.write_text(producer); executable.chmod(0o755)
        (source / 'sample.crexx').write_text('fixture source')
        worker_source = source / 'tests/rexx/test_persistent_worker_executor.crexx'
        worker_source.parent.mkdir(parents=True); worker_source.write_text('worker fixture source')
        exits = source / 'exits.txt'; exits.write_text('first')
        (source / 'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.24)
project(ProducerExitsFreshness NONE)
enable_testing()
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(tool IN ITEMS rxc rxas rxlink rxvm _e5native)
 add_executable(${tool} IMPORTED)
 set_target_properties(${tool} PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/${tool}")
endforeach()
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
 COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/exits.txt" "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
 DEPENDS "${CMAKE_SOURCE_DIR}/exits.txt")
add_custom_target(compiler_exit_bin DEPENDS "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin")
function(crexx_schedule_directory_qa_tier_finalization)
endfunction()
include("${CMAKE_SOURCE_DIR}/cmake/CrexxQaFixtures.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake")
set(CREXX_RXVM_EXECUTABLE "$<TARGET_FILE:rxvm>")
add_custom_target(sample_group)
crexx_add_rexx_opt_matrix(NAME sample SOURCE "${CMAKE_SOURCE_DIR}/sample.crexx"
 TARGET_GROUP sample_group RUNNER rxvm)
@WORKER@
add_custom_target(worker_image DEPENDS "${CMAKE_BINARY_DIR}/test_persistent_worker_executor.rxbin")
'''.replace('@WORKER@', worker.group(0)))
        command('cmake', '-S', source, '-B', build, '-G', 'Ninja')

        def prepare():
            command('cmake', '--build', build, '--target', 'sample_group', 'worker_image', '--parallel', '4')

        images = [build / 'sample_noopt.rxbin', build / 'sample_opt.rxbin',
                  build / 'test_persistent_worker_executor.rxbin']
        prepare()
        self.assertEqual([image.read_text() for image in images], ['first'] * 3)
        calls = (build / 'calls.txt').read_text()
        prepare()
        self.assertEqual((build / 'calls.txt').read_text(), calls, 'Warm preparation must be a no-op')
        exits.write_text('second')
        prepare()
        self.assertEqual([image.read_text() for image in images], ['second'] * 3)
        self.assertEqual(len((build / 'calls.txt').read_text().splitlines()), 12)
        images[0].unlink(); images[2].unlink()
        prepare()
        self.assertEqual([image.read_text() for image in images], ['second'] * 3)
        self.assertEqual(len((build / 'calls.txt').read_text().splitlines()), 16)
        (build / 'bin/rxcexits.rxbin').unlink()
        prepare()
        self.assertEqual([image.read_text() for image in images], ['second'] * 3)
        self.assertEqual(len((build / 'calls.txt').read_text().splitlines()), 22)

    @unittest.skipUnless(os.name != 'nt' and shutil.which('make'), 'Parallel owner fixture requires Unix Makefiles and executable scripts')
    def test_level_l_sdk_make_producers_have_one_owner(self):
        read_text = Path.read_text
        def core_input(path, *args, **kwargs):
            self.assertFalse(path.is_relative_to(ROOT/'research'), 'Core SDK check read research input')
            return read_text(path, *args, **kwargs)
        with patch.object(Path, 'read_text', core_input):
            self._level_l_make_owner_contract(include_research=False)

    @unittest.skipUnless(os.name != 'nt' and shutil.which('make'), 'Parallel owner fixture requires Unix Makefiles and executable scripts')
    def test_level_l_make_producers_have_one_owner(self):
        if os.environ.get('CREXX_QA_RESEARCH_COMPILERS', 'OFF').upper() not in ('1', 'ON', 'TRUE', 'YES'):
            self.skipTest('Research compiler ownership requires CREXX_BUILD_RESEARCH_COMPILERS=ON')
        self._level_l_make_owner_contract(include_research=True)

    def _level_l_make_owner_contract(self, include_research):
        # Always use the maintained SDK graph. The optional research extension
        # adds the original grammar/client graph without changing its oracles.
        # Only tools, source contents and the Unicode generator are substituted.
        source=(self.root/'make-source').resolve(); source.mkdir()
        build=(self.root/'make-build').resolve()
        calls=(self.root/'make-calls').resolve()
        sdk=source/'sdk'; sdk.mkdir()
        sdk_declarations=(ROOT/'lib/rxfnsl/rexx/CMakeLists.txt').read_text()
        (sdk/'CMakeLists.txt').write_text(sdk_declarations)
        modules=re.search(r'set\(RXFNSL_REXX_MODULES\s+([^)]*)\)',sdk_declarations).group(1).split()
        for name in modules:
            (sdk/(name+'.crexx')).write_text('fixture source\n')
        (source/'unicode').mkdir()
        (source/'unicode/Properties.cmake').write_text(
            'set(_ll_data_source "${CMAKE_CURRENT_SOURCE_DIR}/lludata.crexx")\n')

        research = self._level_l_make_research_fixture(source) if include_research else None
        producer=r'''#!@PYTHON@
from pathlib import Path
import os,sys,time
tool=Path(sys.argv[0]).name
args=sys.argv[1:]
if tool=='rxlang':
    output=Path(args[0]).resolve()
elif tool=='rxc':
    output=Path(args[args.index('-o')+1]+'.rxas').resolve()
    for directory in args[args.index('-i')+1].split(';'):
        for imported in Path(directory).glob('*.rxbin'):
            assert imported.read_text()=='complete\n', imported
elif tool=='rxas':
    stem=Path(args[-1])
    assert Path(str(stem)+'.rxas').read_text()=='complete\n', stem
    output=Path(str(stem)+'.rxbin').resolve()
else:
    control=Path(args[args.index('-c')+1]).read_text().splitlines()
    for line in control:
        if line.startswith('INPUT '):
            imported=Path(line[6:])
            assert imported.read_text()=='complete\n', imported
    output=Path(next(line[7:] for line in control if line.startswith('OUTPUT '))+'.rxbin').resolve()
lock=Path(str(output)+'.active')
descriptor=os.open(lock,os.O_WRONLY|os.O_CREAT|os.O_EXCL,0o600)
os.close(descriptor)
try:
    descriptor=os.open(@CALLS@,os.O_WRONLY|os.O_CREAT|os.O_APPEND,0o600)
    os.write(descriptor,(tool+'\t'+str(output)+'\n').encode())
    os.close(descriptor)
    # Keep simultaneous missing-output recipes observable on fast machines.
    time.sleep(0.05)
    if tool=='rxlang':
        print('complete')
    else:
        output.write_text('complete\n')
finally:
    lock.unlink()
'''.replace('@PYTHON@',sys.executable).replace('@CALLS@',repr(str(calls)))
        for tool in ('rxc','rxas','rxlink','rxlang'):
            executable=source/tool
            executable.write_text(producer); executable.chmod(0o755)
        (source/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.24)
project(LevelLOwnerContract NONE)
include("@ROOT@/cmake/CrexxImportRoots.cmake")
function(crexx_add_fixture_command)
  add_custom_command(${ARGV})
endfunction()
foreach(tool IN ITEMS rxc rxas rxlink rxlang)
  add_executable(${tool} IMPORTED)
  set_target_properties(${tool} PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/${tool}")
endforeach()
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin" "${CMAKE_BINARY_DIR}/langtools/common")
file(WRITE "${CMAKE_BINARY_DIR}/bin/library.rxbin" "complete\\n")
add_custom_target(library)
foreach(module IN ITEMS llrxastext llbinmeta)
  file(WRITE "${CMAKE_BINARY_DIR}/langtools/common/${module}.rxbin" "complete\\n")
  add_custom_target(${module})
endforeach()
add_subdirectory(sdk)
@RESEARCH@
'''.replace('@ROOT@',ROOT.as_posix()).replace('@RESEARCH@',
        'add_subdirectory(research)' if include_research else ''))
        command('cmake','-S',source,'-B',build,'-G','Unix Makefiles')
        def prepare():
            command('cmake','--build',build,'--parallel','8')
        def events():
            return Counter(tuple(line.split('\t')) for line in calls.read_text().splitlines())
        expected={('rxlink',str(build/'bin/rxfnsl.rxbin'))}
        if include_research:
            expected.update({('rxlang',str(research/'levelc/levelc.rxl')),
                             ('rxas',str(build/'research/frontends/llcgrammar.rxbin'))})
        for name in modules:
            expected.update({('rxc',str(build/'sdk'/(name+'.rxas'))),
                             ('rxas',str(build/'sdk'/(name+'.rxbin')))})
        if include_research:
            for mode in ('opt','noopt'):
                for directory,name in ((f'common/{mode}','llscalar'),
                                       (f'common/{mode}','llcompilercontracts'),
                                       (f'clients/{mode}','client')):
                    expected.update({('rxc',str(build/'research'/directory/(name+'.rxas'))),
                                     ('rxas',str(build/'research'/directory/(name+'.rxbin')))})
        prepare()
        self.assertEqual(events(),Counter({event:1 for event in expected}))
        prepare()
        self.assertEqual(events(),Counter({event:1 for event in expected}))
        calls.write_text('')
        (build/'sdk/llcheck.rxbin').unlink()
        if include_research:
            (build/'research/frontends/llcgrammar.rxbin').unlink()
        prepare()
        recovered=events()
        self.assertTrue(recovered)
        self.assertEqual(set(recovered.values()),{1})
        required=[('rxc',str(build/'sdk/llcheck.rxas')),
                  ('rxas',str(build/'sdk/llcheck.rxbin'))]
        if include_research:
            required.extend([('rxlang',str(research/'levelc/levelc.rxl')),
                             ('rxas',str(build/'research/frontends/llcgrammar.rxbin'))])
        for event in required:
            self.assertEqual(recovered[event],1)
        self.assertNotIn(('rxc',str(build/'sdk/llir.rxas')),recovered)

    def _level_l_make_research_fixture(self, source):
        compiler=(ROOT/'research/level-l/compilers/CMakeLists.txt').read_text()
        def declarations(begin,end):
            self.assertEqual(compiler.count(begin),1)
            start=compiler.index(begin)
            self.assertEqual(compiler[start:].count(end),1)
            return compiler[start:compiler.index(end,start)]
        # This full head identifies the common loop immediately after the
        # grammar declarations; later mode loops belong to other fixtures.
        common_head='''foreach(_mode opt noopt)
  set(_flags)
  if(_mode STREQUAL "noopt")
    set(_flags -n)
  endif()
  set(_common "${CMAKE_CURRENT_BINARY_DIR}/common/${_mode}")'''
        frontend=declarations('function(ll_compile NAME SOURCE DIRECTORY)',
                              '\n'+common_head)
        common=declarations(common_head, '\n  set(_allhosts)')
        research=source/'research'; research.mkdir()
        (research/'levelc').mkdir()
        (research/'levelc/levelc.rxl').write_text('fixture grammar\n')
        (research/'common').mkdir()
        for name in ('llscalar','llcompilercontracts'):
            (research/'common'/(name+'.crexx')).write_text('fixture source\n')
        (research/'client.crexx').write_text('fixture consumer\n')
        (research/'CMakeLists.txt').write_text('set(_languages c)\n'+frontend+'\n'+common+'''
  set(_dir "${CMAKE_CURRENT_BINARY_DIR}/clients/${_mode}")
  ll_compile(client "${CMAKE_CURRENT_SOURCE_DIR}/client.crexx" "${_dir}"
    ${_base} ${_shared} "${_front}/llcgrammar.rxbin")
  add_custom_target(client_${_mode} ALL DEPENDS "${_dir}/client.rxbin")
endforeach()
''')
        generator=source/'langtools/cmake'; generator.mkdir(parents=True)
        shutil.copyfile(ROOT/'langtools/cmake/generate.cmake',generator/'generate.cmake')
        return research

    def workflow_block(self, name):
        workflow = (ROOT/'.github/workflows/sanitizers.yml').read_text().splitlines()
        start = workflow.index('      - name: ' + name)
        start = workflow.index('        run: |', start) + 1
        body = []
        for line in workflow[start:]:
            if line.startswith('          '): body.append(line[10:])
            elif not line: body.append('')
            else: break
        return '\n'.join(body).replace('${{ needs.assurance-gate.outputs.target_sha }}', 'a' * 40)

    @unittest.skipUnless(Path('/bin/bash').is_file(), 'Workflow source contract requires Bash')
    def test_workflow_source_resolution_preserves_tags_and_explicit_refs(self):
        bindir = self.root/'source-bin'; bindir.mkdir()
        output = self.root/'source-output'
        calls = self.root/'source-calls'
        gh = bindir/'gh'
        gh.write_text('#!/bin/sh\nprintf "%s\\n" "$*" >> "$QA_SOURCE_CALLS"\nprintf "%s\\n" development-head\n')
        gh.chmod(0o755)
        cases = [('schedule', 'refs/heads/master', '', 'development-head'),
                 ('workflow_dispatch', 'refs/heads/master', '', 'development-head'),
                 ('workflow_dispatch', 'refs/heads/hotfix', '', 'selected-head'),
                 ('workflow_dispatch', 'refs/heads/master', 'explicit-head', 'explicit-head'),
                 ('push', 'refs/tags/v1.0.0', '', 'selected-head')]
        for event, ref, requested, expected in cases:
            with self.subTest(event=event, ref=ref, requested=requested):
                output.unlink(missing_ok=True); calls.unlink(missing_ok=True)
                env = {**os.environ, 'PATH': str(bindir)+os.pathsep+os.environ['PATH'],
                       'GITHUB_OUTPUT': str(output), 'GITHUB_SHA': 'selected-head',
                       'GITHUB_REPOSITORY': 'fixture/crexx', 'GITHUB_REF': ref,
                       'GITHUB_EVENT_NAME': event, 'REQUESTED_SOURCE': requested,
                       'QA_SOURCE_CALLS': str(calls)}
                result = subprocess.run(['/bin/bash', '-c', self.workflow_block('Resolve source')],
                                        env=env, text=True, capture_output=True)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(output.read_text().strip(), 'target_sha='+expected)
                self.assertEqual(calls.exists(), expected == 'development-head')

    @unittest.skipUnless(Path('/bin/bash').is_file(), 'Workflow shell contract requires Bash')
    def test_workflow_commands_preserve_optional_arguments(self):
        source = self.root/'workflow source'
        (source/'tools').mkdir(parents=True)
        bindir = source/'bin'; bindir.mkdir()
        log = self.root/'calls.jsonl'
        recorder = '''#!@PYTHON@
import json, os, sys
from pathlib import Path
name = Path(sys.argv[0]).name
with open(os.environ['CALL_LOG'], 'a') as stream:
    stream.write(json.dumps([name, sys.argv[1:]]) + '\\n')
if name == 'python3': print(os.environ['QA_PROFILE'])
'''.replace('@PYTHON@', sys.executable)
        for executable in (bindir/'cmake', bindir/'python3', source/'tools/asan-run.sh'):
            executable.write_text(recorder); executable.chmod(0o755)
        environment = {**os.environ, 'PATH': str(bindir)+os.pathsep+os.environ['PATH'],
            'CALL_LOG': str(log), 'RUNNER_TEMP': str(self.root/'runner temp'),
            'GITHUB_OUTPUT': str(self.root/'output'), 'GITHUB_STEP_SUMMARY': str(self.root/'summary'),
            'GITHUB_EVENT_NAME': 'workflow_dispatch'}
        shells = sorted({str(Path(p).resolve()) for p in ('/bin/bash', shutil.which('bash') or '/bin/bash')})

        def execute(shell, name, **values):
            log.unlink(missing_ok=True)
            result = subprocess.run([shell, '-c', self.workflow_block(name)], cwd=source,
                env={**environment, **values}, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            self.assertEqual(result.returncode, 0, result.stdout)
            return [json.loads(line) for line in log.read_text().splitlines()]

        for shell in shells:
            for profile in ('full', 'daily'):
                for platform in ('linux', 'macos'):
                    for baseline in ('', '{"linux":"old","macos":"old"}'):
                        calls = execute(shell, 'Configure matching ordinary and instrumented inputs',
                            QA_PROFILE=profile, QA_PLATFORM=platform, QA_QUALIFICATION=profile,
                            GITHUB_EVENT_NAME='schedule', QA_BASELINES=baseline)
                        cmake = [args for name, args in calls if name == 'cmake']
                        self.assertEqual(len(cmake), 2 if profile == 'daily' else 1)
                        self.assertIn('-DCMAKE_C_FLAGS=-fsanitize=address -fno-omit-frame-pointer', cmake[-1])
                        donor = '-DCREXX_QA_FIXTURE_TREE=' + environment['RUNNER_TEMP'] + '/ordinary'
                        self.assertEqual(donor in cmake[-1], profile == 'daily')
                        self.assertEqual(Path(environment['GITHUB_OUTPUT']).read_text().splitlines()[-1],
                                         'profile='+profile)
            for platform in ('linux', 'macos'):
                for qualification in ('daily', 'continuation-linux'):
                    regex = '^(one|two with spaces)$'
                    calls = execute(shell, 'Complete instrumented build and selected runtime scope',
                        QA_PROFILE='daily', QA_PLATFORM=platform, QA_QUALIFICATION=qualification,
                        QA_CONTINUATION_REGEX=regex)
                    self.assertEqual(len(calls), 1)
                    self.assertEqual(calls[0][0], 'asan-run.sh')
                    args = calls[0][1]
                    self.assertIn(environment['RUNNER_TEMP'] + '/debugasan', args)
                    self.assertEqual(args[args.index('--leaks') + 1], 'off' if platform == 'macos' else 'on')
                    self.assertEqual(args[args.index('--build-leaks') + 1], 'off' if platform == 'macos' else 'on')
                    self.assertEqual(args[-2:] == ['--regex', regex], qualification == 'continuation-linux')
            for regex in ('', '^(one|two with spaces)$'):
                calls = execute(shell, 'Candidate ordinary correctness (Linux donor)',
                    QA_PROFILE='full', QA_ORDINARY_REGEX=regex)
                self.assertEqual(len(calls), 1)
                args = calls[0][1]
                self.assertEqual('--regex' in args, bool(regex))
                if regex: self.assertEqual(args[-2:], ['--regex', regex])

    @unittest.skipUnless(Path('/bin/bash').is_file(), 'Workflow matrix contract requires Bash')
    def test_manual_daily_platform_selection_cannot_narrow_full_scope(self):
        source = self.root/'matrix'; source.mkdir()
        (source/'tools').symlink_to(ROOT/'tools', target_is_directory=True)
        bindir=source/'bin'; bindir.mkdir()
        git=bindir/'git'; git.write_text('#!/bin/sh\nprintf "%s\\n" ' + 'a'*40 + '\n'); git.chmod(0o755)
        output=source/'output'
        environment={**os.environ, 'PATH': str(bindir)+os.pathsep+os.environ['PATH'],
            'GITHUB_OUTPUT': str(output), 'GITHUB_STEP_SUMMARY': str(source/'summary'),
            'GITHUB_REF': 'refs/heads/hotfix', 'GITHUB_EVENT_NAME': 'workflow_dispatch',
            'SANITIZER_DAILY_ACTIVATED': 'false'}

        def execute(scope, platform, success=True, event='workflow_dispatch'):
            output.unlink(missing_ok=True)
            result=subprocess.run(['/bin/bash','-c',self.workflow_block('Select profile and independent platform jobs')],
                cwd=source,env={**environment,'REQUESTED_SCOPE':scope,'DAILY_PLATFORM':platform,'GITHUB_EVENT_NAME':event},
                text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            self.assertEqual(result.returncode==0,success,result.stdout)
            if not success:
                self.assertFalse(output.exists())
                return
            values=dict(line.split('=',1) for line in output.read_text().splitlines())
            return json.loads(values['matrix'])['include']

        self.assertTrue(all(row['profile']=='daily' for row in execute('','both')))
        for scope in ('full','daily'):
            rows=execute(scope,'both')
            self.assertEqual({row['platform'] for row in rows},{'linux','macos'})
            self.assertTrue(all(row['profile']==scope for row in rows))
        for platform in ('linux','macos'):
            rows=execute('daily',platform)
            self.assertEqual([(row['platform'],row['profile']) for row in rows],[(platform,'daily')])
            execute('full',platform,success=False)
            execute('candidate',platform,success=False)
        execute('daily','macos',success=False,event='schedule')

    @unittest.skipUnless(Path('/bin/bash').is_file(), 'Workflow matrix contract requires Bash')
    def test_scheduled_scope_ignores_history_and_changed_inputs(self):
        from datetime import datetime, timezone
        source = self.root/'scheduled'; source.mkdir()
        (source/'tools').symlink_to(ROOT/'tools', target_is_directory=True)
        bindir = source/'bin'; bindir.mkdir()
        git = bindir/'git'
        # Source resolution is allowed; any reintroduced history/diff query fails.
        git.write_text('#!/bin/sh\n[ "$*" = "rev-parse HEAD" ] || exit 37\nprintf "%s\\n" ' + 'a'*40 + '\n')
        git.chmod(0o755)
        for name in ('compiler/native.c', 'CMakeLists.txt', 'AGENTS.md', 'unclassified.input'):
            path = source/name; path.parent.mkdir(exist_ok=True); path.write_text('changed')
        history = source/'.ci-assurance/history.json'; history.parent.mkdir()
        output = source/'output'
        expected = 'daily'
        for previous in (None, 'invalid stale history', json.dumps({'head':'missing','native_configs':{'linux':'old'}})):
            with self.subTest(history=previous):
                if previous is not None: history.write_text(previous)
                result = subprocess.run(['/bin/bash', '-c', self.workflow_block('Select profile and independent platform jobs')],
                    cwd=source, env={**os.environ, 'PATH':str(bindir)+os.pathsep+os.environ['PATH'],
                        'GITHUB_OUTPUT':str(output), 'GITHUB_STEP_SUMMARY':str(source/'summary'),
                        'GITHUB_REF':'refs/heads/master', 'GITHUB_EVENT_NAME':'schedule',
                        'SANITIZER_DAILY_ACTIVATED':'true', 'REQUESTED_SCOPE':'', 'DAILY_PLATFORM':'both'},
                    text=True, capture_output=True)
                self.assertEqual(result.returncode, 0, result.stderr)
                values = dict(line.split('=',1) for line in output.read_text().splitlines())
                rows = json.loads(values['matrix'])['include']
                self.assertEqual(values['profile'], expected)
                self.assertEqual({row['platform'] for row in rows}, {'linux','macos'})
                self.assertTrue(all(row['profile'] == expected and not row.get('affected') for row in rows))

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

    def test_fixture_outputs_reject_outside_build_tree(self):
        source = self.root / 'containment-source'; source.mkdir()
        build = self.root / 'containment-build'
        (source / 'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.24)
project(Containment NONE)
include("@HELPER@")
crexx_add_fixture_command(OUTPUT "${TEST_OUTPUT}"
  COMMAND "${CMAKE_COMMAND}" -E touch "${TEST_OUTPUT}" VERBATIM)
'''.replace('@HELPER@', (ROOT / 'cmake/CrexxQaFixtures.cmake').as_posix()))
        command('cmake', '-S', source, '-B', build, '-G', 'Ninja',
                f'-DTEST_OUTPUT={build / "owned.rxbin"}')
        outside = [self.root / 'outside.rxbin',
                   self.root / 'containment-build-sibling' / 'outside.rxbin']
        if os.name == 'nt':
            other_drive = 'Y:' if self.root.drive.upper() == 'Z:' else 'Z:'
            outside.append(Path(other_drive + '/outside.rxbin'))
        for output in outside:
            with self.subTest(output=output):
                result = command('cmake', '-S', source, '-B', build,
                                 f'-DTEST_OUTPUT={output}', success=False)
                self.assertIn('QA fixture must belong to the build tree', result.stdout)

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

    @unittest.skipUnless(shutil.which('ninja') and shutil.which('git'), 'Cold native planning fixture requires Ninja and Git')
    @patch.object(P,'MEMORY_TESTS',frozenset({'future_native_contract'}))
    def test_cold_and_warm_native_preparation_match(self):
        source=self.root/'cold-source'; source.mkdir(); command('git','init','-q',source)
        build=self.root/'cold-build'
        (source/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.24)
project(ColdNative C)
enable_testing()
include("@HELPER@")
foreach(name rxc rxas rxlink rxvm rxbvm rxtvm crexx library classlib rxfnsc rxfnsg)
 add_custom_target(${name})
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/host.c" "int main(void) { return 0; }\\n")
add_executable(owned_host "${CMAKE_BINARY_DIR}/host.c")
add_test(NAME future_native_contract COMMAND owned_host)
set_tests_properties(future_native_contract PROPERTIES CREXX_PREP_TARGETS owned_host)
crexx_finalize_qa_fixtures()
'''.replace('@HELPER@',(ROOT/'cmake/CrexxQaFixtures.cmake').as_posix()))
        command('cmake','-S',source,'-B',build,'-G','Ninja')
        definitions,_=P.ctest_inventory(['--test-dir',str(build)])
        cold_test=next(t for t in definitions['tests'] if t['name']=='future_native_contract')
        self.assertFalse(cold_test.get('command'))
        cold=P.preparation_plan(build,'daily')
        command('cmake','--build',build,'--target','owned_host')
        definitions,_=P.ctest_inventory(['--test-dir',str(build)])
        warm_test=next(t for t in definitions['tests'] if t['name']=='future_native_contract')
        self.assertTrue(warm_test.get('command'))
        self.assertEqual(cold,P.preparation_plan(build,'daily'))

    def test_selected_preparation_fails_unknown_owner_and_retains_native_children(self):
        build=self.root/'prepared'; build.mkdir()
        baseline=('rxc','rxas','rxlink','rxvm','rxbvm','rxtvm','crexx','library','classlib','rxfnsc','rxfnsg')
        registry=[name+'\tUTILITY\t' for name in baseline]
        registry.extend(['owned_host\tEXECUTABLE\t'+str(build/'host'),
                         'ordinary_image\tUTILITY\t', 'unused_weekly_image\tUTILITY\t',
                         'implicit_provider\tMODULE_LIBRARY\t'+str(build/'provider.rxplugin')])
        (build/'qa-build-targets.tsv').write_text('\n'.join(registry)+'\n')
        tests=[{'name':'host','command':[str(build/'host')], 'properties':[{'name':'CREXX_PREP_TARGETS','value':'owned_host'}]},
               {'name':'wrapper','command':['cmake','-P','runtime.cmake'], 'properties':[
                   {'name':'CREXX_PREP_TARGETS','value':'unused_weekly_image'},
                   {'name':'CREXX_DAILY_PREP_TARGETS','value':'ordinary_image;rxvm'}]},
               {'name':'qa_control','command':['python','contract.py'], 'properties':[
                   {'name':'CREXX_PREP_NONE','value':True}]}]
        with patch.object(P,'ctest_inventory',return_value=({'tests':tests},set(t['name'] for t in tests))), patch.object(F,'identity',return_value={'source':'same'}), patch.object(P,'fixture_tools',return_value=F), patch.object(P,'MEMORY_TESTS',frozenset({'host','wrapper','qa_control','future_wrapper'})):
            value=P.preparation_plan(build,'daily')
            self.assertIn('owned_host',value['targets'])
            self.assertIn('ordinary_image',value['targets'])
            self.assertIn('implicit_provider',value['targets'])
            self.assertNotIn('unused_weekly_image',value['targets'])
            self.assertFalse(set(value['targets']) & {'all','ALL','qa-prep'})
            tests[0].pop('command')
            self.assertEqual(value,P.preparation_plan(build,'daily'))
            tests[0]['command']=[str(build/'host')]
            tests[0]['properties']=[]
            with self.assertRaisesRegex(ValueError,'host'):P.preparation_plan(build,'daily')
            tests[0]['properties']=[{'name':'CREXX_PREP_TARGETS','value':'owned_host'}]
            tests.append({'name':'future_wrapper','command':['cmake','-P','new.cmake'],'properties':[]})
            with self.assertRaisesRegex(ValueError,'future_wrapper'):P.preparation_plan(build,'daily')
            tests.pop()
            tests[2]['properties'][0]['value']='FALSE'
            with self.assertRaisesRegex(ValueError,'qa_control'):P.preparation_plan(build,'daily')
            tests[2]['properties'][0]['value']=True
            tests[1]['properties'][1]['value']='missing_fixture_target'
            with self.assertRaisesRegex(ValueError,'missing_fixture_target'):P.preparation_plan(build,'daily')

    @unittest.skipUnless(shutil.which('git'), 'Source-identity contracts require Git')
    def test_test_consumed_docs_and_changed_input_identity(self):
        source = self.root/'src'; source.mkdir(); command('git', 'init', '-q', source)
        (source/'sample.c').write_text('a')
        (source/'docs').mkdir(); document=source/'docs'/'oracle.md'; document.write_text('one')
        ordinary, result = F.source_identity(source, False), F.source_identity(source)
        document.write_text('two')
        self.assertEqual(ordinary, F.source_identity(source, False))
        self.assertNotEqual(result, F.source_identity(source))
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
        common={'CMAKE_HOME_DIRECTORY':str(source),'DSLSH_SOURCE_DIR':str(dependency),'CMAKE_BUILD_TYPE':'Debug',
                'CREXX_BUILD_RESEARCH_COMPILERS':'OFF'}
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
        F.json_write(native/F.CONFIG,{**common,'CREXX_QA_FIXTURE_TREE':str(donor),
                                     'CREXX_BUILD_RESEARCH_COMPILERS':'ON'})
        with self.assertRaisesRegex(ValueError,'configuration keys=CREXX_BUILD_RESEARCH_COMPILERS'): F.validate(native)
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
        # Required names must use the same host spelling as manifest publication.
        manifest={'cases':{'sample':{'providers':[{'filename':'provider.crexx'}]}}}
        for path_type in (PurePosixPath, PureWindowsPath):
            root=path_type('prepared')
            products=[root / mode / 'sample' / (name + suffix)
                      for mode in ('noopt', 'opt') for name in ('program', 'provider')
                      for suffix in ('.rxas', '.rxbin')]
            published={str(path.relative_to(root)) for path in products}
            with patch.object(C, 'Path', path_type):
                self.assertEqual(C.ContractFixture.required_artifacts(manifest), published)
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
        restored=[]
        pretty={**environment,'CREXX_QA_AFFECTED_TESTS':json.dumps(list(reversed(restored)),indent=2)}
        with patch.dict(os.environ,pretty):
            direct=P.canonical_input(a,'daily')
            self.assertEqual(direct,P.canonical_input(a,'daily',{**environment,'CREXX_QA_AFFECTED_TESTS':json.dumps(restored)}))
            cli=command(sys.executable,ROOT/'tools/sanitizer-policy.py','key','--build-dir',a,'--profile','daily').stdout.strip()
            self.assertEqual(cli,direct)
            output=self.root/'inventory.json'
            with patch.object(P,'ctest_inventory',return_value=({'tests':[{'name':'future_test','properties':[]}]},{'future_test'})), patch.object(P,'MEMORY_TESTS',frozenset({'future_test'})), patch.object(P,'MEMORY_REGRESSIONS',{'synthetic':['future_test']}), patch.object(P,'MEMORY_REQUIRED',frozenset({'future_test'})):
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
if operation=='include': raise SystemExit(0)
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
        command('cmake','-S',source,'-B',build,'-G','Ninja','-DSEMANTIC_REGEX=OFF')
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
