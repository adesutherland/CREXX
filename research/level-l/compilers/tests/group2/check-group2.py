#!/usr/bin/env python3
"""One prepared Debug/rxbvm lane for the changed Group 2 call boundaries."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


PACKET_DECLARATION = ('.meta "g2packet.run"="b" ".void" llprovider() '
                      '"frame=.rexxactivation..rexxactivationarguments"')


def signature_image(*, packet=False):
    """Metadata-only fixtures have no provider execution or task contexts."""
    if packet:
        # Declaration-only input for emission admission; never linked or run.
        # --routine has this fixed ABI independently of the C provider body.
        return ('\n'.join(('.globals=0', 'llprovider() .locals=1 .expose=g2packet.run',
                           'ret', PACKET_DECLARATION)) + '\n')
    rows = ['.globals=0']
    for name, result, arguments in (
        ('scalar.required', '.int', 'value=.int'),
        ('scalar.optional', '.string', "?value='DEFAULT'"),
        ('scalar.tail', '.void', 'prefix=.string,...=.string'),
        ('scalar.reference', '.void', 'value=reference .int'),
        ('scalar.array', '.void', 'value=.int[*]'),
        ('scalar.task', '.int', 'value=.int'),
        ('scalar.duplicate', '.int', 'value=.int'),
    ):
        rows.extend((f'§{name}() .locals=1 .expose={name}', 'load r0,0',
                     'ret' if result == '.void' else 'ret r0',
                     f'.meta "{name}"="b" "{result}" §{name}() "{arguments}"'))
    for ordinal, name in enumerate(('scalar.task', 'scalar.duplicate', 'scalar.duplicate'), 1):
        # The writer seals each placeholder. Distinct targets must not alias
        # one deduplicated binary constant before that owning relocation.
        rows.append(f'.meta "{name}"=".task1" 0x' + f'{ordinal:02x}' * 80)
    return '\n'.join(rows) + '\n'


def typed_shapes(text):
    """Assert ABI obligations without depending on local register allocation."""
    rows = text.splitlines()
    calls = {}
    for at, row in enumerate(rows):
        match = re.search(r'\bcall\s+[^,]+,(g2scalar\.[a-z]+)\(\),(r\d+)\s*$', row)
        if not match:
            continue
        window = match[2]
        before = '\n'.join(rows[max(0, at - 16):at])
        counts = re.findall(r'\bload\s+' + window + r',(\d+)\s*$', before, re.M)
        assert counts, ('counted ordinary ABI', row, before)
        calls.setdefault(match[1], []).append(int(counts[-1]))
        if match[1] == 'g2scalar.mutate':
            formal = 'r' + str(int(window[1:]) + 1)
            assert re.search(r'\bsettp\s+' + formal + r',256\s*$', before, re.M), 'exposed scalar has its own ordinary writable cell'
    assert calls.get('g2scalar.twice') == [1, 1], calls
    assert calls.get('g2scalar.optional') == [2, 2, 2], 'omitted optional fixed slots retain the ordinary ABI'
    assert calls.get('g2scalar.tail') == [1, 3], 'scalar varargs use actual repeated cells without a synthetic array'
    assert not re.search(r'\bcall\s+[^,]+,(?:rxcontract|llbinmeta|llcbindings)\.', text), 'static application has no compiler decoder'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--process-fixture', type=Path, required=True)
    parser.add_argument('--argv-fixture', type=Path, required=True)
    parser.add_argument('--spans-fixture', type=Path, required=True)
    parser.add_argument('--phase', choices=('all', 'prepare', 'run', 'emit'), default='all')
    parser.add_argument('--reuse-c', action='append', default=[], help='Reuse an explicitly reviewed C assembly in active prepared scratch')
    parser.add_argument('--skip-check', action='append', default=[], help='Reuse an already passed exact check stem with --phase run')
    parser.add_argument('--emit-case', action='append', default=[], help='Select maintained request stems only in --phase emit')
    args = parser.parse_args()
    emit_only = args.phase == 'emit'
    if args.emit_case and not emit_only:
        parser.error('--emit-case requires --phase emit')
    if emit_only and args.reuse_c:
        parser.error('--phase emit compiles current requests; --reuse-c is functional preparation only')
    if args.skip_check and args.phase != 'run':
        parser.error('--skip-check requires --phase run and an existing prepared manifest')
    build = args.build.resolve(); work = args.work.resolve(); work.mkdir(parents=True, exist_ok=True)
    temporary = None
    if args.phase == 'all':
        temporary = tempfile.TemporaryDirectory(prefix='group2-', dir=work)
        work = Path(temporary.name)
    if emit_only:
        # Do not replace or qualify any prepared functional products.
        work = work / 'emission'; work.mkdir(exist_ok=True)
    fixtures = Path(__file__).resolve().parent
    native_fixtures = fixtures.parents[4] / 'compiler/tests/rexx_src'
    backend = build / 'research/level-l/compilers/backend/opt'
    suffix = '.exe' if os.name == 'nt' else ''
    runtime = work / 'runtime'; host = work / 'host'; app = work / 'application'
    empty = work / 'empty PATH'
    for directory in (runtime, host, app, empty):
        directory.mkdir(exist_ok=True)
    env = dict(os.environ, PATH=str(empty)); env.pop('CREXX_PROVIDER_PATH', None)
    vm = app / ('rxbvm' + suffix); hostvm = host / vm.name
    compiler = host / 'llrxc-c-linked.rxbin'
    manifest = work / 'prepared.json'
    checks = []; requests = []; pending = []; negatives = []

    def run(label, command, *, failure=False, silent=True, cwd=app, timeout=900):
        result = subprocess.run([str(item) for item in command], cwd=cwd, env=env,
                                capture_output=True, timeout=timeout)
        diagnostic = result.stdout + result.stderr
        assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
        assert (result.returncode != 0 if failure else result.returncode == 0), (label, result.returncode, diagnostic[-6000:])
        if silent:
            assert not diagnostic, (label, diagnostic[-4000:])
        return result

    def tool(name):
        return build / 'bin' / (name + suffix)

    def assemble(path):
        run('assemble ' + path.name, [tool('rxas'), path])
        return path.with_suffix('.rxbin')

    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')

    def link(stem, image, extra=()):
        output = app / (stem + '-linked'); control = work / (stem + '.ctl')
        inputs = [image, *extra, *(runtime / (name + '.rxbin') for name in libraries)]
        control.write_text(''.join('INPUT ' + str(item) + '\n' for item in inputs) +
                           'ROOT ' + image.stem + '\nOUTPUT ' + str(output) + '\n', encoding='utf-8')
        run('link ' + stem, [tool('rxlink'), '-c', control])
        return output.with_suffix('.rxbin')

    def bcompile(source, stem):
        # Native typed import discovery also scans the primary source root.
        # Keep this consumer away from unrelated Classic request sources.
        source_root = work / 'typed-sources' / stem
        source_root.mkdir(parents=True, exist_ok=True)
        isolated_source = source_root / source.name
        shutil.copy2(source, isolated_source)
        assembly = app / (stem + '.rxas')
        run('B/G fixture ' + stem, [tool('rxc'), '-x', '--no-exe-import', '-i', runtime, '-o', assembly, isolated_source])
        return assemble(assembly)

    def ccompile(text, stem, bindings=(), option=(), failure=False, directory=None):
        source = work / (stem + '.crexx')
        assembly = (directory or app) / (stem + '.rxas')
        if stem in args.reuse_c:
            assert not failure and source.read_bytes() == text.encode() and assembly.exists(), ('reviewed unchanged C assembly', stem)
            if not assembly.with_suffix('.rxbin').exists():
                assemble(assembly)
            return assembly
        source.write_text(text, encoding='utf-8')
        if failure:
            assembly.write_bytes(b'previous output\n')
            negatives.append((stem, assembly))
        assert len(bindings) % 3 == 0
        fields = [str(source), str(assembly), source.name,
                  option[1] if option and option[0] == '--routine' else '',
                  option[1] if option and option[0] == '--host' else '',
                  'IMPORT' if failure else '', str(len(bindings) // 3)]
        for at in range(0, len(bindings), 3):
            kind = {'--provider': 'rxbin', '--native': 'native', '--executable': 'process'}[bindings[at]]
            fields += [kind, bindings[at + 1], bindings[at + 2]]
        assert not any('|' in field or '\n' in field for field in fields), 'bounded fixture manifest fields'
        requests.append((stem, '|'.join(fields)))
        return assembly

    def check(stem, assembly, expected, extra=(), stderr='', failure=False, cause=''):
        pending.append((stem, assembly, expected, extra, stderr, failure, cause))

    if args.phase != 'run':
        for name in libraries:
            source = backend if name == 'llctarget' else build / 'bin'
            shutil.copy2(source / (name + '.rxbin'), runtime / (name + '.rxbin'))
        # Host callbacks can fall through to automatic source lookup. Their
        # compiler metadata is available here; only that consumer links it.
        dynamic = runtime / 'llcdynamic.rxbin'
        shutil.copy2(build / 'research/level-l/compilers/products/llcdynamic.rxbin', dynamic)
        for directory in (host, app):
            shutil.copy2(tool('rxbvm'), directory / ('rxbvm' + suffix))
            providers = directory / 'providers'; providers.mkdir(exist_ok=True)
            for alias in ('rxfs', 'rxcstream', 'rxcontract', 'rx_hash', 'rx_contract', 'rx_fs', 'rx_cstream'):
                for source in (build / 'bin' / (alias + '.rxplugin'), build / 'bin/providers' / (alias + '.rxplugin')):
                    if source.exists():
                        shutil.copy2(source, providers / source.name)
                        # Typed fixtures import native declarations from -i.
                        shutil.copy2(source, runtime / source.name)
            if os.name == 'nt':
                for source in (build / 'bin').glob('*.dll'):
                    shutil.copy2(source, directory / source.name)
                    shutil.copy2(source, runtime / source.name)
        shutil.copy2(backend / compiler.name, compiler)
        resolution = native_fixtures / 'levelc_call_resolution_result.rexx'
        resolution_expected = resolution.with_suffix('.expected')
        recursive = native_fixtures / 'levelc_frame_recursive_call.rexx'
        check('minimal-c', ccompile('say 1\n', 'minimal-c'), '1\n')
        signatures = app / 'group2-signatures.rxas'
        signatures.write_text(signature_image(), encoding='utf-8')
        shutil.copy2(assemble(signatures), runtime / 'group2-signatures.rxbin')
        if not emit_only:
            contracts = host / 'call-external-contracts-linked.rxbin'
            shutil.copy2(backend / contracts.name, contracts)
            checks.append(dict(stem='raw typed/loaded admission and task metadata', image=str(contracts),
                               expected='', stderr='', failure=False, cause='', argv=[str(runtime), 'group2'],
                               host=True, pattern=r'GROUP2_CONTRACTS\|checks=\d+\|failures=0\n'))
            checks.append(dict(stem='Classic external completion guard admission', image=str(contracts),
                               expected='', stderr='', failure=False, cause='', argv=[str(runtime), 'hostfailure'],
                               host=True, pattern=r'GROUP2_HOST_GUARDS\|checks=\d+\|failures=0\n'))
        process = work / ('process fixture with spaces' + suffix)
        argv_process = work / ('argv fixture with spaces' + suffix)
        shutil.copy2(args.process_fixture, process); shutil.copy2(args.argv_fixture, argv_process)
        if not emit_only:
            shared = bcompile(fixtures / 'process-services.crexx', 'process-services')
            image = link('process-services', shared)
            checks.append(dict(stem='shared process', image=str(image), expected='EMPTY-ARGV\nCHILD-STDOUT\nARG1=alpha beta\nARG2=\nARG3=Gràdh 中 😀\nARG4=; && | $(touch NEVER) * ? [x] {y}\nPASS: shared process services\n', stderr='CHILD-STDERR\n', failure=False, cause='', argv=[str(process), str(argv_process)]))
            cancelled = bcompile(fixtures / 'cancel-process.crexx', 'cancel-process')
            checks.append(dict(stem='live process cancellation and same VM recovery', image=str(link('cancel-process', cancelled)),
                               expected='G2_CANCEL_PROCESS_OK\n', stderr='', failure=False, cause='', argv=[str(process)]))
        scalar = bcompile(fixtures / 'scalar-provider.crexx', 'scalar-provider')
        shutil.copy2(scalar, runtime / scalar.name)
        check('typed-preview', ccompile("say g2scalar.twice('21')\n", 'typed-preview',
              ('--provider', 'scalar-provider', 'g2scalar.twice')), '42\n', (scalar,))
        resolution_text = resolution.read_text(); resolution_oracle = resolution_expected.read_text()
        check('existing-call-resolution', ccompile(resolution_text, 'existing-call-resolution'), resolution_oracle)
        check('existing-recursive-call', ccompile(recursive.read_text(), 'existing-recursive-call'), '3\n')
        names = 'twice real decimal bytes flag value optional tail mutate touch observe noresult count length handler'.split()
        bindings = [part for name in names for part in ('--provider', 'scalar-provider', 'g2scalar.' + name)]
        collisions = [*bindings, '--executable', 'LENGTH', str(process)]
        assembly = ccompile((fixtures / 'scalar-c.crexx').read_text(), 'scalar-c', collisions)
        check('scalar-c', assembly, '42\n2.5\n0.3\n1\nGràdh 中 😀\nGràdh\x00中😀\nA/DEFAULT/0\nA//1\nA/DEFAULT/0\nP/0\nP/2/one/\n12\n5\n16\n1\nLIT\nLIT\n2\nLOCAL-LENGTH\n4\n5\nEXTERNAL-LENGTH\n', (scalar,))
        gsource = work / 'scalar-provider-g.crexx'
        gsource.write_text((fixtures / 'scalar-provider.crexx').read_text().replace('options levelb', 'options levelg', 1).replace('namespace g2scalar ', 'namespace g2scalarg ', 1), encoding='utf-8')
        gprovider = bcompile(gsource, 'scalar-provider-g'); shutil.copy2(gprovider, runtime / gprovider.name)
        check('g-provider-c', ccompile("say g2scalarg.twice('21')\n", 'g-provider-c', ('--provider', 'scalar-provider-g', 'g2scalarg.twice')), '42\n', (gprovider,))
        check('typed-handler', ccompile("call on notready name g2scalar.handler\ncall linein 'group2-input-that-does-not-exist'\nsay g2scalar.count()\n", 'typed-handler', bindings), '1\n', (scalar,))
        check('process-handler', ccompile("call on notready name child\ncall linein 'group2-input-that-does-not-exist'\nsay '['||result||']'\n", 'process-handler', ('--executable', 'child', str(process))), 'EMPTY-ARGV\n[]\n')
        if emit_only:
            declaration = app / 'packet-provider-declaration.rxas'
            declaration.write_text(signature_image(packet=True), encoding='utf-8')
            shutil.copy2(assemble(declaration), runtime / 'packet-provider.rxbin')
        packet_assembly = ccompile((fixtures / 'packet-provider.crexx').read_text(), 'packet-provider', option=('--routine', 'g2packet.run'), directory=runtime)
        packet = packet_assembly.with_suffix('.rxbin')
        packetbindings = ['--provider', 'packet-provider', 'g2packet.run']
        check('packet-c', ccompile("say 'g2packet.run'('probe',,'NUL'||'00'x)\ncall g2packet.run 'probe',,'tail'\nsay result\nsay '['||g2packet.run('empty')||']'\nsay g2packet.run('exit')\nsay 'continued'\n", 'packet-c', packetbindings), 'FUNCTION|probe|0|1|NUL\x00\nSUBROUTINE|probe|0|1|tail\n[]\nCHILD-EXIT\ncontinued\n', (packet,))
        check('packet-bare', ccompile("say g2packet.run('bare')\n", 'packet-bare', packetbindings), '', (packet,), failure=True, cause='45.1')
        halt_assembly = ccompile((fixtures / 'halt-c.crexx').read_text(), 'halt-c')
        host_assembly = ccompile((fixtures / 'host-c.crexx').read_text(), 'g2host', packetbindings, option=('--host', 'g2host.run'), directory=runtime)
        stream_assembly = ccompile((fixtures / 'host-stream-c.crexx').read_text(), 'g2hoststream',
                                  ('--executable', 'child', str(process)), ('--host', 'g2hoststream.run'), directory=runtime)
        check('void-expression', ccompile("say g2scalar.noresult()\n", 'void-expression', bindings), '', (scalar,), failure=True, cause='44.1')
        check('conversion', ccompile("say g2scalar.decimal(g2scalar.observe('bad'),g2scalar.observe('0.2'))\n", 'conversion', bindings), 'actual:bad\nactual:0.2\n', (scalar,), failure=True, cause='CONVERSION_ERROR')
        check('process-c', ccompile("RC='original'\nsay child('--status','7')\ncall child '--streams'\nsay result\ncall 'child' '--status','0'\nsay result\nsay rc\n", 'process-c', ('--executable', 'child', str(process))), '7\nCHILD-STDOUT\n7\n0\noriginal\n', stderr='CHILD-STDERR\n')
        check('process-argv', ccompile("say child('alpha beta','','Gràdh 中 😀','; && | $(touch NEVER) * ? [x] {y}')\n", 'process-argv', ('--executable', 'child', str(argv_process))), 'ARG1=alpha beta\nARG2=\nARG3=Gràdh 中 😀\nARG4=; && | $(touch NEVER) * ? [x] {y}\n0\n')
        check('process-nul', ccompile("say child('a'||'00'x||'b')\n", 'process-nul', ('--executable', 'child', str(process))), '', failure=True, cause='CHANNEL_ERROR')
        check('process-missing', ccompile("say child()\n", 'process-missing', ('--executable', 'child', str(process) + '.missing')), '', failure=True, cause='CHANNEL_ERROR')
        native = build / 'bin/providers/rx_hash.rxplugin'
        if not native.exists():
            native = build / 'bin/rx_hash.rxplugin'
        shutil.copy2(native, runtime / 'rx_hash.rxplugin')
        raw = 'Gràdh\x00中😀'
        assembly = ccompile("say rxhash.sha256hex('Gràdh'||'00'x||'中😀')\n", 'native-c', ('--native', 'rx_hash', 'rxhash.sha256hex'))
        check('native-c', assembly, hashlib.sha256(raw.encode()).hexdigest() + '\n')
        for stem, source, options in (
            ('required', 'if 0 then call g2scalar.twice\n', bindings),
            ('surplus', 'call g2scalar.twice 1,2\n', bindings),
            ('vararg-hole', "call g2scalar.tail 'P',,'x'\n", bindings),
            ('process-hole', "if 0 then call child ,1\n", ('--executable', 'child', str(process))),
            ('ambiguous', 'if 0 then call twice 1\n', (*bindings, '--executable', 'twice', str(process))),
            ('required-handler', 'call on error name g2scalar.twice\nnop\n', bindings),
        ):
            ccompile(source, stem, options, failure=True)
        selected = set()
        if emit_only:
            known = {stem for stem, _ in requests} | {'cli-options'}
            selected = set(args.emit_case) or known
            assert selected <= known, ('unknown emission request', sorted(selected - known))
            if selected & {'packet-c', 'packet-bare', 'g2host'}:
                selected.add('packet-provider')
            requests = [item for item in requests if item[0] in selected]
            negatives = [item for item in negatives if item[0] in selected]
        worker_source = fixtures / 'compile-fixtures.crexx'
        worker = host / 'compile-group2.rxas'
        run('warm worker compile', [tool('rxc'), '-x', '--no-exe-import', '-i', backend / 'imports-llrxc-c', '-o', worker, worker_source])
        worker_image = assemble(worker); worker_linked = host / 'compile-group2-linked.rxbin'
        control = work / 'compile-group2.ctl'
        compiler_inputs = sorted((backend / 'imports-llrxc-c').glob('*.rxbin'))
        control.write_text(''.join(f'INPUT {image}\n' for image in (worker_image, *compiler_inputs)) +
                           f'ROOT {worker_image.stem}\nOUTPUT {worker_linked.with_suffix("")}\n')
        run('warm worker link', [tool('rxlink'), '-s', '-c', control])
        order = {'minimal-c': 0, 'typed-preview': 1, 'scalar-c': 2,
                 'packet-provider': 3, 'g2host': 4, 'g2hoststream': 5}
        requests.sort(key=lambda item: order.get(item[0], 6))
        request_manifest = work / 'compile.manifest'
        request_manifest.write_text(''.join(row + '\n' for _, row in requests), encoding='utf-8')
        batch_log = work / 'compile-batch.log'
        print('GROUP2_BATCH|' + str(batch_log), flush=True)
        with batch_log.open('wb') as output:
            result = subprocess.run([str(hostvm), '--no-autoload', str(worker_linked), '-a', str(runtime), str(request_manifest), str(tool('rxas')), *(['emit-only'] if emit_only else [])],
                                    cwd=host, env=env, stdout=output, stderr=subprocess.STDOUT, timeout=1800)
        diagnostic = batch_log.read_bytes()
        if emit_only:
            marker = f'GROUP2_EMISSION|requests={len(requests)}|EMISSION'.encode()
            rows = diagnostic.replace(b'\r\n', b'\n').splitlines()
            assert result.returncode == 0 and rows.count(marker) == 1, ('warm emission batch', diagnostic[-6000:])
            emitted = [('EMITTED|' + row.split('|')[2]).encode() for _, row in requests if not row.split('|')[5]]
            assert sorted(row for row in rows if row.startswith(b'EMITTED|')) == sorted(emitted), 'exact emitted requests'
            assert not any(row.startswith((b'COMPILED|', b'GROUP2_FIXTURES|')) for row in rows), 'emission is not assembly qualification'
        else:
            assert result.returncode == 0 and b'|PASS\n' in diagnostic, ('warm compiler batch', diagnostic[-6000:])
        assert not re.search(rb'Warning in |warning:|#WARNING|Sanitizer|runtime error:', diagnostic), diagnostic[-6000:]
        # One CLI request covers option parsing; the warm service above owns
        # the behavioral cases without restarting the compiler per fixture.
        if not emit_only or 'cli-options' in selected:
            cli_source = work / 'cli-options.crexx'; cli_assembly = app / 'cli-options.rxas'
            if 'cli-options' in args.reuse_c:
                assert cli_source.read_bytes() == b'nop\n' and cli_assembly.exists(), 'reviewed unchanged CLI assembly'
            else:
                cli_source.write_text('nop\n', encoding='utf-8')
                run('combined CLI options', [hostvm, '--no-autoload', compiler, '-a', '-i', runtime, '-o', cli_assembly,
                    '--provider', 'scalar-provider', 'g2scalar.twice', '--native', 'rx_hash', 'rxhash.sha256hex',
                    '--executable', 'child', process, '--host', 'g2clismoke.run', cli_source], cwd=host)
            assert '.meta "g2clismoke.run"="b"' in cli_assembly.read_text(), 'CLI host export is published'
            if not emit_only and ('cli-options' not in args.reuse_c or not cli_assembly.with_suffix('.rxbin').exists()):
                assemble(cli_assembly)
        for stem, assembly in negatives:
            assert assembly.read_bytes() == b'previous output\n' and ('REFUSAL|' + stem + '.crexx|').encode() in diagnostic, (stem, diagnostic[-4000:])
        if not emit_only or 'scalar-c' in selected:
            typed_shapes((app / 'scalar-c.rxas').read_text())
        if not emit_only or 'minimal-c' in selected:
            assert not re.search(r'llcdynamic|llirexecute|llcgraph|llcfrontend|llccompiler', (app / 'minimal-c.rxas').read_text(), re.I), 'trivial static consumer has no dynamic/compiler/executor dependency'
        if not emit_only or 'native-c' in selected:
            assert '=".provider" "rx_hash"' in (app / 'native-c.rxas').read_text(), 'native manifest identity is emitted'
        if emit_only:
            if 'packet-provider' in selected:
                declarations = [row.strip() for row in packet_assembly.read_text(encoding='utf-8').splitlines()
                                if re.match(r'\.meta "g2packet\.run"=', row.strip())]
                assert declarations == [PACKET_DECLARATION], 'complete fresh provider declaration matches emission-only import'
            requests_identity = []
            for stem, row in requests:
                fields = row.split('|'); source, output = Path(fields[0]), Path(fields[1])
                if not fields[5]:
                    assert output.stat().st_size > 0, ('emitted RXAS is nonempty', stem)
                requests_identity.append(dict(stem=stem, row=row, source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
                                              rxas_sha256=hashlib.sha256(output.read_bytes()).hexdigest(), refusal=bool(fields[5])))
            if 'cli-options' in selected:
                requests_identity.append(dict(stem='cli-options', source_sha256=hashlib.sha256(cli_source.read_bytes()).hexdigest(),
                                              rxas_sha256=hashlib.sha256(cli_assembly.read_bytes()).hexdigest(),
                                              providers=['scalar-provider:g2scalar.twice', 'native:rx_hash:rxhash.sha256hex'],
                                              executable=str(process), host='g2clismoke.run', refusal=False))
            expected_starts = [('START|' + row.split('|')[2]).encode() for _, row in requests]
            assert [row for row in rows if row.startswith(b'START|')] == expected_starts, 'exact ordered request records'
            expected_refusals = [row.split('|')[2].encode() for _, row in requests if row.split('|')[5]]
            assert sorted(row.split(b'|', 2)[1] for row in rows if row.startswith(b'REFUSAL|')) == sorted(expected_refusals), 'exact refusal records'
            identity = dict(kind='EMISSION', compiler_mode='opt', semantic_ir='on', requests=requests_identity,
                            declaration_only_imports={'packet-provider.rxbin': PACKET_DECLARATION},
                            worker_sha256=hashlib.sha256(worker_linked.read_bytes()).hexdigest(),
                            vm_sha256=hashlib.sha256(hostvm.read_bytes()).hexdigest(),
                            cli_compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
                            imports={path.name: hashlib.sha256(path.read_bytes()).hexdigest() for path in runtime.iterdir() if path.is_file() and path.suffix in ('.rxbin', '.rxplugin')})
            (work / 'emission.json').write_text(json.dumps(identity), encoding='utf-8')
            print('GROUP2_EMISSION|requests=' + str(len(requests_identity)) + '|EMISSION|' + str(work), flush=True)
            return
        for stem, assembly, expected, extra, stderr, failure, cause in pending:
            checks.append(dict(stem=stem, image=str(link(stem, assembly.with_suffix('.rxbin'), extra)), expected=expected,
                               stderr=stderr, failure=failure, cause=cause, argv=[]))
        halt_image = link('halt-c', halt_assembly.with_suffix('.rxbin'))
        checks.append(dict(stem='real native interrupt enters compiled C HALT policy', image=str(halt_image),
                           expected='PASS: compiled Classic HALT ingress\n', stderr='', failure=False, cause='', argv=[],
                           command=[str(args.spans_fixture.resolve()), str(build / 'bin/library.rxbin'),
                                    str(build / 'interpreter/test_rxvml_spans.rxbin'), str(halt_image)]))
        host_image = host_assembly.with_suffix('.rxbin')
        host_consumer = bcompile(fixtures / 'host-consumer.crexx', 'host-consumer')
        checks.append(dict(stem='host modes and live windows', image=str(link('host-consumer', host_consumer, (host_image, packet, dynamic))), expected='G2_HOST_OK\n', stderr='', failure=False, cause='', argv=[]))
        stream_image = stream_assembly.with_suffix('.rxbin')
        stream_consumer = bcompile(fixtures / 'host-stream-consumer.crexx', 'host-stream-consumer')
        checks.append(dict(stem='configured process streams and recovery', image=str(link('host-stream-consumer', stream_consumer, (stream_image,))),
                           expected='CHILD-STDOUT\nG2_HOST_STREAMS_OK\n', stderr='CHILD-STDERR\n', failure=False, cause='',
                           argv=[str(process), str(work / 'input.bin'), str(work / 'output.bin'), str(work / 'error.bin')]))
        inputs = [*fixtures.glob('*.crexx'), resolution, resolution_expected, recursive]
        manifest.write_text(json.dumps(dict(checks=checks, inputs={str(path): hashlib.sha256(path.read_bytes()).hexdigest() for path in inputs})), encoding='utf-8')
        print('GROUP2_PREPARED|' + str(work), flush=True)
    if args.phase != 'prepare':
        prepared = json.loads(manifest.read_text())
        skipped = set(args.skip_check)
        known_checks = {item['stem'] for item in prepared['checks']}
        assert not skipped - known_checks, ('unknown prepared check stem', sorted(skipped - known_checks))
        for path, digest in prepared['inputs'].items():
            assert hashlib.sha256(Path(path).read_bytes()).hexdigest() == digest, ('changed prepared fixture', path)
        for item in prepared['checks']:
            if item['stem'] in skipped:
                continue
            command = item.get('command') or [hostvm if item.get('host') else vm, '--no-autoload', item['image']]
            if item['argv']:
                command += ['-a', *item['argv']]
            result = run(item['stem'], command, failure=item['failure'], silent=False)
            if item['failure']:
                assert item['cause'].encode() in result.stdout + result.stderr, (item['stem'], result.stdout, result.stderr)
                assert result.stdout.replace(b'\r\n', b'\n') == item['expected'].encode(), (item['stem'], result.stdout)
            else:
                stdout = result.stdout.replace(b'\r\n', b'\n')
                if item.get('pattern'):
                    assert re.fullmatch(item['pattern'].encode(), stdout), (item['stem'], stdout)
                else:
                    assert stdout == item['expected'].encode(), (item['stem'], stdout)
                assert result.stderr.replace(b'\r\n', b'\n') == item['stderr'].encode(), (item['stem'], result.stderr)
        print('GROUP2_CHECK|PASS|cases=' + str(len(prepared['checks']) - len(skipped)) +
              ('|reused=' + str(len(skipped)) if skipped else ''))
    if temporary is not None:
        temporary.cleanup()


if __name__ == '__main__':
    main()
