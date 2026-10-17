#!/usr/bin/env python3
"""STEP07A orchestration only: compilation, metadata and emission stay Level B."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time

p = argparse.ArgumentParser()
p.add_argument('--build', type=Path, required=True)
p.add_argument('--work', type=Path, required=True)
p.add_argument('--mode', choices=['opt', 'noopt'], default='opt')
p.add_argument('--vm', default='rxbvm')
p.add_argument('--installed', type=Path)
p.add_argument('--native-controls', action='store_true')
a = p.parse_args()
build, work_parent = a.build.resolve(), a.work.resolve()
work_parent.mkdir(parents=True, exist_ok=True)
run_directory = tempfile.TemporaryDirectory(prefix='step07-', dir=work_parent)
work = Path(run_directory.name)
root = Path(__file__).resolve().parent
examples = root.parent/'examples/step07a'
backend = build/'lib/rxfnsl/compilers/backend'/a.mode
bin_dir = (a.installed or build/'bin').resolve()
suffix = '.exe' if os.name == 'nt' else ''
tool = lambda name: bin_dir/(name+suffix)
runtime = work/'runtime'
runtime.mkdir(exist_ok=True)
for name in ['library', 'rxfnsc']:
    shutil.copyfile(bin_dir/(name+'.rxbin'), runtime/(name+'.rxbin'))
shutil.copyfile(bin_dir/'llctarget.rxbin' if a.installed else backend/'llctarget.rxbin', runtime/'llctarget.rxbin')
env = dict(os.environ, PATH=str(work/'empty-path'))
records, shapes = [], {}

def run(label, argv, failure=False):
    argv = [str(x) for x in argv]
    before = time.monotonic()
    result = subprocess.run(argv, cwd=runtime, env=env, capture_output=True, timeout=300)
    (work/(label+'.stdout')).write_bytes(result.stdout)
    (work/(label+'.stderr')).write_bytes(result.stderr)
    assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:', result.stdout+result.stderr), (label, result.stderr[-3000:])
    records.append(dict(label=label, argv=argv, status=result.returncode, seconds=time.monotonic()-before))
    assert (result.returncode != 0 if failure else result.returncode == 0), (label, result.returncode, result.stdout[-2000:], result.stderr[-2000:])
    return result

def compile_source(level, source, out, ir='on', failure=False, label=None, imports=None):
    command = [tool('llrxc-'+level)] if a.installed else [tool(a.vm), backend/f'llrxc-{level}-linked.rxbin', '-a']
    command += ([] if ir == 'on' else ['-n']) + ['-i', imports or runtime, '-o', out, source]
    return run(label or out.stem+'-compile', command, failure)

def execute(level, out, expected, label, provider=None, error=None, assembler='noopt'):
    run(label+'-assemble', [tool('rxas'), *(['-n'] if assembler == 'noopt' else []), out])
    return execute_image(level, out.with_suffix('.rxbin'), expected, label, provider, error)

def execute_image(level, image, expected, label, provider=None, error=None):
    linked = work/(label+'-linked')
    modules = [image, runtime/'library.rxbin']
    if provider: modules.append(provider)
    if level == 'c': modules += [runtime/'llctarget.rxbin', runtime/'rxfnsc.rxbin']
    run(label+'-link', [tool('rxlink'), '-o', linked, *modules])
    result = run(label+'-run', [tool(a.vm), str(linked)+'.rxbin'], error is not None)
    assert result.stdout.replace(b'\r\n', b'\n') == expected, (label, result.stdout, expected)
    if error: assert error in result.stderr, (label, result.stderr)
    else: assert not result.stderr, (label, result.stderr)

def reject(source, message, label, level='g', imports=None):
    out = work/(label+'.rxas'); out.write_bytes(b'preserve this file\n')
    result = compile_source(level, source, out, failure=True, label=label, imports=imports)
    assert message in result.stdout+result.stderr, (label, result.stdout, result.stderr)
    diagnostic = result.stdout+result.stderr
    assert source.name.encode()+b':' in diagnostic and b'line ' in diagnostic and b'column ' in diagnostic and b'^' in diagnostic, (label, diagnostic)
    assert out.read_bytes() == b'preserve this file\n', label

def metadata_image(directory, signature, *, exported=True, duplicate=False):
    directory.mkdir(exist_ok=True)
    expose = ' .expose=scalelib.scale' if exported else ''
    source = '.globals=0\nscale() .locals=1'+expose+'\n  load r0,1\n  ret r0\n'
    source += '  .meta "scalelib.scale"="b" ".int" scale() '+json.dumps(signature)+'\n'
    if duplicate: source += '  .meta "scalelib.scale"="b" ".int" scale() '+json.dumps(signature)+'\n'
    asm = directory/'scalelib.rxas'; asm.write_text(source)
    run(directory.name+'-assemble', [tool('rxas'), '-n', asm])

try:
    for level in ['g', 'c']:
        for name in ['scale', 'presence', 'calls']:
            srcdir = root/'step07a' if name == 'calls' else examples
            source = srcdir/f'{name}-{level}.crexx'
            expected = (srcdir/f'{name}-{level}.expected').read_bytes()
            for ir in ['off', 'on']:
                label = f'{name}-{level}-{ir}'; asm = work/(label+'.rxas')
                compile_source(level, source, asm, ir)
                text = asm.read_text()
                assert str(source.parent.resolve()) not in text
                shapes[label] = dict(bytes=len(text.encode()), calls=text.count('\ncall '), frames=re.findall(r'\.locals=(\d+)', text))
                execute(level, asm, expected, label)
                if name == 'scale' and level == 'g':
                    # Literal appears only in the missing branch, and missing callers
                    # clear supplied status without constructing an empty/default value.
                    assert re.search(r'brtpandt (llarg\d+)_present,a2,256\nload r\d+,2\n', text), text
                    assert ',512\n' in text and not re.search(r'^load r\d+,""$', text, re.M)
                    execute(level, asm, expected, label+'-asopt', assembler='opt')
    for ir in ['off', 'on']:
        asm = work/f'conversion-{ir}.rxas'
        compile_source('g', root/'step07a/conversion-g.crexx', asm, ir)
        execute('g', asm, b'2\n', 'conversion-'+ir, error=b'CONVERSION')
        assert b'conversion-g.crexx:3:12:' in (work/('conversion-'+ir+'-run.stderr')).read_bytes()
    # Provider emits no main and preserves optional flags through round trips.
    provider = runtime/'scalelib.rxas'
    compile_source('g', examples/'scalelib.crexx', provider, label='provider-compile')
    text = provider.read_text()
    assert 'main()' not in text and '.expose=scalelib.scale' in text and '?p2=.int' in text
    run('provider-assemble', [tool('rxas'), '-n', provider])
    consumer = work/'consumer.rxas'
    compile_source('g', examples/'import-g.crexx', consumer, label='consumer-compile')
    execute('g', consumer, b'14\n21\n', 'modules', provider.with_suffix('.rxbin'))
    frozen = hashlib.sha256(consumer.with_suffix('.rxbin').read_bytes()).hexdigest()
    changed = work/'changed-provider.crexx'
    changed.write_bytes((examples/'scalelib.crexx').read_bytes().replace(b'factor = 2', b'factor = 4'))
    compile_source('g', changed, provider, label='changed-provider-compile')
    run('changed-provider-assemble', [tool('rxas'), '-n', provider])
    execute_image('g', consumer.with_suffix('.rxbin'), b'28\n21\n', 'changed-provider', provider.with_suffix('.rxbin'))
    assert frozen == hashlib.sha256(consumer.with_suffix('.rxbin').read_bytes()).hexdigest()
    primitives = runtime/'primitives.rxas'
    compile_source('g', root/'step07a/primitives.crexx', primitives, label='primitives-provider')
    run('primitives-assemble', [tool('rxas'), '-n', primitives])
    primitive_user = work/'primitives-use.rxas'
    compile_source('g', root/'step07a/primitives-use.crexx', primitive_user, label='primitives-consumer')
    execute('g', primitive_user, b'0\n1\nfallback\n[]\n-0.25\n0.125\nempty\n', 'primitives', primitives.with_suffix('.rxbin'))
    # Service and CLI must agree, with generic IR save/load retaining the contract.
    if not a.installed:
        result = run('service-views', [tool(a.vm), backend/'backend-step07-linked.rxbin', '-a', 'g', examples/'scale-g.crexx', runtime, work/'service'])
        assert b'failures=0' in result.stdout
        assert (work/'service.rxas').read_bytes() == (work/'scale-g-on.rxas').read_bytes()
    # Unknown/private/ambiguous/malformed selected signatures fail atomically.
    for label, signature, expected, kwargs in [
        ('required', 'x=.int,y=.int', b'missing required', {}),
        ('duplicate', 'x=.int,X=.int', b'duplicate formal', {}),
        ('wrongtype', 'x=.object,?y=.int', b'unsupported scalar', {}),
        ('badliteral', 'x=1+2,?y=.int', b'invalid argument', {}),
        ('private', 'x=.int,?y=.int', b'not exported', {'exported':False}),
        ('ambiguous', 'x=.int,?y=.int', b'ambiguous callable', {'duplicate':True}),
    ]:
        directory = work/label
        metadata_image(directory, signature, **kwargs)
        reject(examples/'import-g.crexx', expected, label+'-reject', imports=directory)
    directory = work/'truncated'; directory.mkdir(exist_ok=True)
    (directory/'scalelib.rxbin').write_bytes(b'cReXx007')
    reject(examples/'import-g.crexx', b'RXBIN007', 'truncated-reject', imports=directory)
    reject(examples/'missing-g.crexx', b'required', 'required-source-reject')
    # Authored C metadata isolates optionality/context mismatches independently
    # of the real runtime package. These files are never executed.
    for label, argspec, context in [('c-optional','?context=reference .rexxbifcallcontext','.int'), ('c-context','context=reference .wrong','.int'), ('c-layout','context=reference .rexxbifcallcontext','.string')]:
        directory = work/label; directory.mkdir(exist_ok=True)
        asm = directory/'rxfnsc.rxas'
        asm.write_text('.globals=0\n.meta "rexxclassicbifs.rexxbifcallcontext"="b" ".unknown" .class\n.meta "rexxclassicbifs.rexxbifcallcontext.bif_arg_count"="b" "'+context+'" .attr 1\nf() .locals=1 .expose=rexxclassicbiflength.rexxclassicbif_length\nret r0\n.meta "rexxclassicbiflength.rexxclassicbif_length"="b" ".rexxvalue..rexxvalue" f() "'+argspec+'"\n')
        run(label+'-assemble',[tool('rxas'),'-n',asm])
        source = directory/'bad-c.crexx'; source.write_text("options levelc\nsay length('x')\n")
        reject(source,b'incompatible',label+'-reject','c',directory)
    if a.native_controls:
        # Both directions are independent compilations; the new host has no
        # native compiler available on PATH during any source request.
        native = work/'native'; native.mkdir(exist_ok=True)
        run('native-provider', [tool('rxc'), '-n', '-x', '--no-exe-import', '-i', runtime, '-o', native/'scalelib', examples/'scalelib.crexx'])
        run('native-provider-assemble', [tool('rxas'), '-n', native/'scalelib'])
        compile_source('g', examples/'import-g.crexx', work/'native-provider-consumer.rxas', label='new-imports-native', imports=native)
        execute('g',work/'native-provider-consumer.rxas',b'14\n21\n','new-with-native',native/'scalelib.rxbin')
        run('native-consumer', [tool('rxc'), '-n', '-x', '--no-exe-import', '-i', runtime, '-o', work/'native-consumer', examples/'import-g.crexx'])
        execute('g',work/'native-consumer.rxas',b'28\n21\n','native-with-new',provider.with_suffix('.rxbin'))
    print(f'STEP07_E2E|commands={len(records)}|failures=0')
finally:
    run_directory.cleanup()
