#!/usr/bin/env python3
"""Isolated STEP04 toolchain regression orchestration; no compiler implementation.

Use prebuilt B-only compiler images. Native compilers are only optional,
independent controls; no source request can fall back to them. Diagnostics and
command identities are temporary and removed at the end of each invocation.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('--build', type=Path, required=True)
parser.add_argument('--work', type=Path, required=True)
parser.add_argument('--mode', choices=['opt', 'noopt'], default='noopt')
parser.add_argument('--vm', default='rxbvm')
parser.add_argument('--native-controls', action='store_true')
parser.add_argument('--installed', type=Path)
parser.add_argument('--case', action='append', help='focused name-level case; skips unrelated controls and uses assembler -n')
parser.add_argument('--ir', choices=('on', 'off'), default='on', help='research compiler IR optimisation')
parser.add_argument('--inspect', type=Path, help='copy selected readable RXAS to a temporary review directory')
args = parser.parse_args()
source_dir = Path(__file__).resolve().parent / 'backend'
cases = json.loads((source_dir / 'cases.json').read_text(encoding='utf-8'))
if args.case:
    requested = set(args.case)
    unknown = requested - {case['name'] + '-' + case['level'] for case in cases}
    if unknown:
        parser.error('unknown or empty case selection: ' + repr(sorted(unknown)))
    cases = [case for case in cases if case['name'] + '-' + case['level'] in requested]
if args.native_controls and args.ir != 'on':
    parser.error('--ir applies only to the research compiler')
if args.inspect and not args.case:
    parser.error('--inspect requires a focused --case selection')
build = args.build.resolve()
args.work.mkdir(parents=True, exist_ok=True)
run_directory = tempfile.TemporaryDirectory(prefix='check-',dir=args.work)
work = Path(run_directory.name)
bin_dir = (args.installed or build / 'bin').resolve()
ext = '.exe' if os.name == 'nt' else ''
tool = lambda name: str(bin_dir / (name + ext))
backend = build / 'research/level-l/compilers/backend' / args.mode
records = []
start = time.monotonic()
# Isolated target-contract directory. Host executables cannot discover native
# compilers through PATH, and do not receive their path as an argument.
runtime = work / 'runtime'
runtime.mkdir(exist_ok=True)
for name in ['library', 'rxfnsc', 'classlib', 'rxunicode-core']:
    shutil.copyfile(bin_dir / (name + '.rxbin'), runtime / (name + '.rxbin'))
shutil.copyfile((bin_dir / 'llctarget.rxbin') if args.installed else backend / 'llctarget.rxbin', runtime / 'llctarget.rxbin')
env = dict(os.environ)
env['PATH'] = str(work / 'empty-path')

def run(label, command, success=True, isolated=False):
    before = time.monotonic()
    # The contracts invocation runs the combined source/graph/shape fixture.
    # Give that serial nested workload a generous hang guard of its own.
    limit = 1800 if label == 'contracts' else 180
    result = subprocess.run(command, cwd=work, env=env if isolated else None,
                            capture_output=True, timeout=limit)
    (work / (label + '.stdout')).write_bytes(result.stdout)
    (work / (label + '.stderr')).write_bytes(result.stderr)
    records.append(dict(label=label, command=command, status=result.returncode,
                        seconds=round(time.monotonic()-before, 3)))
    if success:
        assert result.returncode == 0, (label, result.returncode,
                                       result.stdout[-3000:], result.stderr[-3000:])
    return result

def compiler(level, destination, source):
    if args.installed:
        return [tool('llrxc-'+level), *(['-n'] if args.ir == 'off' else []), '-i', str(runtime), '-o', str(destination), str(source)]
    return [tool(args.vm), str(backend / ('llrxc-'+level+'-linked.rxbin')), '-a',
            *(['-n'] if args.ir == 'off' else []), '-i', str(runtime), '-o', str(destination), str(source)]

try:
    if not args.native_controls and not args.installed and not args.case:
        exact = work/'exact.rxas'
        result = run('contracts', [tool(args.vm), str(backend/'backend-tests-linked.rxbin'), '-a', str(runtime), str(exact)], isolated=True)
        assert b'failures=0' in result.stdout, result.stdout
        for mode in ['noopt', 'opt']:
            run('exact-'+mode+'-assemble', [tool('rxas')] + (['-n'] if mode=='noopt' else []) + [str(exact)])
            run('exact-'+mode+'-link', [tool('rxlink'), '-o', str(work/'exact-linked'), str(work/'exact.rxbin'), str(runtime/'library.rxbin')])
            result = run('exact-'+mode+'-run', [tool(args.vm), str(work/'exact-linked.rxbin')])
            assert result.stdout.replace(b'\r\n', b'\n') == b'A\x00B"\\\xce\xb1\n-inf\n', result.stdout
    for case in cases:
        stem = case['name'] + '-' + case['level']
        source = (source_dir / case['source']).resolve()
        assembly = work / (stem + '.rxas')
        if args.native_controls:
            # Controls are independently built/executed and never consulted by
            # the new compiler. Known native admission differences are explicit.
            if stem == 'loops-g':
                continue
            command = [tool('rxc-c' if case['level']=='c' else 'rxc'), '-n',
                       '-i', str(bin_dir), '-o', str(assembly), str(source)]
        else:
            command = compiler(case['level'], assembly, source)
        run(stem+'-compile', command, isolated=not args.native_controls)
        assert assembly.exists(), stem
        if args.inspect:
            args.inspect.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(assembly, args.inspect / f'{stem}-{args.mode}-{args.ir}-{Path(args.vm).name}.rxas')
        # Both assembler modes exercise generated control/call windows. Shared
        # IR optimisation is on; check-optimise.py covers the compiler -n control.
        for assembler_mode in (['noopt'] if args.native_controls or args.case else ['noopt', 'opt']):
            label = stem + '-' + assembler_mode
            command = [tool('rxas')] + (['-n'] if assembler_mode=='noopt' else []) + [str(assembly)]
            run(label+'-assemble', command)
            linked = work / (label+'-linked')
            modules = [str(assembly.with_suffix('.rxbin')), str(runtime/'library.rxbin')]
            if case['level']=='c':
                modules += [str(runtime/'llctarget.rxbin'), str(runtime/'rxfnsc.rxbin'),
                            str(runtime/'classlib.rxbin'), str(runtime/'rxunicode-core.rxbin')]
            # Preserve source metadata; -s intentionally strips runtime sources.
            run(label+'-link', [tool('rxlink'), '-o', str(linked)] + modules)
            command = [tool(args.vm), str(linked)+'.rxbin']
            if case['args']:
                command += ['-a'] + case['args']
            result = run(label+'-run', command, success=not case['error'])
            stdout = result.stdout.replace(b'\r\n', b'\n')
            stderr = result.stderr.decode(errors='replace')
            assert stdout == case['output'].encode(), (label, stdout, case['output'])
            if case['error']:
                assert result.returncode != 0 and case['error'].lower() in stderr.lower(), (label, stderr)
                assert source.name in stderr, (label, 'missing authored source', stderr)
                if 'error_line' in case:
                    assert f"{source.name}:{case['error_line']}:" in stderr, (label, 'wrong authored fault line', stderr)
                if case['name']=='origin-error':
                    assert ':3:' in stderr, (label, 'wrong CRLF source line', stderr)
            else:
                assert not stderr, (label, stderr)
        print(stem+' PASS', flush=True)
    if not args.native_controls and not args.case:
        # Every failing command preserves a pre-existing destination, including
        # incomplete source, unsupported features, missing/forged contracts,
        # staging collisions and rename failures. No user-owned file is deleted.
        source = work/'bad.crexx'
        destination = work/'preserve.rxas'
        for label, level, content, expected in [
            ('syntax','g','if 1 say 2','LL_COMPILER_SYNTAX'),
            ('type','g','x = .unknown','LL_COMPILER_TYPE'),
            ('profile','c','interpret "say 1"','LL_COMPILER_PROFILE'),
            ('utf8','g',b'say \xff','LL_UTF8')]:
            source.write_bytes(content if isinstance(content,bytes) else content.encode())
            destination.write_bytes(b'previous output')
            result=run('failure-'+label, compiler(level,destination,source), False, True)
            assert result.returncode and expected.encode() in result.stdout, (label,result.stdout,result.stderr)
            assert destination.read_bytes()==b'previous output'
            assert not Path(str(destination)+'.llrxc-stage').exists()
        source.write_text('say 1\n')
        stage=Path(str(destination)+'.llrxc-stage');stage.mkdir(exist_ok=True)
        (stage/'owned-by-user').write_text('keep')
        result=run('failure-stage',compiler('g',destination,source),False,True)
        assert result.returncode and b'LL_COMPILER_OUTPUT' in result.stdout
        assert destination.read_bytes()==b'previous output' and (stage/'owned-by-user').read_text()=='keep'
        (stage/'owned-by-user').unlink();stage.rmdir()
        directory=work/'destination-directory';directory.mkdir(exist_ok=True)
        result=run('failure-rename',compiler('g',directory,source),False,True)
        assert result.returncode and b'LL_COMPILER_OUTPUT' in result.stdout and directory.is_dir()
        assert not Path(str(directory)+'.llrxc-stage').exists()
        result=run('failure-input',compiler('g',destination,work/'does-not-exist.crexx'),False,True)
        assert result.returncode and b'LL_COMPILER_INPUT' in result.stdout
        assert destination.read_bytes()==b'previous output'
        # A foreign RXBIN has a valid envelope but cannot supply the adapter ABI.
        shutil.copyfile(runtime/'library.rxbin',runtime/'llctarget.rxbin')
        result=run('failure-contract',compiler('c',destination,source),False,True)
        assert result.returncode and b'LL_COMPILER_IMPORT' in result.stdout
        assert destination.read_bytes()==b'previous output'
    print('BACKEND_E2E|cases='+str(len(cases))+'|commands='+str(len(records))+'|failures=0')
finally:
    run_directory.cleanup()
