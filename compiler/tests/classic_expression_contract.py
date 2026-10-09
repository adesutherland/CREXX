"""Classic operator values, error identities, conditions and shared math modes."""
import argparse
import json
import subprocess
import tempfile
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('--bindir', type=Path, required=True)
p.add_argument('--noopt', action='store_true')
p.add_argument('--linked', action='store_true')
a = p.parse_args()
a.bindir = a.bindir.resolve()
work = Path(tempfile.mkdtemp(prefix='crexx-expression-contract-'))
commands = []


def run(argv, expected=0):
    log = work / f'command-{len(commands)}.log'
    commands.append([str(x) for x in argv])
    with log.open('wb') as f:
        result = subprocess.run(commands[-1], cwd=work, stdout=f,
                                stderr=subprocess.STDOUT, timeout=300)
    output = log.read_bytes()
    if result.returncode != expected:
        raise AssertionError(f'exit {result.returncode}, expected {expected}; {log}\n'
                             + output[-12000:].decode(errors='replace'))
    return output


def execute(stem, source, classic=True, expected_exit=0):
    case = work / stem
    case.mkdir()
    path = case / ('program.rexx' if classic else 'program.crexx')
    path.write_text(source)
    output = case / 'program'
    run([a.bindir / 'rxc', *(['-n'] if a.noopt else []), '-i', a.bindir,
         '-o', output, path])
    run([a.bindir / 'rxas', '-o', str(output) + '.rxbin', output])
    image = str(output) + '.rxbin'
    libraries = [a.bindir / (x + '.rxbin') for x in ('library', 'classlib')]
    if classic:
        libraries.append(a.bindir / 'rxfnsc.rxbin')
    if a.linked:
        image = str(output) + '_image.rxbin'
        run([a.bindir / 'rxlink', '-o', image, str(output) + '.rxbin', *libraries])
        libraries = []
    return run([a.bindir / 'rxvm', image, *libraries], expected_exit).decode()


try:
    positives = [
        ('-2**2', '4'), ('2**3**2', '64'), ('2**-2', '0.25'), ('0**0', '1'),
        ('1**9223372036854775808', '1'),
        ('1.0001**1000000000', '1.89174533E+43427'),
        ('1.0001**-1000000000', '5.28612380E-43428'),
        ('1+2*3-4/2', '5'), ('-7%3', '-2'), ('-7//3', '-1'),
        ("' 07 ' = 7", '1'), ("' + 6 ' = 6", '1'),
        ("' 07 ' == 7", '0'), ("'a'='a '", '1'),
        ("''='  '", '1'), ("''==' '", '0'),
        ("'a'>>'A'", '1'), ("'a'\\>>'A'", '0'),
        ("'a' || 'b' 'c'", 'ab c'), ("'a'('b')", 'ab'),
        ('\\0 & 1 | 0 && 1', '0'), ('1 && 0', '1'),
    ]
    source = 'options levelc\n' + ''.join(f'say {expr}\n' for expr, _ in positives)
    expected = ''.join(value + '\n' for _, value in positives)
    assert execute('values', source) == expected
    source = ('options levelb\nimport rexxvalue\n'
              + 'say .RexxValue("+ 6").compareEqual(.RexxValue("6")).asString()\n'
              + 'say .RexxValue(" + 6 ").compareEqual(.RexxValue("6"),1).asString()\nreturn 0\n')
    assert execute('common_value_guard', source) == '0\n1\n'
    source = ("options levelc\nsignal on lostdigits name caught\nnumeric digits 1\n"
              + "say '+ 6'+0\nsay '- 6'+0\nsay '+ 06'+0\nexit\n"
              + "caught:\nsay 'WRONG LOSTDIGITS'\nexit 98\n")
    assert execute('sign_blank_lostdigits', source) == '6\n-6\n6\n'
    source = ("options levelc\nsignal on lostdigits name caught\nnumeric digits 1\n"
              + "say '+ 67'+0\nexit 99\ncaught:\nsay condition('D')\nexit\n")
    assert execute('sign_blank_genuine_lostdigits', source) == '+ 67\n'
    # The option deliberately preserves typed B/G value/signal behavior.
    for level in ('b', 'g'):
        source = (f'options level{level} numeric_classic\n'
                  + 'say -3**2\nsay 2**3**2\nsay 7/2\nsay 7%2\nsay 7//2\nreturn 0\n')
        assert execute('mode_' + level, source, False) == '9\n64\n3.5\n3\n1\n'
        source = (f'options level{level} numeric_common\n'
                  + 'say -3**2\nsay 2**3**2\nreturn 0\n')
        assert execute('common_' + level, source, False) == '-9\n512\n'
        source = (f'options level{level} numeric_classic\n'
                  + 'numeric digits 8\na=2.9999 as .decimal\nb=1 as .decimal\n'
                  + 'call reduced a,b\nreturn 0\n'
                  + 'reduced: procedure = .void\nnumeric digits 3\n'
                  + 'arg expose a=.decimal, b=.decimal\nsay a%b\nsay a//b\n'
                  + 'a=a%b\nsay a\nsay 0.0**0\nreturn\n')
        assert execute('decimal_mode_' + level, source, False) == '2\n0.999\n2\n1\n'
        source = (f'options level{level} numeric_classic\nnumeric digits 50\n'
                  + 'a=1.0000000000000000000000000000000000000001 as .decimal\n'
                  + 'call reduced a\nreturn 0\n'
                  + 'reduced: procedure = .void\nnumeric digits 3\n'
                  + 'arg expose a=.decimal\nsay a**100000000000000000000.0\nreturn\n')
        assert execute('retained_power_' + level, source, False) == '1\n'
        source = (f'options level{level} numeric_classic\nnumeric digits 512\n'
                  + 'a=1.0000000000000000000000000000000000000001 as .decimal\n'
                  + 'b=100000000000000000000.' + '0' * 520 + ' as .decimal\n'
                  + 'call reduced a,b\nreturn 0\n'
                  + 'reduced: procedure = .void\nnumeric digits 3\n'
                  + 'arg expose a=.decimal,b=.decimal\nsay a**b\nreturn\n')
        assert execute('retained_exponent_' + level, source, False) == '1\n'
    negatives = [
        ("say 'x'+1", '41.1'), ("say 1+'x'", '41.2'),
        ("say +'x'", '41.3'), ("say -'x'", '41.3'),
        ('say 1/0', '42.3'), ('say 1%0', '42.3'), ('say 1//0', '42.3'),
        ('say 2**1.5', '26.8'), ('say 2&&1', '34.5'),
        ('say 1&&2', '34.6'), ('say 2&1', '34.5'), ('say 1&2', '34.6'),
        ('say 2|1', '34.5'), ('say 1|2', '34.6'), ('say \\2', '34.6'),
        ("if 2 then say 'BAD'", '34.1'),
        ("select; when 2 then say 'BAD'; otherwise nop; end", '34.2'),
        ('do while 2; nop; end', '34.3'), ('do until 2; nop; end', '34.4'),
        ('numeric digits 2; say 999%1', '26.11'),
        ('numeric digits 2; say 999//1', '26.12'),
        ('say 2**1e20', '42.1'), ('say 0.5**1e20', '42.2'),
    ]
    for i, (statement, code) in enumerate(negatives):
        source = ('options levelc\nsignal on syntax name caught\n' + statement
                  + "\nsay 'MISSED'\nexit 99\ncaught:\n"
                  + "say rc || '|' || sigl || '|' || condition('E')\n"
                  + "if .MN <> condition('E') | .SIGL <> sigl then exit 97\n"
                  + "if condition('C') <> 'SYNTAX' | condition('I') <> 'SIGNAL' then exit 98\nexit\n")
        expected = code.split('.')[0] + '|3|' + code + '\n'
        assert execute(f'error{i}', source) == expected, (statement, code)
    source = "options levelc\nsay 1/0\n"
    output = execute('untrapped', source, expected_exit=28)
    assert 'RXC-LC-42.3' in output and 'program.rexx:2:' in output, output
    output = execute('nul_diagnostic', "options levelc\n/* left\x00right */ say 1/0\n", expected_exit=28)
    assert 'program.rexx:2:' in output and '/* left\\0right */ say 1/0' in output, output
    source = """options levelc
flag=0
say 0 & mark()
say 1 | mark()
say 1 && mark()
say flag
exit
mark: procedure expose flag
flag=flag+1
return 1
"""
    assert execute('eager', source) == '0\n1\n0\n3\n'
    source = """options levelc
numeric digits 4
say 12345+0
numeric fuzz 1
say 1.234=1.235
call child
say digits() fuzz() form()
exit
child: procedure
numeric digits 6
numeric fuzz 2
numeric form engineering
return
"""
    assert execute('numeric', source) == '1.235E+4\n0\n4 1 SCIENTIFIC\n'
    source = """options levelc
numeric digits 3
say 1.234-1.233
say 2.9999%1
say 2.9999//1
say 999.9%1
exit
"""
    assert execute('rounding', source) == '0\n2\n0.999\n999\n'
    source = """options levelc
say=3
if=4
parse=5
say say if parse
say .5+1e-1
say 123abc 1a.b
say 'a''b' 'a"b'
say '[' || ''x || ']'
say c2x('F F0'x)
say c2x('1 1010'b)
say 1 +, /* continuation */
2
/* outer /* nested */ comment */
label1: ; label2: ; say 'labels'
exit
"""
    assert execute('lexical', source) == '3 4 5\n0.6\n123ABC 1A.B\na\'b a"b\n[]\n0FF0\n1A\n3\nlabels\n'
    source = """options levelc
signal next
exit 95
next:
if sigl <> .SIGL then exit 94
say 'signal-alias'
exit
"""
    assert execute('signal_alias', source) == 'signal-alias\n'
    bad_sources = [("say .unknown", '50.1'), ('say name.()', '51.1'),
                   ("say 'GG'x", '15.3'), ("say '12'b", '15.4'),
                   ("say '12 1'x", '15.1'), ("say 'unclosed", '6.2'),
                   ('/* unclosed', '6.1'), ('say 1 +', '35.1')]
    for i, (body, code) in enumerate(bad_sources):
        case = work / f'source_error{i}'
        case.mkdir()
        path = case / 'invalid.rexx'
        path.write_text('options levelc\n' + body + '\n')
        diagnostic = run([a.bindir / 'rxc', '--diagnostics', 'raw', '-i', a.bindir,
                          '-o', case / 'invalid', path], expected=2).decode()
        assert 'RXC-LC-' + code in diagnostic, (body, diagnostic)
    print(f'PASS: Classic expression contract, noopt={a.noopt}, linked={a.linked}')
finally:
    (work / 'commands.json').write_text(json.dumps(commands, indent=2))
    print(f'Expression evidence: {work}')
