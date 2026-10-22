#!/usr/bin/env python3
"""Focused real CLI/cache transport controls; all products stay temporary.

The service fixture proves actual hits. A transparent host shim observes only
wrapper arguments, then runs the real generator; it implements no lexer/cache.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import time


parser = argparse.ArgumentParser()
parser.add_argument('--bin', type=Path, required=True)
parser.add_argument('--cmake', default='cmake')
parser.add_argument('--service-rxas', type=Path,
                    help='also inspect the prepared service export and buffer shapes')
args = parser.parse_args()
binary = args.bin.resolve()
tool = binary / ('rxlang.exe' if os.name == 'nt' else 'rxlang')
wrapper = Path(__file__).resolve().parents[1] / 'cmake/generate.cmake'
commands = 0
started = time.monotonic()
BASE = """options levell
namespace reuse_cli
name: token = 1
scan: lexer input utf8
  letter: charset = 'a' to 'z' | U+03BB
  word: rule = letter+ then emit name
  spaces: rule = (' ' | U+000A)+ then skip
parse: parser start root backend lr1
  root: rule = name
"""
CHANGED = BASE.replace('root: rule = name', 'root: rule = name+')


def run(command, *, success=True):
    global commands
    commands += 1
    result = subprocess.run(list(map(str, command)), capture_output=True, timeout=600)
    if success:
        assert result.returncode == 0, (command, result.returncode,
                                        result.stdout[-2000:], result.stderr[-2000:])
    else:
        assert result.returncode != 0 and result.stdout + result.stderr, (command, result)
        assert b'.globals=' not in result.stdout and b'options levelb' not in result.stdout, (
            'failed transport published generated output', command, result.stdout[-2000:])
    return result


def service_shape(path):
    """Read real compiler output without pinning register or label numbers."""
    source = path.read_text(encoding='utf-8')
    headers = list(re.finditer(r'^(\S+)\(\)\s+\.locals=\d+[^\n]*$', source, re.M))
    bodies = {match[1].lstrip('§'): source[match.end():
              headers[index + 1].start() if index + 1 < len(headers) else len(source)]
              for index, match in enumerate(headers)}
    exports = set(re.findall(r'^\S+\(\)[^\n]*\.expose=(llgenerate\.\S+)', source, re.M))
    # Exposing request historically exposes its existing implementation methods.
    # The new capsule machinery must stay private; only these two queries are new.
    methods = '''§factory run status error output lexical reused fail line generate
        quoted sourceoperators contextmask emitdata assembly asmoperators asmimport
        asmload asmcall asmquote servicesource serviceassembly asmclass asmheader
        asmwrap asmdefault'''.split()
    assert exports == {'llgenerate.request.' + name for name in methods}, exports
    signature = ('source=.binary,?format=.string,?filename=.string,'
                 '?limits=.lllimits..policy,?rxl=.int,?bytes=.int,?lexical=.binary')
    assert f'"{signature}"' in bodies['llgenerate.request.§factory']

    def executable(name):
        return [line.strip() for line in bodies['llgenerate.lexicalbuffer.' + name].splitlines()
                if line.strip() and not line.lstrip().startswith(('.', '/*', '*'))]

    integer, put, ensure = map(executable, ['integer', 'put', 'ensure'])
    forbidden = re.compile(r'^(?:bappend|concat|concatc|bresize)\b')
    assert not any(forbidden.match(line) for line in integer + put)
    writes = [index for index, line in enumerate(integer) if line.startswith('bseti32 ')]
    assert len(writes) == 1
    write = writes[0]
    destination = integer[write].split()[1].split(',')[0]
    data_attr = re.search(r'^\.meta "llgenerate\.lexicalbuffer\._data"[^\n]*\.attr (\d+)$',
                          source, re.M)[1]
    assert any(line == f'linkattr1 {destination},a1,{data_attr}' for line in integer[:write])
    assert any(line.startswith(('unlink ', 'unlinkn ')) and
               destination in line.split(' ', 1)[1].split(',') for line in integer[write + 1:])
    assert sum('rxfnsb.bincopy()' in line for line in put) == 1
    assert any('llgenerate.lexicalbuffer.ensure()' in line for line in put)
    resizes = [index for index, line in enumerate(ensure) if line.startswith('bresize ')]
    assert len(resizes) == 1
    resize = resizes[0]
    # A false growth test jumps over the sole resize; doubling remains in that arm.
    assert any(line.startswith('brf ') and
               line.split()[1].split(',')[0] + ':' in ensure[resize + 1:]
               for line in ensure[:resize])
    assert any(re.match(r'imult [^,]+,[^,]+,2$', line) for line in ensure[:resize])
    print('LEXICAL_REUSE_SHAPE|exports=26|linked_integer=1|growth_resize=1|span_copy=1', flush=True)


if args.service_rxas:
    service_shape(args.service_rxas)


with tempfile.TemporaryDirectory(prefix='crexx-lexical-reuse-') as temporary:
    work = Path(temporary) / 'paths with spaces'
    work.mkdir()
    spec = work / 'language.rxl'
    spec.write_text(BASE, encoding='utf-8')
    capsule = work / 'input capsule.bin'
    original = run([tool, spec, '--rxas']).stdout
    cold = run([tool, spec, '--rxas', '--lexer-out', capsule])
    assert cold.stdout == original and capsule.stat().st_size > 0
    saved_capsule = capsule.read_bytes()
    spec.write_text(CHANGED, encoding='utf-8')
    current = run([tool, spec, '--rxas']).stdout
    warm = run([tool, spec, '--rxas', '--lexer-in', capsule])
    assert warm.stdout == current and warm.stdout != original, 'parser output was reused wholesale'
    plain = run([tool, spec]).stdout
    assert run([tool, spec, '--lexer-in', capsule]).stdout == plain
    assert capsule.read_bytes() == saved_capsule

    # Successful exclusive output is complete before generated stdout appears.
    invalid = work / 'invalid.rxl'
    invalid.write_text('not a specification', encoding='utf-8')
    absent_output = work / 'must remain absent.bin'
    run([tool, invalid, '--rxas', '--lexer-out', absent_output], success=False)
    assert not absent_output.exists()
    run([tool, spec, '--graph', '--lexer-in', capsule], success=False)
    run([tool, spec, '--lexer-in', capsule, '--lexer-in', capsule], success=False)
    run([tool, spec, '--lexer-out', absent_output, '--lexer-out', absent_output], success=False)
    for stream_name in ['stdin', 'stdout', 'stderr']:
        for option in ['--lexer-in', '--lexer-out']:
            run([tool, spec, '--rxas', option, stream_name], success=False)
    run([tool, spec, '--lexer-in', work / 'missing.bin'], success=False)
    run([tool, spec, '--lexer-out', work / 'missing directory/output.bin'], success=False)
    source_bytes = spec.read_bytes()
    run([tool, spec, '--rxas', '--lexer-out', spec], success=False)
    run([tool, spec, '--rxas', '--lexer-in', spec], success=False)
    run([tool, spec, '--rxas', '--lexer-in', capsule, '--lexer-out', capsule], success=False)
    run([tool, spec, '--rxas', '--lexer-out', capsule], success=False)
    assert spec.read_bytes() == source_bytes and capsule.read_bytes() == saved_capsule
    for kind in ['symlink', 'hardlink']:
        alias = work / f'{kind} source alias.bin'
        try:
            if kind == 'symlink':
                alias.symlink_to(spec)
            else:
                os.link(spec, alias)
        except (OSError, NotImplementedError):
            print(f'LEXICAL_REUSE_CAPABILITY|{kind}=unavailable', flush=True)
            continue
        run([tool, spec, '--rxas', '--lexer-out', alias], success=False)
        run([tool, spec, '--rxas', '--lexer-in', alias], success=False)
        assert spec.read_bytes() == source_bytes
    oversize = work / 'oversize.bin'
    with oversize.open('wb') as stream:
        stream.truncate(67108865)
    assert run([tool, spec, '--rxas', '--lexer-in', oversize]).stdout == current

    # The VM+image adapter form also provides portable shim execution on Windows.
    events = work / 'arguments.jsonl'
    shim = work / 'generator shim.py'
    shim.write_text(
        'import json, subprocess, sys\n'
        f'event_path = {str(events)!r}\n'
        f'tool = {str(tool)!r}\n'
        "argv = sys.argv[1:]\n"
        "if argv and argv[0] == '-a': argv = argv[1:]\n"
        "with open(event_path, 'a', encoding='utf-8') as out: out.write(json.dumps(argv)+'\\n')\n"
        "raise SystemExit(subprocess.call([tool, *argv]))\n", encoding='utf-8')
    output = work / 'generated.rxas'
    cache = Path(str(output) + '.lexer')
    manifest = Path(str(output) + '.lexer.manifest')

    def wrapper_command():
        return [args.cmake, f'-DLEVEL_L_VM={sys.executable}', f'-DLEVEL_L_BYTECODE={shim}',
                f'-DSPEC={spec}', f'-DOUTPUT={output}', '-DFORMAT=rxas',
                '-DREUSE_LEXER=ON', '-P', wrapper]

    def observed():
        return [json.loads(line) for line in events.read_text(encoding='utf-8').splitlines()]

    def wrapped(*, expect_in):
        before = len(observed()) if events.exists() else 0
        run(wrapper_command())
        rows = observed()
        assert len(rows) == before + 1
        assert ('--lexer-in' in rows[-1]) == expect_in, rows[-1]
        assert '--lexer-out' in rows[-1] and output.read_bytes() == current
        assert cache.exists() and manifest.exists()

    wrapped(expect_in=False)
    cache_bytes, manifest_bytes = cache.read_bytes(), manifest.read_bytes()
    stamp = cache.stat().st_mtime_ns
    wrapped(expect_in=True)
    assert cache.read_bytes() == cache_bytes and cache.stat().st_mtime_ns == stamp
    assert manifest.read_bytes() == manifest_bytes
    cache.write_bytes(cache_bytes + b'corrupt')
    wrapped(expect_in=False)
    assert cache.read_bytes() == cache_bytes
    manifest.write_text('malformed manifest\n', encoding='utf-8')
    wrapped(expect_in=False)
    manifest.unlink()
    wrapped(expect_in=False)
    cache.unlink()
    wrapped(expect_in=False)
    # Actual launch-image bytes, rather than timestamps/manual revision, bind reuse.
    old_time = shim.stat().st_mtime_ns
    with shim.open('a', encoding='utf-8') as stream:
        stream.write('# same behavior, changed generator image identity\n')
    os.utime(shim, ns=(old_time, old_time))
    wrapped(expect_in=False)
    wrapped(expect_in=True)
    prior = (output.read_bytes(), cache.read_bytes(), manifest.read_bytes())
    spec.write_text('not a specification', encoding='utf-8')
    run(wrapper_command(), success=False)
    assert prior == (output.read_bytes(), cache.read_bytes(), manifest.read_bytes())
    spec.write_text(CHANGED, encoding='utf-8')
    before = len(observed())
    with ThreadPoolExecutor(max_workers=2) as pool:
        results = list(pool.map(lambda _: subprocess.run(list(map(str, wrapper_command())),
                            capture_output=True, timeout=600), range(2)))
    commands += 2
    assert all(result.returncode == 0 for result in results), results
    assert len(observed()) == before + 2 and output.read_bytes() == current
    assert (cache.read_bytes(), manifest.read_bytes()) == prior[1:]
    assert not list(work.glob('generated.rxas.stage-*')), 'owned staging was retained'

    # One real custom producer: no-op builds and missing advisory caches invoke no tool.
    project = work / 'build project'
    project.mkdir()
    declarations = '\n'.join([
        'cmake_minimum_required(VERSION 3.24)', 'project(lexical_reuse NONE)',
        'add_custom_command(OUTPUT "${PRODUCT}"',
        ' COMMAND "${CMAKE_COMMAND}" "-DLEVEL_L_VM=${HOST_VM}" "-DLEVEL_L_BYTECODE=${HOST_IMAGE}"',
        ' "-DSPEC=${SPEC}" "-DOUTPUT=${PRODUCT}" -DFORMAT=rxas -DREUSE_LEXER=ON -P "${WRAPPER}"',
        ' DEPENDS "${SPEC}" "${HOST_VM}" "${HOST_IMAGE}" "${WRAPPER}" VERBATIM)',
        'add_custom_target(generate ALL DEPENDS "${PRODUCT}")', ''])
    (project / 'CMakeLists.txt').write_text(declarations, encoding='utf-8')
    build = work / 'build'
    product = work / 'build-generated.rxas'
    run([args.cmake, '-S', project, '-B', build, f'-DPRODUCT={product}',
         f'-DSPEC={spec}', f'-DHOST_VM={sys.executable}', f'-DHOST_IMAGE={shim}',
         f'-DWRAPPER={wrapper}'])
    run([args.cmake, '--build', build, '--parallel', '2'])
    before = len(observed())
    run([args.cmake, '--build', build, '--parallel', '2'])
    assert len(observed()) == before
    Path(str(product) + '.lexer').unlink()
    Path(str(product) + '.lexer.manifest').unlink()
    run([args.cmake, '--build', build, '--parallel', '2'])
    assert len(observed()) == before, 'missing optional cache forced a product rebuild'

print(f'LEXICAL_REUSE_CLI|commands={commands}|failures=0|seconds={time.monotonic()-started:.3f}')
