#!/usr/bin/env python3
"""Actual switch-VM signals with the native platform console boundary on a host."""
import argparse
import os
from pathlib import Path
import subprocess

p = argparse.ArgumentParser()
for name in ('assembler', 'vm', 'codec', 'work'):
    p.add_argument('--' + name, required=True)
a = p.parse_args()
work = Path(a.work).resolve()
work.mkdir(parents=True, exist_ok=True)
log = work / 'commands.log'
log.write_bytes(b'')
count = 0


def run(argv, **kwargs):
    global count
    result = subprocess.run(argv, cwd=work, stderr=subprocess.PIPE,
                            timeout=240, **kwargs)
    with log.open('ab') as out:
        out.write((repr(argv) + f' rc={result.returncode}\n').encode())
        out.write(result.stdout or b'')
        out.write(result.stderr)
    count += 1
    return result


def image(label, operation, caught=False):
    text = '.globals=0\nmain() .locals=8\n'
    if caught:
        text += '    sigbr caught,"UNICODE_ERROR"\n'
    text += operation + ('    ret 91\ncaught:\n    say "caught"\n' if caught else '')
    text += '    ret 0\n'
    source = work / (label + '.rxas')
    source.write_text(text)
    result = run([a.assembler, '-o', str(work / label), str(source)], stdout=subprocess.PIPE)
    assert result.returncode == 0, result.stderr
    return str(work / (label + '.rxbin'))


def output_operation(instruction, value):
    return ('    load r1,"stdout"\n    load r2,"w"\n    fopen r3,r1,r2\n'
            f'    load r4,{value}\n    {instruction} r3,r4\n')


bad = {
    'say': '    say "€"\n',
    'sayx': '    sayx "€"\n',
    'fwrite': output_operation('fwrite', '"€"'),
    'fwritecdpt': output_operation('fwritecdpt', '8364'),
}
for name, operation in bad.items():
    binary = image(name, operation)
    result = run([a.vm, binary], stdout=subprocess.PIPE)
    assert result.returncode == 9, (name, result.returncode, result.stderr)
    assert b'SIGNAL UNICODE_ERROR' in result.stderr, result.stderr
    caught = image(name + '-caught', operation, caught=True)
    result = run([a.vm, caught], stdout=subprocess.PIPE)
    assert result.returncode == 0, (name, result.returncode, result.stderr)
    decoded = subprocess.run([a.codec, 'decode'], input=result.stdout,
                             capture_output=True, check=True, timeout=240).stdout
    assert decoded == b'caught\n', (name, decoded)

valid = image('valid', '    say "Aé"\n' + output_operation('fwrite', '"Aé"')
              + output_operation('fwritecdpt', '233'))
result = run([a.vm, valid], stdout=subprocess.PIPE)
assert result.returncode == 0 and not result.stderr, result.stderr
assert result.stdout == bytes((0xc1, 0x51, 0x0a, 0xc1, 0x51, 0x51)), result.stdout

# Custom SAY callbacks retain the void ABI and their own output policy.
custom = image('custom', '    say "€"\n')
result = run([a.vm, custom], stdout=subprocess.PIPE,
             env=dict(os.environ, CREXX_TEST_CUSTOM_SAY='1'))
assert result.returncode == 0 and result.stdout == '€\n'.encode(), result.stderr

# A real read-only descriptor forces write/flush failure without a platform
# /dev/full dependency. Unbuffering makes the FWRITE failures synchronous.
sink = work / 'readonly-output'
sink.write_bytes(b'')
for name, operation in {
    'say': '    say "A"\n',
    'fwrite': output_operation('fwrite', '"A"'),
    'fwritecdpt': output_operation('fwritecdpt', '65'),
}.items():
    binary = image(name + '-io-error', operation)
    with sink.open('rb') as stream:
        result = run([a.vm, binary], stdout=stream,
                     env=dict(os.environ, CREXX_TEST_CONSOLE_UNBUFFERED='1'))
    assert result.returncode == 15, (name, result.returncode, result.stderr)
    assert b'SIGNAL NOTREADY' in result.stderr, result.stderr

# Default SAY must detect a deferred flush error as well.
with sink.open('rb') as stream:
    result = run([a.vm, str(work / 'say-io-error.rxbin')], stdout=stream)
assert result.returncode == 15 and b'SIGNAL NOTREADY' in result.stderr, result.stderr
print(f'PASS: {count} assemble/real-VM checks; terminal/caught console signals, native accents, custom SAY and I/O errors')
