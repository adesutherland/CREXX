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
p.add_argument('--c-only', action='store_true',
               help='check existing C calls in restricted compiler-host and static-target roots')
a = p.parse_args()
if a.c_only and (a.installed or a.native_controls):
    p.error('--c-only uses build-linked compiler images; installed/native controls remain mixed-mode checks')
if a.c_only and a.vm not in ('rxbvm', 'rxtvm'):
    p.error('--c-only requires an unbundled rxbvm or rxtvm')
build, work_parent = a.build.resolve(), a.work.resolve()
work_parent.mkdir(parents=True, exist_ok=True)
run_directory = tempfile.TemporaryDirectory(prefix='step07-', dir=work_parent)
work = Path(run_directory.name)
root = Path(__file__).resolve().parent
examples = root.parent/'examples/step07a'
backend = build/'research/level-l/compilers/backend'/a.mode
bin_dir = (a.installed or build/'bin').resolve()
suffix = '.exe' if os.name == 'nt' else ''
tool = lambda name: bin_dir/(name+suffix)
runtime = work/'runtime'
runtime.mkdir(exist_ok=True)
for name in ['library', 'rxfnsc', 'classlib', 'rxunicode-core']:
    shutil.copyfile(bin_dir/(name+'.rxbin'), runtime/(name+'.rxbin'))
shutil.copyfile(bin_dir/'llctarget.rxbin' if a.installed else backend/'llctarget.rxbin', runtime/'llctarget.rxbin')
env = dict(os.environ, PATH=str(work/'empty-path'))
records, shapes = [], {}
compiler_vm = tool(a.vm)
compiler_image = backend/'llrxc-c-linked.rxbin'
compiler_directory = runtime
target_vm = compiler_vm
target_directory = runtime
vm_options = []
runtime_dlls = tuple(sorted(bin_dir.glob('*.dll'))) if os.name == 'nt' else ()
dll_names = {path.name for path in runtime_dlls}


def stage_vm(directory, providers):
    directory.mkdir()
    shutil.copy2(tool(a.vm), directory/(a.vm+suffix))
    for path in runtime_dlls:
        shutil.copy2(path, directory/path.name)
    provider_directory = directory/'providers'
    provider_directory.mkdir()
    for provider in providers:
        name = provider+'.rxplugin'
        shutil.copy2(bin_dir/name, provider_directory/name)
    assert {path.name for path in provider_directory.iterdir()} == {
        provider+'.rxplugin' for provider in providers}
    return directory/(a.vm+suffix)


if a.c_only:
    # VM lookup includes the executable directory: PATH alone cannot exclude
    # sibling compilers or ambient bytecode from the shared build bin/.
    (work/'empty-path').mkdir()
    env.pop('CREXX_PROVIDER_PATH', None)
    compiler_directory = work/'compiler host'
    compiler_vm = stage_vm(compiler_directory, ['rxfs'])
    compiler_image = compiler_directory/compiler_image.name
    shutil.copyfile(backend/compiler_image.name, compiler_image)
    assert {path.name for path in compiler_directory.iterdir()} == {
        a.vm+suffix, compiler_image.name, 'providers', *dll_names}
    target_directory = work/'static target'
    target_vm = stage_vm(target_directory, ['rxfs', 'rxcstream'])
    assert {path.name for path in target_directory.iterdir()} == {
        a.vm+suffix, 'providers', *dll_names}
    assert {path.name for path in runtime.iterdir()} == {
        'library.rxbin', 'classlib.rxbin', 'rxfnsc.rxbin', 'rxunicode-core.rxbin',
        'llctarget.rxbin'}
    for compiler in ('rxc', 'rxc-c'):
        assert shutil.which(compiler, path=env['PATH']) is None
    # Target metadata/link inputs are never loaded by the compiler host.
    # RXAS/RXLINK remain explicit file-stage tools outside execution roots.
    vm_options = ['--no-autoload']

def run(label, argv, failure=False, cwd=None):
    argv = [str(x) for x in argv]
    before = time.monotonic()
    result = subprocess.run(argv, cwd=cwd or runtime, env=env, capture_output=True, timeout=300)
    (work/(label+'.stdout')).write_bytes(result.stdout)
    (work/(label+'.stderr')).write_bytes(result.stderr)
    assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:', result.stdout+result.stderr), (label, result.stderr[-3000:])
    records.append(dict(label=label, argv=argv, status=result.returncode, seconds=time.monotonic()-before))
    assert (result.returncode != 0 if failure else result.returncode == 0), (label, result.returncode, result.stdout[-2000:], result.stderr[-2000:])
    return result

def compile_source(level, source, out, ir='on', failure=False, label=None, imports=None):
    if a.c_only:
        assert level == 'c'
        command = [compiler_vm, *vm_options, compiler_image, '-a']
    else:
        command = [tool('llrxc-'+level)] if a.installed else [tool(a.vm), backend/f'llrxc-{level}-linked.rxbin', '-a']
    command += ([] if ir == 'on' else ['-n']) + ['-i', imports or runtime, '-o', out, source]
    return run(label or out.stem+'-compile', command, failure, cwd=compiler_directory)

def execute(level, out, expected, label, provider=None, error=None, assembler='noopt'):
    run(label+'-assemble', [tool('rxas'), *(['-n'] if assembler == 'noopt' else []), out])
    return execute_image(level, out.with_suffix('.rxbin'), expected, label, provider, error)

def execute_image(level, image, expected, label, provider=None, error=None):
    linked = work/(label+'-linked')
    modules = [image, runtime/'library.rxbin']
    if provider: modules.append(provider)
    if level == 'c':
        modules += [runtime/'llctarget.rxbin', runtime/'rxfnsc.rxbin',
                    runtime/'classlib.rxbin', runtime/'rxunicode-core.rxbin']
    run(label+'-link', [tool('rxlink'), '-o', linked, *modules])
    image = Path(str(linked)+'.rxbin')
    if a.c_only:
        staged = target_directory/image.name
        shutil.copyfile(image, staged)
        image = staged
    result = run(label+'-run', [target_vm, *vm_options, image], error is not None,
                 cwd=target_directory)
    assert result.stdout.replace(b'\r\n', b'\n') == expected, (label, result.stdout, expected)
    if error: assert error in result.stderr, (label, result.stderr)
    else: assert not result.stderr, (label, result.stderr)

def reject(source, message, label, level='g', imports=None, ir='on'):
    out = work/(label+'.rxas'); out.write_bytes(b'preserve this file\n')
    result = compile_source(level, source, out, ir=ir, failure=True, label=label, imports=imports)
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


def classic_shape(text, name):
    # Pin semantic call identities and packet ABI, not physical registers or
    # a repeated copy/load sequence. Runtime expectations prove presence/order.
    declarations = re.findall(
        r'^\s*\.meta "llprogram\.([^"]+)"="b" "[^"]+" (llf\d+)\(\) "([^"]*)"',
        text, re.M)
    functions = {identity: label for identity, label, _ in declarations}
    expected = {'scale': {'$MAIN', 'SCALE'}, 'presence': {'$MAIN', 'PROBE'},
                'calls': {'$MAIN', 'HOLES', 'RECURSE', 'MARK', 'PAIR'},
                'pipeline': {'$MAIN', 'TWICE'}, 'numeric': {'$MAIN', 'CHANGE'}}[name]
    assert set(functions) == expected and len(declarations) == len(expected), declarations
    for identity, label, arguments in declarations:
        assert len(re.findall(r'^'+label+r'\(\) \.locals=', text, re.M)) == 1, identity
        types = [argument.split('=', 1)[1] for argument in arguments.split(',')]
        assert len(types) == 2 and all(value in (
            '.rexxactivationarguments', '.rexxactivation..rexxactivationarguments')
            for value in types), arguments
    helpers = {'scale': ('arguments', 'append', 'existsarg'),
               'presence': ('arguments', 'append', 'existsarg'),
               'calls': ('arguments', 'append', 'existsarg'),
               'pipeline': ('arguments', 'append', 'lengthvalue'),
               'numeric': ('arguments', 'digits')}[name]
    for helper in helpers:
        assert re.search(r'^call [^\n]*llctarget\.'+helper+r'\(\)', text, re.M), helper
    if 'append' in helpers:
        assert re.search(r'\.meta "llctarget\.append"=[^\n]*present=\.int', text)
    assert not re.search(r'\.expose=llprogram\.', text), 'private routines became public entries'
    if name == 'calls':
        main = re.search(r'^'+functions['$MAIN']+r'\(\)[\s\S]*?(?=^llf\d+\(\)|\Z)',
                         text, re.M).group()
        calls = re.findall(r'^call [^,\n]+,(llf\d+)\(\)', main, re.M)
        assert calls == [functions[identity] for identity in
                         ('HOLES', 'HOLES', 'RECURSE', 'MARK', 'MARK', 'PAIR')], calls


try:
    for level in (['c'] if a.c_only else ['g', 'c']):
        fixtures = []
        for name in ['scale', 'presence', 'calls']:
            srcdir = root/'step07a' if name == 'calls' else examples
            source = srcdir/f'{name}-{level}.crexx'
            expected = (srcdir/f'{name}-{level}.expected').read_bytes().replace(b'\r\n', b'\n')
            fixtures.append((name, source, expected))
        if a.c_only:
            # Existing backend rows supply both source paths and independent
            # output expectations for the admitted LENGTH/DIGITS bindings.
            cases = json.loads((root/'backend/cases.json').read_text(encoding='utf-8'))
            selected = [case for case in cases if case['level'] == 'c' and
                        case['name'] in ('pipeline', 'numeric')]
            assert len(selected) == 2 and {case['name'] for case in selected} == {
                'pipeline', 'numeric'}
            for case in selected:
                assert not case['args'] and not case['error'], case
                fixtures.append((case['name'], (root/'backend'/case['source']).resolve(),
                                 case['output'].encode('utf-8')))
        for name, source, expected in fixtures:
            for ir in ['off', 'on']:
                label = f'{name}-{level}-{ir}'; asm = work/(label+'.rxas')
                compile_source(level, source, asm, ir)
                text = asm.read_text()
                assert str(source.parent.resolve()) not in text
                assert not re.search(r'"(?:[A-Za-z]:)?[^"\n]*[/\\]'+re.escape(source.name)+r'"', text)
                assert re.search(
                    r'^\s*\.srcstep\s+\d+\s+\d+\s+\d+\s+"'+re.escape(source.name)+
                    r'"\s+[1-9]\d*\s+[1-9]\d*\s+\d+\s+"', text, re.M), (
                        name, 'missing authored source record', source.name)
                if a.c_only:
                    classic_shape(text, name)
                shapes[label] = dict(bytes=len(text.encode()), calls=text.count('\ncall '), frames=re.findall(r'\.locals=(\d+)', text))
                execute(level, asm, expected, label)
                if name == 'scale' and level == 'g':
                    # Literal appears only in the missing branch, and missing callers
                    # clear supplied status without constructing an empty/default value.
                    assert re.search(r'brtpandt (llarg\d+)_present,a2,256\nload r\d+,2\n', text), text
                    assert ',512\n' in text and not re.search(r'^load r\d+,""$', text, re.M)
                    execute(level, asm, expected, label+'-asopt', assembler='opt')
                if a.c_only and name == 'scale':
                    execute(level, asm, expected, label+'-asopt', assembler='opt')
    if not a.c_only:
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
        for ir in (['off', 'on'] if a.c_only else ['on']):
            reject_label = label+'-reject'+('-'+ir if a.c_only else '')
            reject(source,b'incompatible',reject_label,'c',directory,ir=ir)
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
    verdict = 'STEP07_C_ONLY' if a.c_only else 'STEP07_E2E'
    print(f'{verdict}|commands={len(records)}|failures=0')
finally:
    run_directory.cleanup()
