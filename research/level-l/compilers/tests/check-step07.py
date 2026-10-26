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
p.add_argument('--inspect', type=Path, help='temporary directory for representative caller RXAS review')
p.add_argument('--c-only', action='store_true',
               help='check existing C calls in restricted compiler-host and static-target roots')
p.add_argument('--leave-metadata', action='store_true',
               help='run only focused invalid-transfer metadata controls (requires --c-only)')
p.add_argument('--do-metadata', action='store_true',
               help='run only the focused DO target metadata controls (requires --c-only)')
p.add_argument('--select-metadata', action='store_true',
               help='run only the focused SELECT target metadata controls (requires --c-only)')
p.add_argument('--bif-metadata', action='store_true',
               help='run only generic BIF target metadata controls (requires --c-only)')
a = p.parse_args()
if sum((a.leave_metadata, a.do_metadata, a.select_metadata, a.bif_metadata)) > 1:
    p.error('choose at most one focused metadata selection')
if (a.leave_metadata or a.do_metadata or a.select_metadata or a.bif_metadata) and not a.c_only:
    p.error('focused metadata selection requires --c-only')
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
        source = bin_dir/name
        if not source.exists():
            source = bin_dir/'providers'/name
        shutil.copy2(source, provider_directory/name)
    assert {path.name for path in provider_directory.iterdir()} == {
        provider+'.rxplugin' for provider in providers}
    return directory/(a.vm+suffix)


if a.c_only:
    # VM lookup includes the executable directory: PATH alone cannot exclude
    # sibling compilers or ambient bytecode from the shared build bin/.
    (work/'empty-path').mkdir()
    env.pop('CREXX_PROVIDER_PATH', None)
    compiler_directory = work/'compiler host'
    compiler_vm = stage_vm(compiler_directory, ['rxfs', 'rxcontract'])
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

def execute(level, out, expected, label, provider=None, error=None, assembler='noopt', arguments=()):
    run(label+'-assemble', [tool('rxas'), *(['-n'] if assembler == 'noopt' else []), out])
    return execute_image(level, out.with_suffix('.rxbin'), expected, label, provider, error, arguments)

def execute_image(level, image, expected, label, provider=None, error=None, arguments=()):
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
    result = run(label+'-run', [target_vm, *vm_options, image, *(['-a', *arguments] if arguments else [])], error is not None,
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


def classic_metadata(select_only=False, do_only=False, leave_only=False, bif_only=False):
    """Selected ABI mutations are assembled for import only, never executed.

    A valid authored class catalog is admitted first. Each negative changes
    one contract field, so missing unrelated metadata cannot mask the intended
    refusal. Expensive forgeries run once, while all four normal C-only cells
    exercise the real runtime's complete callable/owner contract.
    """
    c = 'rexxclassiccontext.rexxclassiccontext'
    a = 'rexxactivation.rexxactivationarguments'
    pool = 'rexxpool.rexxvariablepool'
    config = 'rexxclassicconfig.rexxclassicconfig'
    ct = '.rexxclassiccontext..rexxclassiccontext'
    at = '.rexxactivation..rexxactivationarguments'
    pt = '.rexxpool..rexxvariablepool'
    ft = '.rexxclassicconfig..rexxclassicconfig'
    declarations = {
        c+'.§factory': (ct, f'activation=reference {at},pool=reference {pt},config=reference {ft}'),
        c+'.activation': ('reference '+at, ''),
        c+'.pool': ('reference '+pt, ''),
        c+'.config': ('reference '+ft, ''),
        c+'.bindpool': ('.void', 'pool=reference '+pt),
        c+'.readsymbol': ('.rexxvalue..rexxvalue', 'name=.string'),
        config+'.§factory': (ft, '?profile=.string'),
        a+'.numericdigits': ('.int', ''), a+'.numericfuzz': ('.int', ''),
        a+'.numericform': ('.int', ''),
        a+'.append': ('.void', 'value=.rexxvalue,exists=.boolean'),
        a+'.inheritsignalpolicy': ('.void', 'caller=reference '+at),
        a+'.begininternalfunction': ('.void', ''),
        a+'.begininternalcall': ('.void', ''), a+'.enterprocedure': ('.void', ''),
        a+'.procedureeligible': ('.boolean', ''), a+'.consumefirstinstruction': ('.void', ''),
        a+'.requestprogramfailure': ('.boolean', 'problem=.rxfnsb..runtime_signal'),
        a+'.returnvalue': ('.rexxvalue..rexxvalue', ''),
        a+'.hasreturnvalue': ('.boolean', ''),
        a+'.requestprogramexit': ('.void', 'value=.rexxvalue..rexxvalue,present=.boolean'),
        a+'.programexitrequested': ('.boolean', ''),
        a+'.queuecallcondition': ('.boolean', 'condition=.int,description=.string,line=.int'),
        a+'.signalpolicy': ('.int', 'condition=.int'),
        'rexxclassicconditionevent.rexxclassicconditionevent.§factory':
            ('.rexxclassicconditionevent..rexxclassicconditionevent', 'condition=.int,description=.string'),
        pool+'.inheritcommandstatus': ('.void', 'caller=reference '+pt),
        pool+'.setcalltrapsigl': ('.void', 'line=.int'),
        'rexxclassicbifnumeric.rexxclassicbif_digits':
            ('.rexxvalue..rexxvalue', 'context=reference .rexxbifcallcontext'),
        'rexxclassicbifabs.rexxclassicbif_abs':
            ('.rexxvalue..rexxvalue', 'context=reference .rexxbifcallcontext'),
        'rexxclassicbifs.rexxbifcallcontext.haserror': ('.boolean', ''),
        'rexxclassicbifs.rexxbifcallcontext.errordetail': ('.string', ''),
        'rexxclassicbifs.rexxbifcallcontext.conditionid': ('.int', ''),
        'rexxclassicbifs.rexxbifcallcontext.conditiondescription': ('.string', ''),
    }
    lines = ['.globals=0']
    for owner in (c, a, pool, config, 'rexxclassicconditionevent.rexxclassicconditionevent'):
        lines.append(f'.meta "{owner}"="b" ".unknown" .class')
    lines += ['.meta "rexxclassicbifs.rexxbifcallcontext"="b" ".unknown" .class',
              '.meta "rexxclassicbifs.rexxbifcallcontext.bif_arg_count"="b" ".int" .attr 1',
              f'.meta "{a}.values"="b" ".rexxvalue[*]" .attr 1',
              f'.meta "{a}.presence"="b" ".int[*]" .attr 2',
              f'.meta "{a}.slot_count"="b" ".int" .attr 3',
              f'.meta "{a}.supplied_count"="b" ".int" .attr 4']
    for name, (result, arguments) in declarations.items():
        lines += [f'§{name}() .locals=1 .expose={name}', 'ret r0',
                  f'.meta "{name}"="b" "{result}" §{name}() "{arguments}"']
    valid = '\n'.join(lines)+'\n'
    target = (backend/'llctarget.rxas').read_text(encoding='utf-8')
    source = root/'backend/numeric-c.crexx'
    select_source = root/'backend/select-missing-c.crexx'
    do_source = work/'do-metadata.crexx'
    do_source.write_text('options levelc; do i=1 to 2 by 1 for 1 while flag; nop; end; do until flag; nop; end; exit 0\n')
    leave_source = work/'leave-metadata.crexx'
    leave_source.write_text('options levelc strictc; leave MiXeD; exit 0\n')
    bif_source = work/'bif-metadata.crexx'
    bif_source.write_text('options levelc\nsay abs(-1)\nexit 0\n')

    def fixture(label, package, text):
        directory = work/label
        directory.mkdir()
        for module in ('llctarget', 'rxfnsc'):
            if module != package:
                shutil.copyfile(runtime/(module+'.rxbin'), directory/(module+'.rxbin'))
        asm = directory/(package+'.rxas')
        asm.write_text(text, encoding='utf-8')
        run(label+'-assemble', [tool('rxas'), '-n', asm])
        return directory

    directory = fixture('c-valid-catalog', 'rxfnsc', valid)
    for ir in (['on'] if select_only or do_only or leave_only or bif_only else ['off', 'on']):
        compile_source('c', bif_source if bif_only else leave_source if leave_only else do_source if do_only else select_source if select_only else source, work/('c-valid-catalog-'+ir+'.rxas'), ir,
                       label='c-valid-catalog-'+ir, imports=directory)
    if not select_only and not do_only and not leave_only and not bif_only:
        compile_source('c', select_source, work/'c-select-valid-catalog.rxas',
                       label='c-select-valid-catalog', imports=directory)
        compile_source('c', do_source, work/'c-do-valid-catalog.rxas',
                       label='c-do-valid-catalog', imports=directory)
        compile_source('c', leave_source, work/'c-leave-valid-catalog.rxas',
                       label='c-leave-valid-catalog', imports=directory)

    cases = [
        ('c-target-optional', 'llctarget', 'context=reference '+ct,
         '?context=reference '+ct, 'llctarget.binaryop'),
        ('c-target-nominal', 'llctarget', 'context=reference '+ct,
         'context=reference .foreign..rexxclassiccontext', 'llctarget.binaryop'),
        ('c-setting-result', 'llctarget', '".void"', '".rexxvalue..rexxvalue"',
         'llctarget.setting'),
        ('c-class-interface', 'rxfnsc', '.class', '.interface', c),
        ('c-factory-optional', 'rxfnsc', 'activation=reference '+at,
         '?activation=reference '+at, c+'.§factory'),
        ('c-getter-value', 'rxfnsc', '"reference '+at+'"', '"'+at+'"', c+'.activation'),
        ('c-getter-nominal', 'rxfnsc', '"reference '+at+'"',
         '"reference .foreign..rexxactivationarguments"', c+'.activation'),
        ('c-getter-receiver', 'rxfnsc', '() ""', '() "self=reference '+ct+'"', c+'.activation'),
        ('c-config-required', 'rxfnsc', '?profile=.string', 'profile=.string', config+'.§factory'),
        ('c-member-identity', 'rxfnsc', '"'+c+'.pool"', '"'+c+'.wrongpool"', c+'.pool'),
        ('c-bif-optional', 'rxfnsc', 'context=reference .rexxbifcallcontext',
         '?context=reference .rexxbifcallcontext', 'rexxclassicbifabs.rexxclassicbif_abs'),
        ('c-bif-context', 'rxfnsc', 'context=reference .rexxbifcallcontext',
         'context=reference .wrong', 'rexxclassicbifabs.rexxclassicbif_abs'),
        ('c-bif-layout', 'rxfnsc', '".int"', '".string"',
         'rexxclassicbifs.rexxbifcallcontext.bif_arg_count'),
        ('c-select-truth-result', 'llctarget', '".int"', '".string"',
         'llctarget.whentruth'),
        ('c-select-truth-context', 'llctarget', 'context=reference '+ct,
         '?context=reference '+ct, 'llctarget.whentruth'),
        ('c-select-missing-result', 'llctarget', '".void"', '".int"',
         'llctarget.selectmissing'),
        ('c-select-missing-line', 'llctarget', 'line=.string', 'line=.binary',
         'llctarget.selectmissing'),
        ('c-select-missing-optional', 'llctarget', 'line=.string', '?line=.string',
         'llctarget.selectmissing'),
        ('c-leave-result', 'llctarget', '".void"', '".int"', 'llctarget.invalidtransfer'),
        ('c-leave-code-optional', 'llctarget', 'code=.string', '?code=.string', 'llctarget.invalidtransfer'),
        ('c-leave-name-optional', 'llctarget', 'name=.string', '?name=.string', 'llctarget.invalidtransfer'),
        ('c-leave-name-type', 'llctarget', 'name=.string', 'name=.int', 'llctarget.invalidtransfer'),
        ('c-do-new-result', 'llctarget', '".rexxdostate..rexxdostate"', '".rexxvalue..rexxvalue"', 'llctarget.donew'),
        ('c-do-capture-state', 'llctarget', 'state=reference .rexxdostate..rexxdostate', 'state=.rexxdostate..rexxdostate', 'llctarget.docapture'),
        ('c-do-start-nominal', 'llctarget', 'state=reference .rexxdostate..rexxdostate', 'state=reference .foreign..rexxdostate', 'llctarget.dostart'),
        ('c-do-entry-result', 'llctarget', '".int"', '".boolean"', 'llctarget.doentry'),
        ('c-do-advance-optional', 'llctarget', 'state=reference .rexxdostate..rexxdostate', '?state=reference .rexxdostate..rexxdostate', 'llctarget.doadvance'),
        ('c-do-while-context', 'llctarget', 'context=reference '+ct, '?context=reference '+ct, 'llctarget.whiletruth'),
        ('c-do-until-result', 'llctarget', '".int"', '".string"', 'llctarget.untiltruth'),
    ]
    for label, package, before, after, name in cases:
        is_select = label.startswith('c-select-')
        is_do = label.startswith('c-do-')
        is_leave = label.startswith('c-leave-')
        is_bif = label.startswith('c-bif-')
        if (select_only and not is_select) or (do_only and not is_do) or (leave_only and not is_leave) or (bif_only and not is_bif):
            continue
        text = target if package == 'llctarget' else valid
        pattern = re.compile(r'(^\s*\.meta "'+re.escape(name)+r'"="b"[^\n]*$)', re.M)
        matches = list(pattern.finditer(text))
        assert len(matches) == 1 and matches[0].group().count(before) == 1, (label, matches)
        replacement = matches[0].group().replace(before, after, 1)
        text = text[:matches[0].start()]+replacement+text[matches[0].end():]
        directory = fixture(label, package, text)
        for ir in (['on'] if is_select or is_do or is_leave or bif_only else ['off', 'on']):
            # Identity mutations deliberately exercise selected-call lookup;
            # all other cases reach a concrete signature or class refusal.
            message = b'not found' if label == 'c-member-identity' else b'incompatible'
            reject(bif_source if is_bif else leave_source if is_leave else do_source if is_do else select_source if is_select else source, message, label+'-'+ir, 'c', directory, ir=ir)


def classic_construction(text, name):
    # Check the frontend's unoptimised construction at authored clauses.
    # A supplied expression must not first construct an unused default value.
    literals, line, latest = {}, 0, None
    for row in text.splitlines():
        origin = re.match(r'^\.srcstep \d+ \d+ \d+ "[^"]*" (\d+) ', row)
        if origin:
            line = int(origin[1])
        loaded = re.match(r'^load r\d+,"([^"]*)"$', row)
        if loaded:
            latest = loaded[1]
        if re.match(r'^call [^\n]*llctarget\.literal\(\)', row):
            literals.setdefault(line, []).append(latest)
    if name == 'calls':
        assert literals[2] == ['', '3', ''], ('omitted argument placeholders', literals[2])
        assert literals[3] == ['4', '3', '5'], ('present arguments construct only supplied values', literals[3])
    if name == 'numeric-fatal':
        for line, expected in {2: ['4'], 3: ['1'], 8: ['0'],
                               10: ['3'], 11: ['2'], 13: ['2']}.items():
            assert literals[line] == expected, ('explicit NUMERIC/EXIT value', line, literals[line])


def classic_packets(text, body):
    seeds = list(re.finditer(r'^call (r\d+),llctarget\.arguments\(\),r\d+$', body, re.M))
    owners = {seed[1]: seed.start() for seed in seeds}
    assert seeds and len(seeds) == len(owners), 'distinct owned fresh packet seeds'
    assert not re.search(r'^call [^\n]*llctarget\.append\(\)', body, re.M), 'fresh builders never copy persistent prefixes'
    target = 'rexxactivation.rexxactivationarguments.append'
    mutations = re.findall(r'^call [^\n]*' + re.escape(target) + r'\(\)', body, re.M)
    # Each direct method borrows its fixed seed, copies the actual value and
    # Boolean presence, then restores/releases every call-window reference.
    windows = list(re.finditer(
        r'^link (?P<receiver>r\d+),(?P<owner>r\d+)\n'
        r'copy (?P<valuearg>r\d+),r\d+\nicopy (?P<presentarg>r\d+),r\d+\n'
        r'swap (?P=receiver),(?P<receiverarg>r\d+)\nload (?P<count>r\d+),3\n'
        r'settp (?P=receiverarg),256\nsettp (?P=valuearg),768\nsettp (?P=presentarg),768\n'
        r'call r\d+,' + re.escape(target) + r'\(\),(?P=count)\n'
        r'swap (?P=receiver),(?P=receiverarg)\nnull (?P=receiverarg)\n'
        r'null (?P=valuearg)\nnull (?P=presentarg)\nunlink (?P=receiver)$', body, re.M))
    assert mutations and len(windows) == len(mutations), 'all fresh appends retain exact value/presence/receiver windows'
    for window in windows:
        assert window['owner'] in owners and owners[window['owner']] < window.start(), 'append receiver borrows its preceding fresh seed owner'
        base = int(window['count'][1:])
        assert [int(window[field][1:]) for field in ('receiverarg', 'valuearg', 'presentarg')] == [base + 1, base + 2, base + 3], 'contiguous receiver/value/presence arguments'
    assert re.search(r'^\s*\.meta "' + re.escape(target) + r'"="b" "\.void" ' +
                     re.escape(target) + r'\(\) "value=\.rexxvalue,exists=\.boolean"$', text, re.M), 'existing mutating append Boolean ABI'


def classic_packet_controls():
    seed = 'call r3,llctarget.arguments(),r20\n'
    window = ('link r8,r3\ncopy r22,r4\nicopy r23,r5\nswap r8,r21\nload r20,3\n'
              'settp r21,256\nsettp r22,768\nsettp r23,768\n'
              'call r9,rexxactivation.rexxactivationarguments.append(),r20\n'
              'swap r8,r21\nnull r21\nnull r22\nnull r23\nunlink r8\n')
    metadata = (' .meta "rexxactivation.rexxactivationarguments.append"="b" ".void" '
                'rexxactivation.rexxactivationarguments.append() "value=.rexxvalue,exists=.boolean"\n')
    body = seed + window
    classic_packets(body + metadata, body)
    for bad_body, bad_metadata in (
        (body + 'call r6,llctarget.append(),r20\n', metadata),
        (body.replace('icopy r23', 'copy r23'), metadata),
        (body.replace('copy r22', 'move r22'), metadata),
        (body.replace('link r8,r3', 'link r8,r7'), metadata),
        (body.replace('unlink r8\n', ''), metadata),
        (seed + body, metadata),
        (window + seed, metadata),
        (body, metadata.replace('exists=.boolean', 'exists=.int')),
    ):
        try:
            classic_packets(bad_body + bad_metadata, bad_body)
        except AssertionError:
            continue
        raise AssertionError('fresh-packet control admitted an invalid append')


def classic_shape(text, name):
    # One checked unit owns every source label. Packet presence, numeric
    # inheritance and the independent runtime outputs retain their old checks.
    declarations = re.findall(
        r'^\s*\.meta "llprogram\.([^"]+)"="b" "[^"]+" (llf\d+)\(\) "([^"]*)"',
        text, re.M)
    signature = 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int'
    assert declarations == [('$MAIN', 'llf1', signature)], declarations
    labels = {'scale': ('SCALE',), 'presence': ('PROBE',),
              'calls': ('HOLES', 'RECURSE', 'MARK', 'PAIR'),
              'pipeline': ('TWICE',), 'numeric': ('CHANGE',), 'argv': (),
              'numeric-fatal': ('INVALID',)}[name]
    identities = re.findall(r'^beq llprocedure1_\d+_match,[^,\n]+,(\d+)$', text, re.M)
    assert identities == [str(i) for i in range(len(labels) + 1)], ('dense integer dispatch', identities)
    helpers = {'scale': ('arguments', 'existsarg'),
               'presence': ('arguments', 'existsarg'),
               'calls': ('arguments', 'existsarg'),
               'pipeline': ('arguments', 'lengthvalue'),
               'numeric': ('digits', 'setting'), 'argv': ('parsesource', 'countargs'),
               'numeric-fatal': ('setting',)}[name]
    for helper in helpers:
        assert re.search(r'^call [^\n]*llctarget\.'+helper+r'\(\)', text, re.M), helper
    assert not re.search(r'\.expose=llprogram\.', text), 'unit became a public source entry'
    assert 'llctarget2' not in text and 'contextnew' not in text, 'obsolete C target bridge'
    bodies = {match[1]: match[2] for match in re.finditer(
        r'^(main|llf\d+)\(\) \.locals=\d+([^\n]*\n[\s\S]*?)(?=^(?:main|llf\d+)\(\) \.locals=|\Z)', text, re.M)}
    assert set(bodies) == {'main', 'llf1'}, bodies.keys()
    if name in ('scale', 'presence', 'calls', 'pipeline'):
        classic_packets(text, bodies['llf1'])
    config_factory = 'rexxclassicconfig.rexxclassicconfig.§factory'
    bundle_factory = 'rexxclassiccontext.rexxclassiccontext.§factory'
    pool_getter = 'rexxclassiccontext.rexxclassiccontext.pool'
    config_getter = 'rexxclassiccontext.rexxclassiccontext.config'
    activation_owner = 'rexxactivation.rexxactivationarguments.'
    assert len(re.findall(r'^call [^\n]*'+re.escape(config_factory)+r'\(\)', text, re.M)) == 1
    assert re.search(r'^load r\d+,"UTF8"$', bodies['main'], re.M), 'root configuration policy'
    for label, body in bodies.items():
        instructions, source_lines, line = [], [], 0
        for row in body.splitlines():
            origin = re.match(r'^\.srcstep \d+ \d+ \d+ "[^"]*" (\d+) ', row)
            if origin:
                line = int(origin[1])
            if row and not row.startswith('.') and not row.endswith(':'):
                instructions.append((row.split(' ', 1)+[''])[:2])
                source_lines.append(line)
        calls = [(i, *re.fullmatch(r'(r\d+),([^,]+)\(\),(r\d+)', args).groups())
                 for i, (op, args) in enumerate(instructions) if op == 'call']
        local = [call for call in calls if re.fullmatch(r'llf\d+', call[2])]
        pools = [call for call in calls if call[2] == 'llctarget.poolnew']
        bundles = [call for call in calls if call[2] == bundle_factory]
        assert len(bundles) == len(local), (label, 'one separate bundle per root/child call')
        assert len(pools) == (1 if label == 'main' else len(labels)), (label, 'root or reached PROCEDURE pool sites')
        assert not any(op == 'deref' for op, _ in instructions), (label, 'copied authoritative owner')
        entry_calls = []
        for index, destination, target, window in local:
            assert target == 'llf1', (label, target, 'single source unit')
            first = 'r'+str(int(window[1:])+1)
            writes = [(op, args.split(',')[1]) for op, args in instructions[:index]
                      if op in ('copy', 'mkref') and args.startswith(first+',')]
            preceding = [call for call in bundles if call[0] < index][-1]
            assert writes[-1] == ('mkref', preceding[1]), (label, 'bundle reference')
            assert instructions[index+1] == ['null', first], (label, 'reference window cleanup')
            second = 'r'+str(int(window[1:])+2)
            entry_writes = [(op, args.split(',', 1)[1]) for op, args in instructions[:index]
                            if op in ('load', 'copy') and args.startswith(second+',')]
            assert entry_writes[-1][0] == 'load', (label, 'explicit entry identity')
            entry = int(entry_writes[-1][1])
            assert entry == 0 if label == 'main' else 1 <= entry <= len(labels)
            entry_calls.append((source_lines[index], entry))
        for index, destination, target, window in bundles:
            base = int(window[1:])
            fields = []
            for ordinal in (1, 2, 3):
                slot = f'r{base+ordinal}'
                writes = [(op, args.split(',')[1]) for op, args in instructions[:index]
                          if op in ('copy', 'mkref') and args.startswith(slot+',')]
                fields.append(writes[-1])
            if label == 'main':
                assert [op for op, _ in fields] == ['mkref', 'mkref', 'copy'], 'root owns activation/pool and shares config'
                assert fields[1][1] == pools[0][1], 'root pool owner'
            else:
                assert [op for op, _ in fields] == ['mkref', 'copy', 'copy'], 'child owns activation and borrows caller pool/config'
                assert fields[1][1] == [call for call in calls if call[0] < index and call[2] == pool_getter][-1][1], 'borrow current caller pool'
                assert fields[2][1] == [call for call in calls if call[0] < index and call[2] == config_getter][-1][1], 'borrow caller configuration'
                policy = [call for call in calls if call[0] < index and call[2] == activation_owner+'inheritsignalpolicy'][-1]
                region = [call[2] for call in calls if policy[0] <= call[0] < index]
                assert not any(method in region for method in ('llctarget.poolnew', activation_owner+'enterprocedure', 'rexxpool.rexxvariablepool.inheritcommandstatus')), 'call setup must share until reached PROCEDURE'
        if label == 'main':
            continue
        assert sum(target == activation_owner+'enterprocedure' for _, _, target, _ in calls) == len(labels)
        assert sum(target == activation_owner+'procedureeligible' for _, _, target, _ in calls) == len(labels)+1
        assert sum(target == activation_owner+'inheritsignalpolicy' for _, _, target, _ in calls) == len(local)
        assert sum(target in (activation_owner+'begininternalcall', activation_owner+'begininternalfunction') for _, _, target, _ in calls) == len(local)
        assert re.findall(r'^numsci (.*)$', body, re.M) == ['18,1,2'], 'Classic standard at unit entry'
        settings = sum(target == 'llctarget.setting' for _, _, target, _ in calls)
        installs = re.findall(r'^setnumfuz 0\nsetnumdgts r\d+\nsetnumfuz r\d+\nsetnumfrm r\d+$', body, re.M)
        assert len(installs) == settings+1, ('entry/successful NUMERIC install count', len(installs), settings)
        assert len(re.findall(r'^setnumdgts ', body, re.M)) == len(installs)
        assert len(re.findall(r'^setnumfrm ', body, re.M)) == len(installs)
        handlers = [i for i, (op, _) in enumerate(instructions) if op == 'sigbrv']
        first_numeric = next(i for i, (op, _) in enumerate(instructions) if op == 'numsci')
        finite = {'CLASSIC_SYNTAX', 'DIVISION_BY_ZERO', 'CONVERSION_ERROR', 'INVALID_ARGUMENTS',
                  'OBJECT_NOT_INITIALIZED', 'REFERENCE_INVALID', 'UNICODE_ERROR', 'OUT_OF_RANGE', 'FAILURE'}
        assert len(handlers) == len(finite) and max(handlers) < first_numeric, 'own-frame diversion before context installation'
        assert set(re.findall(r'^sigbrv [^\n]*,"([A-Z_]+)"$', body, re.M)) == finite
        error_body = body[body.index('\nllerror'):]
        assert set(re.findall(r'^sighalt "([A-Z_]+)"$', error_body, re.M)) == finite
        assert not re.search(r'^(?:setnumdgts|setnumfuz|setnumfrm) ', error_body, re.M)
        assert len(re.findall(r'^signalorigin [^\n]+\nret$', error_body, re.M)) == len(finite)
        assert all(target != config_factory for _, _, target, _ in calls)
        if name == 'calls':
            by_line = {}
            for line, entry in entry_calls:
                by_line.setdefault(line, []).append(entry)
            assert by_line == {2: [1], 3: [1], 4: [2], 5: [3, 3, 4], 18: [2]}, ('source call order/recursive identity', by_line)


try:
    if a.c_only:
        classic_packet_controls()
    if a.bif_metadata:
        classic_metadata(bif_only=True)
        print(f'BIF_METADATA|commands={len(records)}|failures=0')
        raise SystemExit(0)
    if a.leave_metadata:
        classic_metadata(leave_only=True)
        print(f'LEAVE_METADATA|commands={len(records)}|failures=0')
        raise SystemExit(0)
    if a.do_metadata:
        classic_metadata(do_only=True)
        print(f'DO_METADATA|commands={len(records)}|failures=0')
        raise SystemExit(0)
    if a.select_metadata:
        classic_metadata(select_only=True)
        print(f'SELECT_METADATA|commands={len(records)}|failures=0')
        raise SystemExit(0)
    for level in (['c'] if a.c_only else ['g', 'c']):
        fixtures = []
        for name in ['scale', 'presence', 'calls']:
            srcdir = root/'step07a' if name == 'calls' else examples
            source = srcdir/f'{name}-{level}.crexx'
            expected = (srcdir/f'{name}-{level}.expected').read_bytes().replace(b'\r\n', b'\n')
            fixtures.append((name, source, expected, (), None))
        if a.c_only:
            # Existing backend rows supply both source paths and independent
            # output expectations for the admitted LENGTH/DIGITS bindings.
            cases = json.loads((root/'backend/cases.json').read_text(encoding='utf-8'))
            selected = [case for case in cases if case['level'] == 'c' and
                        case['name'] in ('pipeline', 'numeric', 'argv')]
            assert len(selected) == 3 and {case['name'] for case in selected} == {
                'pipeline', 'numeric', 'argv'}
            for case in selected:
                assert not case['error'], case
                fixtures.append((case['name'], (root/'backend'/case['source']).resolve(),
                                 case['output'].encode('utf-8'), case['args'], None))
            fixtures.append(('numeric-fatal', root/'backend/numeric-fatal-c.crexx',
                             (root/'backend/numeric-fatal-c.expected').read_bytes(),
                             (), b'RXC-LC-33.1'))
        for name, source, expected, arguments, error in fixtures:
            for ir in ['off', 'on']:
                label = f'{name}-{level}-{ir}'; asm = work/(label+'.rxas')
                compile_source(level, source, asm, ir)
                text = asm.read_text()
                if a.inspect and a.c_only and name in ('scale', 'calls', 'numeric', 'numeric-fatal'):
                    a.inspect.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(asm, a.inspect/(label+'.rxas'))
                assert str(source.parent.resolve()) not in text
                assert not re.search(r'"(?:[A-Za-z]:)?[^"\n]*[/\\]'+re.escape(source.name)+r'"', text)
                assert re.search(
                    r'^\s*\.srcstep\s+\d+\s+\d+\s+\d+\s+"'+re.escape(source.name)+
                    r'"\s+[1-9]\d*\s+[1-9]\d*\s+\d+\s+"', text, re.M), (
                        name, 'missing authored source record', source.name)
                if a.c_only:
                    classic_shape(text, name)
                    if ir == 'off':
                        classic_construction(text, name)
                shapes[label] = dict(bytes=len(text.encode()), calls=text.count('\ncall '), frames=re.findall(r'\.locals=(\d+)', text))
                execute(level, asm, expected, label, arguments=arguments, error=error)
                if error:
                    diagnostic = (work/(label+'-run.stderr')).read_bytes()
                    assert b'numeric-fatal-c.crexx:13:' in diagnostic, diagnostic
                    assert b'unreachable' not in diagnostic, diagnostic
                if name == 'scale' and level == 'g':
                    # Literal appears only in the missing branch, and missing callers
                    # clear supplied status without constructing an empty/default value.
                    assert re.search(r'brtpandt (llarg\d+)_present,a2,256\nload r\d+,2\n', text), text
                    assert ',512\n' in text and not re.search(r'^load r\d+,""$', text, re.M)
                    execute(level, asm, expected, label+'-asopt', assembler='opt')
                if a.c_only and name in ('scale', 'numeric'):
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
    if a.c_only and a.mode == 'opt' and a.vm == 'rxbvm':
        classic_metadata()
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
