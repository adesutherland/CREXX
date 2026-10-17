#!/usr/bin/env python3
"""Build each research bytecode closure with the other frontend absent.

Uses existing native host tools and Level L products. No source-language
implementation or native compiler fallback is provided by this harness.
"""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

p = argparse.ArgumentParser()
p.add_argument('--build', type=Path, required=True)
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
host = a.build.resolve()
with tempfile.TemporaryDirectory(prefix='crexx-closure-') as temporary:
    work = Path(temporary)
    for language, owner in [('c', 'levelc'), ('g', 'typed')]:
        source = work/language/'source'; build = work/language/'build'
        compilers = source/'research/level-l/compilers'
        compilers.mkdir(parents=True)
        original = root/'research/level-l/compilers'
        shutil.copyfile(original/'CMakeLists.txt',compilers/'CMakeLists.txt')
        for part in set(['common', owner, 'level'+language]):
            shutil.copytree(original/part,compilers/part)
        (source/'langtools/cmake').mkdir(parents=True)
        shutil.copyfile(root/'langtools/cmake/generate.cmake',source/'langtools/cmake/generate.cmake')
        (build/'bin').mkdir(parents=True)
        for name in ['library.rxbin','rxfnsl.rxbin','rxfnsc.rxbin','rxfs.rxplugin']:
            shutil.copyfile(host/'bin'/name,build/'bin'/name)
        shutil.copytree(host/'langtools/common',build/'langtools/common',ignore=shutil.ignore_patterns('*-imports'))
        definitions = []
        for tool in ['rxc','rxas','rxlink','rxcpack','rxlang']:
            executable = host/'bin'/tool
            definitions += [f'add_executable({tool} IMPORTED GLOBAL)',f'set_target_properties({tool} PROPERTIES IMPORTED_LOCATION "{executable}")']
        (source/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.24)
project(IndependentResearchCompiler C)
function(crexx_schedule_directory_qa_tier_finalization)
endfunction()
function(crexx_register_test_prep_targets)
endfunction()
function(configure_linker_for_static_lib)
endfunction()
add_library(rxvml INTERFACE)
add_library(rxpashim INTERFACE)
''' + f'include("{root}/cmake/CrexxImportRoots.cmake")\nset(CREXX_RESEARCH_LANGUAGES {language})\n' + '\n'.join(definitions) + '\nadd_subdirectory(research/level-l/compilers)\n')
        def run(command):
            result = subprocess.run([str(x) for x in command],capture_output=True,timeout=600)
            assert result.returncode == 0, result.stdout[-2000:]+result.stderr[-2000:]
            return result
        run(['cmake','-S',source,'-B',build,'-G','Ninja'])
        run(['cmake','--build',build,'--target',f'll{language}compiler',f'llcompiler_llrxc-{language}_opt','--parallel','4'])
        runtime = work/language/'runtime'; runtime.mkdir()
        for name in ['library','rxfnsc']:
            shutil.copyfile(host/'bin'/(name+'.rxbin'),runtime/(name+'.rxbin'))
        if language=='c':
            shutil.copyfile(host/'research/level-l/compilers/products/llctarget.rxbin',runtime/'llctarget.rxbin')
        code = work/language/'hello.crexx'; code.write_text('options level'+language+'\nsay 2+3\n')
        asm = work/language/'hello.rxas'
        driver = build/f'research/level-l/compilers/backend/opt/llrxc-{language}-linked.rxbin'
        run([host/'bin/rxbvm',driver,'-a','-i',runtime,'-o',asm,code])
        run([host/'bin/rxas','-n',asm])
        modules = [asm.with_suffix('.rxbin'),runtime/'library.rxbin']
        if language=='c': modules += [runtime/'llctarget.rxbin',runtime/'rxfnsc.rxbin']
        run([host/'bin/rxlink','-o',work/language/'linked',*modules])
        result = run([host/'bin/rxbvm',work/language/'linked.rxbin'])
        assert result.stdout == b'5\n',result.stdout
        print(language.upper()+' isolated source/build/execute: PASS')
