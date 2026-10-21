#!/usr/bin/env python3
"""Independent label/arm ownership and local-entry controls; never builds.

One C source/lane serves output, private-owner host, reached17.1 and RXAS.
Current C17 schema and shared runtime remain unchanged. SIGNAL, condition
dispatch/TRACE and wider host/platform closure belong to their later owners.
"""
import argparse
import os
from pathlib import Path
import re
import runpy
import shlex
import shutil
import subprocess
import tempfile


ADDRESS = runpy.run_path(str(Path(__file__).resolve().parents[1] / 'address' / 'check-address.py'))
called = ADDRESS['called']
unit = ADDRESS['unit']
source_position = ADDRESS['source_position']


def block_map(body):
    labels = list(re.finditer(r'^(llb\d+):\s*$', body, re.M))
    return {label[1]: body[label.end():labels[i + 1].start() if i + 1 < len(labels) else len(body)]
            for i, label in enumerate(labels)}


def linear_path(label, blocks):
    order = list(blocks)
    following = dict(zip(order, order[1:]))
    seen = set(); parts = []
    while label not in seen:
        assert label in blocks, ('declared block', label)
        seen.add(label); text = blocks[label]; parts.append(text)
        if re.search(r'^\s*(?:brt|brf|brtf|beq|bne|bgt|bge|blt|ble|ret|exit|signal|signalorigin)(?:\s|$)', text, re.M):
            break
        branch = re.search(r'^\s*br\s+(llb\d+)\s*$', text, re.M)
        if branch:
            label = branch[1]
        elif re.search(r'^\s*br(?:\s|$)', text, re.M):
            break
        elif label in following:
            label = following[label]
        else:
            break
    return '\n'.join(parts)


def false_arm(body, source):
    _, line, _ = source_position(source, b'if FALSE_CONTROL then FALSE_ARM:')
    current = None; truth = []; offset = 0
    for row in body.splitlines(keepends=True):
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            if fields[4] == 'source-c.crexx':
                current = int(fields[5])
        if current == line:
            truth.extend((offset + item.end(), item[1].strip()) for item in called(row, 'llctarget.truth'))
        offset += len(row)
    assert len(truth) == 1, 'one truth operation controls the simple labelled IF'
    tail = body[truth[0][0]:]
    branch = re.search(r'^\s*brt\s+([^,\s]+),' + re.escape(truth[0][1]) + r'\s*\n\s*br\s+(\S+)\s*$', tail, re.M)
    assert branch and not called(tail[:branch.start()], 'llctarget.sayvalue'), 'truth branches before labelled payload execution'
    blocks = block_map(body)
    yes = linear_path(branch[1], blocks); no = linear_path(branch[2], blocks)
    assert '"UNREACHED false arm"' in yes and called(yes, 'llctarget.sayvalue'), 'true edge owns the actual payload'
    assert '"UNREACHED false arm"' not in no and '"after-false"' in no, 'false edge bypasses both prefix and payload'


def shapes(text, source, target):
    body = unit(text); false_arm(body, source)
    assert not called(body, 'llctarget.status') and not re.search(r'^\s*exit\s', body, re.M), 'local entries retain existing owned completion'
    assert not called(body, 'rexxclassicconfig.rexxclassicconfig.§factory'), 'no per-label configuration construction'
    assert called(body, 'rexxclassiccontext.rexxclassiccontext.bindpool'), 'private entry retains checked binding restoration'
    assert called(body, 'rexxactivation.rexxactivationarguments.enterprocedure'), 'prefix PRIVATE enters the existing activation guard'
    needle = b'procedure expose COUNT PAYLOAD'
    _, line, column = source_position(source, needle)
    records = []
    for row in body.splitlines():
        if not row.lstrip().startswith('.srcstep '):
            continue
        fields = shlex.split(row)
        if fields[4] == 'source-c.crexx' and int(fields[3]) in (17, 65) and (int(fields[5]), int(fields[6])) == (line, column):
            records.append(fields)
    assert records and all(int(fields[2]) > 0 for fields in records), 'first PROCEDURE retains real payload byte/scalar anchor and nonzero clause'
    definitions = re.findall(r'^\S+\([^\n]*\s\.locals=\d+[^\n]*\.expose=(llctarget\.[^\s]+)\s*$', target, re.M)
    assert len(definitions) == len(set(definitions)) == 36, 'unchanged target API'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
    parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
    parser.add_argument('--ir', choices=('on', 'off'), default='on')
    parser.add_argument('--phase', choices=('all', 'source', 'host', 'fatal', 'refusals', 'contracts', 'shapes'), default='all')
    parser.add_argument('--inspect', type=Path)
    args = parser.parse_args()
    build = args.build.resolve(); fixtures = Path(__file__).resolve().parent
    backend = build / 'research/level-l/compilers/backend' / args.mode
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    args.work.mkdir(parents=True, exist_ok=True); commands = 0
    with tempfile.TemporaryDirectory(prefix='labels-', dir=args.work) as temporary:
        work = Path(temporary); runtime = work / 'runtime'; runtime.mkdir()
        for name in libraries:
            root = backend if name == 'llctarget' else build / 'bin'
            shutil.copy2(root / (name + '.rxbin'), runtime / (name + '.rxbin'))
        dlls = tuple((build / 'bin').glob('*.dll')) if os.name == 'nt' else ()

        def tool(name):
            return build / 'bin' / (name + suffix)

        def stage(name, plugins):
            directory = work / name; directory.mkdir(); vm = directory / (args.vm + suffix)
            shutil.copy2(tool(args.vm), vm)
            for dll in dlls:
                shutil.copy2(dll, directory / dll.name)
            providers = directory / 'providers'; providers.mkdir()
            for plugin in plugins:
                shutil.copy2(build / 'bin' / (plugin + '.rxplugin'), providers / (plugin + '.rxplugin'))
            return directory, vm

        host, host_vm = stage('C compiler host', ('rxfs',))
        application, target_vm = stage('static application', ('rxfs', 'rxcstream'))
        compiler = host / 'llrxc-c-linked.rxbin'; shutil.copy2(backend / compiler.name, compiler)
        empty_path = work / 'empty PATH'; empty_path.mkdir()
        env = dict(os.environ, PATH=str(empty_path)); env.pop('CREXX_PROVIDER_PATH', None)
        assert not any(shutil.which(name, path=env['PATH']) for name in ('rxc', 'rxc-c'))

        def run(label, command, *, failure=False, silent=False, cwd=work):
            nonlocal commands
            completed = subprocess.run([str(item) for item in command], cwd=cwd, env=env, capture_output=True, timeout=900)
            commands += 1; diagnostic = completed.stdout + completed.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
            assert (completed.returncode != 0 if failure else completed.returncode == 0), (label, completed.returncode, diagnostic[-6000:])
            if silent:
                assert not diagnostic, (label, diagnostic[-4000:])
            return completed

        def compile_c(source, output, *, failure=False):
            return run('compile ' + source.name, [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []), '-i', runtime, '-o', output, source], failure=failure, silent=not failure, cwd=host)

        def inspect(path, name):
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, args.inspect / f'{name}-{args.mode}-{args.ir}-{args.vm}.rxas')

        if args.phase in ('all', 'source', 'host', 'fatal', 'shapes'):
            source = work / 'source-c.crexx'; data = (fixtures / source.name).read_bytes()
            source.write_bytes(data if args.mode == 'opt' else data.replace(b'\n', b'\r\n'))
            assembly = application / 'source.rxas'; compile_c(source, assembly)
            inspect(assembly, 'labels-source'); text = assembly.read_text()
            assert str(work) not in text and str(build) not in text, 'stable logical source identity'
            shapes(text, source.read_bytes(), (backend / 'llctarget.rxas').read_text())
            if args.phase in ('all', 'source'):
                run('source assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), assembly], silent=True)
                linked = application / 'source-linked'
                run('source link', [tool('rxlink'), '-o', linked, assembly.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)], silent=True)
                completed = run('required label arms', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
                assert completed.stdout.replace(b'\r\n', b'\n') == expected and not completed.stderr, (completed.stdout, completed.stderr, expected)
            if args.phase in ('all', 'host', 'fatal'):
                exported = application / 'source-exported.rxas'
                text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=labelfixture.run\2', text, count=1, flags=re.M)
                text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"labelfixture.run"', text, count=1, flags=re.M)
                assert headers == metadata == 1, 'one test-only context/entry0 export'
                exported.write_text(text, encoding='utf-8'); inspect(exported, 'labels-exported')
                run('exported assemble', [tool('rxas'), '-n', exported], silent=True)
                imports = work / 'B caller imports'; imports.mkdir()
                for provider in runtime.glob('*.rxbin'):
                    shutil.copy2(provider, imports / provider.name)
                shutil.copy2(exported.with_suffix('.rxbin'), imports / 'labelfixture.rxbin')
                caller = application / 'label-host'
                run('B host compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import', '-i', imports, '-o', caller, fixtures / 'host.crexx'], silent=True)
                run('B host assemble', [tool('rxas'), '-n', caller], silent=True)
                linked = application / 'label-host-linked'; control = work / 'host.ctl'
                inputs = [caller.with_suffix('.rxbin'), exported.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) + 'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run('B host link', [tool('rxlink'), '-c', control], silent=True)
                if args.phase in ('all', 'host'):
                    completed = run('fresh IF-prefix private entry', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], cwd=application)
                    assert re.fullmatch(rb'LABEL_HOST\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
                if args.phase in ('all', 'fatal'):
                    line = source.read_text().splitlines().index('if 1 then LATE: procedure') + 1
                    completed = run('natural late PROCEDURE', [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', 'late'], failure=True, cwd=application)
                    assert completed.stdout.replace(b'\r\n', b'\n') == b'LABEL_FATAL|before\n', completed.stdout
                    assert b'CLASSIC_SYNTAX' in completed.stderr and b'17.1' in completed.stderr and f'source-c.crexx:{line}:'.encode() in completed.stderr, completed.stderr
                    assert b'UNREACHED' not in completed.stdout

        if args.phase in ('all', 'refusals'):
            cases = (('then-label-only', 'if 0 then A: ; B:', 'then', b'14.3'),
                     ('else-label-only', 'if 0 then nop else A:', 'else', b'14.4'),
                     ('when-label-only', 'select; when 1 then A: ; end', 'then', b'14.3'),
                     ('unicode-label-only', "if '雪' = '雪' then A:", 'then', b'14.3'),
                     ('first-grouped', "do; LENGTH: nop; end; LENGTH: return; call LENGTH 'abc'", None, b'16.3'))
            output = application / 'prior.rxas'; prior = b'unchanged prior output\n'; output.write_bytes(prior)
            for name, content, token, code in cases:
                source = work / (name + '.crexx'); data = ('options levelc\n' + content + '\n').encode(); source.write_bytes(data)
                completed = compile_c(source, output, failure=True)
                assert code in completed.stdout and not completed.stderr, (name, completed.stdout, completed.stderr)
                if token:
                    offset, line, column = source_position(data, token.encode())
                    assert f'at line {line}, column {column} (byte {offset})'.encode() in completed.stdout, (name, completed.stdout)
                assert output.read_bytes() == prior and not Path(str(output) + '.llrxc-stage').exists(), 'failed label publication remains atomic'

        if args.phase in ('all', 'contracts'):
            probe = host / 'label-contracts-linked.rxbin'; shutil.copy2(backend / probe.name, probe)
            completed = run('label AST/inventory/IR contracts', [host_vm, '--no-autoload', probe, '-a', runtime], cwd=host)
            assert re.fullmatch(rb'LABEL_CONTRACTS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
    print(f'LABEL_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
