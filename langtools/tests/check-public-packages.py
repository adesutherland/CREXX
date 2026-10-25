#!/usr/bin/env python3
"""Public package selection and the maintained RXAS guide's static consumer.

Use prepared products only. Exact import roots, an empty PATH and a copied VM
keep the independent core client and generated target outside compiler/SDK
runtime directories. The guide remains the single owner of its producer source.
"""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


GUIDE = Path(__file__).resolve().parents[1] / 'doc/rxas-design.md'
CORE = '''options levelb
import llir

main: procedure = .int
  schema = .llir..schema()
  contract = schema.finish()
  builder = .llir..builder(contract)
  at = builder.synthetic('independent public core consumer')
  fn = builder.function('main',contract.voidtype())
  entry = fn.block('entry')
  call entry.returnvoid(at)
  graph = builder.finish()
  if graph.error() <> '' then return 1
  say 'CORE|' || graph.functions() || '|' || graph.blocks() || '|' || graph.operations()
  return 0
'''


def guide_source():
    text = GUIDE.read_text(encoding='utf-8')
    section = text.split('## Construct one function\n', 1)
    assert len(section) == 2, 'maintained complete producer section'
    section = section[1].split('\n## ', 1)[0]
    blocks = re.findall(r'^```rexx\n(.*?)^```\s*$', section, re.M | re.S)
    assert len(blocks) == 1, 'one complete maintained producer'
    return blocks[0]


def inspect_target(text):
    lines = [line.strip() for line in text.splitlines() if line.strip()]
    assert lines[0] == 'main() .locals=4', ('four-cell public target', lines)
    instructions = [line.split(' ', 1)[0] for line in lines[1:]]
    assert instructions == ['load', 'icopy', 'itos', 'say', 'null',
                            'load', 'icopy', 'itos', 'say', 'ret'], (
        'real scalar conversion/output and one reuse reset', lines)
    first = re.fullmatch(r'load (r\d+),1', lines[1])
    second = re.fullmatch(r'load (r\d+),3', lines[6])
    assert first and second and first[1] == second[1], (
        'disjoint scalar temporaries share one cell', lines)
    assert lines[5] == 'null ' + first[1], ('reused cell is reset', lines)
    for offset in (1, 6):
        copied = re.fullmatch(r'icopy (r\d+),' + first[1], lines[offset + 1])
        assert copied, ('real scalar copy', lines)
        assert lines[offset + 2:offset + 4] == [
            'itos ' + copied[1], 'say ' + copied[1]], (
                'conversion and output use their owned result', lines)
    assert lines[-1] == 'ret 0', ('normal target completion', lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    args = parser.parse_args()
    build = args.build.resolve()
    tools = build / 'bin'
    suffix = '.exe' if os.name == 'nt' else ''
    # SAY uses native text-stream newlines; record contents remain exact.
    newline = b'\r\n' if os.name == 'nt' else b'\n'
    commands = 0
    with tempfile.TemporaryDirectory(prefix='crexx-public-packages-') as temporary:
        work = Path(temporary)
        empty = work / 'empty PATH'; empty.mkdir()
        runtime = work / 'VM only'; runtime.mkdir()
        vm = runtime / ('rxbvm' + suffix)
        shutil.copy2(tools / vm.name, vm)
        dll_names = set()
        for dll in tools.glob('*.dll'):
            shutil.copy2(dll, runtime / dll.name)
            dll_names.add(dll.name)
        env = dict(os.environ, PATH=str(empty))
        for name in ('CREXX_PROVIDER_PATH', 'CREXX_TRACE', 'CREXX_TRACE_TO'):
            env.pop(name, None)
        assert not any(shutil.which(name, path=env['PATH']) for name in ('rxc', 'rxc-c'))

        def run(command, *, cwd=work, refusal=False):
            nonlocal commands
            commands += 1
            completed = subprocess.run([str(value) for value in command],
                                       cwd=cwd, env=env, capture_output=True,
                                       timeout=180)
            if refusal:
                assert completed.returncode != 0, ('missing package admitted', command)
            else:
                assert completed.returncode == 0 and not completed.stderr, (
                    command, completed.returncode, completed.stdout, completed.stderr)
            return completed

        def imports(name, images):
            root = work / name; root.mkdir()
            for image in images:
                shutil.copy2(tools / (image + '.rxbin'), root)
            assert {path.name for path in root.iterdir()} == {
                image + '.rxbin' for image in images}, 'exact public image root'
            return root

        def compile_source(name, source, root):
            directory = work / name; directory.mkdir()
            path = directory / (name + '.crexx')
            path.write_text(source, encoding='utf-8')
            output = directory / name
            result = run([tools / ('rxc' + suffix), '-x', '--no-exe-import',
                          '-i', root, '-o', output, path])
            assert not result.stdout, ('warning-free public compilation', result.stdout)
            return output

        def assemble_link(output, images):
            run([tools / ('rxas' + suffix), output])
            linked = output.parent / (output.name + '-linked')
            run([tools / ('rxlink' + suffix), '-s', '-o', linked,
                 output.with_suffix('.rxbin'), *images])
            return linked.with_suffix('.rxbin')

        core = imports('core imports', ('library', 'rxfnsl'))
        output = compile_source('core-client', CORE, core)
        image = assemble_link(output, sorted(core.glob('*.rxbin')))
        isolated = runtime / 'core-client.rxbin'; shutil.copy2(image, isolated)
        actual = run([vm, '--no-autoload', isolated], cwd=runtime)
        assert actual.stdout == b'CORE|1|1|1' + newline, ('independent core-only graph', actual.stdout)
        isolated.unlink()

        # Each source uses a real public class. The core root cannot supply any
        # optional package, and executable-directory imports are disabled.
        for module, constructor in (
                ('llirgraph', '.llirgraph..index(reference graph)'),
                ('llrxas', '.llrxas..program(1,1)'),
                ('llirexecute', '.llirexecute..registry(contract)')):
            directory = work / ('missing-' + module); directory.mkdir()
            source = CORE.replace('import llir\n', 'import llir\nimport ' + module + '\n')
            source = source.replace('  return 0\n', '  probe = ' + constructor + '\n  return 0\n')
            path = directory / 'missing.crexx'; path.write_text(source, encoding='utf-8')
            result = run([tools / ('rxc' + suffix), '-x', '--no-exe-import',
                          '-i', core, '-o', directory / 'missing', path], refusal=True)
            diagnostic = result.stdout + result.stderr
            assert b'#CLASS_NOT_FOUND' in diagnostic and module.encode() in diagnostic, (
                'missing public package has a class-admission error', module, diagnostic)

        rxas = imports('RXAS imports', ('library', 'rxfnsl-rxas'))
        output = compile_source('guide-producer', guide_source(), rxas)
        image = assemble_link(output, sorted(rxas.glob('*.rxbin')))
        isolated = runtime / 'guide-producer.rxbin'; shutil.copy2(image, isolated)
        produced = run([vm, '--no-autoload', isolated], cwd=runtime).stdout
        inspect_target(produced.decode('utf-8'))
        isolated.unlink()

        # The output needs no library: its only dependencies are VM operations.
        # Assemble/link with explicit tools, then run beside only the copied VM.
        target = runtime / 'public-target'
        target.with_suffix('.rxas').write_bytes(produced)
        image = assemble_link(target, ())
        actual = run([vm, '--no-autoload', image], cwd=runtime)
        assert actual.stdout == b'1' + newline + b'3' + newline, ('hand-derived static result', actual.stdout)
        assert {path.name for path in runtime.iterdir()} == {
            vm.name, *dll_names, 'public-target.rxas', 'public-target.rxbin',
            'public-target-linked.rxbin'}, 'exact compiler/SDK-free target root'
    print(f'LEVEL_L_PUBLIC_PACKAGES|PASS|commands={commands}')


if __name__ == '__main__':
    main()
