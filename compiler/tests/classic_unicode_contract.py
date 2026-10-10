"""Level C byte ordinals, explicit Unicode algorithms and encoded streams."""
import argparse
import json
import os
import subprocess
import tempfile
import time
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('--bindir', type=Path, required=True)
p.add_argument('--noopt', action='store_true')
p.add_argument('--linked', action='store_true')
p.add_argument('--vm', default='rxvm', choices=('rxvm','rxbvm','rxtvm'))
a = p.parse_args()
a.bindir = a.bindir.resolve()
work = Path(tempfile.mkdtemp(prefix='crexx-unicode-contract-'))
commands = []
start = time.monotonic()


def run(argv, expected=0, stdin=None):
    log = work / f'command-{len(commands)}.log'
    commands.append([str(x) for x in argv])
    with log.open('wb') as f:
        result = subprocess.run(commands[-1], cwd=work, stdout=f,
                                stderr=subprocess.STDOUT, input=stdin, timeout=600)
    output = log.read_bytes()
    if result.returncode != expected:
        raise AssertionError(f'exit {result.returncode}, expected {expected}; {log}\n'
                             + output[-4000:].decode(errors='replace'))
    return output


def execute(stem, source, expected_exit=0, stdin=None, classic=True, raw_output=False, extra_libraries=()):
    case = work / stem
    case.mkdir()
    path = case / ('program.rexx' if classic else 'program.crexx')
    path.write_text(source, encoding='utf-8')
    output = case / 'program'
    run([a.bindir / 'rxc', *(['-n'] if a.noopt else []), '-s', work, '-i', a.bindir,
         '-o', output, path])
    run([a.bindir / 'rxas', '-o', str(output) + '.rxbin', output])
    image = str(output) + '.rxbin'
    libraries = [a.bindir / (x + '.rxbin') for x in ('library', 'classlib', 'rxfnsc', 'rxfnsg')]
    libraries.extend(extra_libraries)
    if a.linked:
        image = str(output) + '_image.rxbin'
        run([a.bindir / 'rxlink', '-o', image, str(output) + '.rxbin', *libraries])
        libraries = []
    result = run([a.bindir / a.vm, image, *libraries], expected_exit, stdin)
    return result if raw_output else result.decode('utf-8').replace('\r\n', '\n')


def literal(value):
    return "'" + str(value).replace("'", "''") + "'"


try:
    source_root = Path(__file__).parent / 'rexx_src'
    for name in ('levelc_unicode_symbols', 'levelc_unicode_services'):
        assert execute(name, (source_root / (name + '.rexx')).read_text(encoding='utf-8')) == (source_root / (name + '.expected')).read_text(encoding='utf-8')
    source = '''options levelc
all=xrange()
copy=reverse(reverse(all))
parse value copy with 1 whole 257 tail
say length(all) length(whole) length(tail)
say c2x(whole)
say c2x(bitand(all,copies('FF'x,256)))=c2x(all)
say c2x(bitor(all,copies('00'x,256)))=c2x(all)
say c2x(bitxor(all,copies('00'x,256)))=c2x(all)
say c2x(substr(all,129,128))
do n=0 to 255
 if c2d(d2c(n))<>n then exit 90
end
say 'ordinals'
say length('00FF'x || '🙂')
say translate('🙂é漢','漢🙂é','🙂é漢')
say substr('a🙂漢é',2,2) pos('é','a🙂漢é') reverse('a🙂漢é')
say 'é' == 'é'
exit
'''
    expected = '256 256 0\n' + bytes(range(256)).hex().upper() + '\n1\n1\n1\n' + bytes(range(128,256)).hex().upper() + '\nordinals\n3\n漢🙂é\n🙂漢 4 é漢🙂a\n0\n'
    assert execute('ordinals', source) == expected
    source = '''options levelc
ſ=11
ı=12
ɐ=13
say s i Ɐ
say value('ſ') value('ı') value('ɐ')
say symbol('ɐ') datatype('ɐ','S')
signal value 'ſTOP'
exit 99
stop:
say 'width'
call ɐlabel
exit
ⱯLABEL:
say 'call'
return
'''
    assert execute('case_width', source) == '11 12 13\n11 12 13\nVAR 1\nwidth\ncall\n'
    provider = work / '变量'
    provider.with_suffix('.rexx').write_text("options levelc\nɐ='provider'\nsay value('ɐ') symbol('ɐ') length('🙂')\nreturn\n",encoding='utf-8')
    run([a.bindir/'rxc',*(['-n'] if a.noopt else []),'--levelc-routine','-i',a.bindir,'-o',provider,provider.with_suffix('.rexx')])
    run([a.bindir/'rxas','-o',provider.with_suffix('.rxbin'),provider])
    source = "options levelc\ncall 变量\nsay 'imported'\nexit\n"
    assert execute('unicode_import',source,extra_libraries=(provider.with_suffix('.rxbin'),)) == 'provider VAR 1\nimported\n'
    byte_calls = ["c2x('€')", "c2d('€')", "bitand('a','€')", "bitor('€')", "bitxor('a','b','€')", "xrange('€','€')", "decode('€')", "encode('€','Latin1')", "decode('FF'x,'UTF-8')"]
    for i, call in enumerate(byte_calls):
        source = ("options levelc\nsignal on syntax name caught\nsay " + call + "\nexit 99\ncaught:\nsay rc sigl condition('E')\nexit\n")
        assert execute('byte_error_' + str(i), source) == '23 3 23.1\n', call
    output = execute('untrapped', "options levelc\nsay c2x('€')\n", expected_exit=28)
    assert 'RXC-LC-23.1' in output and 'program.rexx:2:' in output
    source = '''options levelc
say c2x(encode('€','ASCII','?' ))
say decode('FF41'x,'UTF-8','?')
say graphemesubstr('a',1,3,'🙂')
say graphemesubstr('a',1,, '🙂')
say isencodingsupported('not-an-encoding')
exit
'''
    assert execute('replacement_omission', source) == '3F\n?A\na🙂🙂\na\n0\n'
    raw = work / 'raw.dat'
    encoded = work / 'encoded.dat'
    malformed = work / 'malformed.dat'
    malformed.write_bytes(b'\xff')
    source = f'''options levelc
raw={literal(raw)}
encoded={literal(encoded)}
say stream(raw,'C','OPEN WRITE REPLACE')
say charout(raw,xrange())
say stream(raw,'C','QUERY WRITE POSITION')
say stream(raw,'C','CLOSE')
say stream(raw,'C','OPEN BOTH')
say chars(raw)
say c2x(charin(raw,1,256))
say chars(raw)
say stream(raw,'C','QUERY READ POSITION')
say charout(raw,'AB',2)
say c2x(charin(raw,1,4))
say length(charin(raw,9223372036854775807,0))
say charout(raw,,9223372036854775807)
say stream(raw,'C','QUERY WRITE POSITION')
say charout(raw)
say stream(encoded,'C','OPEN WRITE REPLACE ENCODING CP1252 CODEPOINTS')
say stream(encoded,'C','QUERY ENCODING NAME')
say lineout(encoded,'€é')
say charout(encoded)
say stream(encoded,'C','OPEN READ ENCODING Windows-1252')
say chars(encoded) lines(encoded)
say linein(encoded)
say chars(encoded) lines(encoded)
say stream(encoded,'C','CLOSE')
say qualify(raw) == raw
exit
'''
    expected = 'READY:\n0\n257\nREADY:\nREADY:\n256\n' + bytes(range(256)).hex().upper() + '\n0\n257\n0\n00414203\n0\n0\n9223372036854775807\n0\nREADY:\nWindows-1252\n0\n0\nREADY:\n3 1\n€é\n0 0\nREADY:\n1\n'
    assert execute('streams', source) == expected
    assert raw.read_bytes() == bytes([0,65,66,3]) + bytes(range(4,256))
    assert encoded.read_bytes() == b'\x80\xe9\n'
    for codec in ('UTF-8','UTF-16LE','UTF-16BE','UTF-32LE','UTF-32BE','ASCII','Latin1','Windows-1252','IBM437','IBM850','IBM1047'):
        path = work / (codec + '.dat')
        text = 'A\x00B' if codec == 'ASCII' else 'é\x00B'
        source = f'''options levelc
file={literal(path)}
say stream(file,'C','OPEN WRITE REPLACE ENCODING {codec}')
say charout(file,{literal(text)})
say lineout(file,'Z')
say charout(file)
say stream(file,'C','OPEN READ ENCODING {codec}')
say length(linein(file))
say charin(file,1,3) == {literal(text)}
say stream(file,'C','QUERY READ POSITION')
say stream(file,'C','CLOSE')
exit
'''
        assert execute('codec_' + codec, source) == 'READY:\n0\n0\n0\nREADY:\n4\n1\n4\nREADY:\n', codec
    source = f'''options levelc
signal on syntax name caught
say stream({literal(malformed)},'C','OPEN READ ENCODING UTF-8')
say linein({literal(malformed)})
exit 99
caught:
say rc condition('E')
exit
'''
    # OPEN READ does not consume input; the read performs strict validation.
    assert execute('strict_stream', source) == 'READY:\n23 23.1\n'
    source = f'''options levelc
say stream({literal(malformed)},'C','OPEN READ ENCODING UTF-8 ERROR REPLACE')
say linein({literal(malformed)})
say stream({literal(malformed)},'C','QUERY ENCODING ERROR')
exit
'''
    assert execute('replacement_stream', source) == 'READY:\n�\nREPLACE\n'
    source = '''options levelc
say stream('','C','OPEN READ ENCODING UTF-16LE')
say charin(,,1)
say linein()
say linein()
say chars()
exit
'''
    assert execute('default_transport', source, stdin='🙂tail\r\n漢\n'.encode('utf-16le')) == 'READY:\n🙂\ntail\n漢\n1\n'
    source = f'''options levelc
signal on notready name caught
say linein({literal(work / 'missing' / 'absent.dat')})
exit 99
caught:
say condition('C') (condition('D') == {literal(work / 'missing' / 'absent.dat')}) sigl
exit
'''
    assert execute('notready', source) == 'NOTREADY 1 3\n'
    source = """options levelc
say stream('','C','OPEN READ ENCODING UTF-8')
parse linein value
say value length(value)
exit
"""
    assert execute('parse_linein', source, stdin='é🙂\n'.encode()) == 'READY:\né🙂 2\n'
    source = f"""options levelc
handled=0
call on notready name caught
say '[' || linein({literal(work / 'missing' / 'absent.dat')}) || ']'
say handled
exit
caught:
handled=handled+1
say condition('C') condition('I') sigl
return
"""
    assert execute('call_notready', source) == '[]\nNOTREADY CALL 4\n1\n'
    source = """options levelc
parse linein value
say value length(value)
exit
"""
    assert execute('console_default', source, stdin='é🙂\n'.encode()) == 'é🙂 2\n'
    source = """options levelc
say stream('','C','OPEN READ ENCODING BYTE')
say c2x(charin(,,256))
exit
"""
    assert execute('console_raw', source, stdin=bytes(range(256))) == 'READY:\n' + bytes(range(256)).hex().upper() + '\n'
    source = """options levelc
say stream('','C','OPEN WRITE ENCODING CP1252')
say stream('','C','QUERY ENCODING')
say charout(,'€')
say stream('','C','QUERY WRITE POSITION')
say stream('','C','CLOSE')
say stream('')
exit
"""
    assert execute('console_output',source,raw_output=True).replace(b'\r\n',b'\n') == b'READY:\nWindows-1252\n\x800\n2\nREADY:\nUNKNOWN\n'
    lines_file = work / 'lines.dat'
    lines_file.write_bytes(b'a\r\nb\rc\nlast')
    source = f"""options levelc
file={literal(lines_file)}
say stream(file,'C','OPEN BOTH')
say lines(file) chars(file)
say lines(file,'C') chars(file,'I') (qualify() == '')
say '[' || linein(file,,0) || ']'
say linein(file) c2x(linein(file)) linein(file)
say lines(file) chars(file)
say '[' || linein(file) || ']'
say stream(file)
say linein(file,2) stream(file,'C','QUERY READ POSITION')
say stream(file,'C','QUERY EXISTS') == qualify(file)
say stream({literal(work/'absent.dat')},'C','QUERY EXISTS') == ''
say charout(file,,5) stream(file,'C','QUERY WRITE POSITION')
say lineout(file,,2) stream(file,'C','QUERY WRITE POSITION')
say lineout(file,'Z',2)
say c2x(charin(file,1,11))
exit
"""
    assert execute('line_edges',source) == 'READY:\n3 11\n3 1 1\n[]\na 620D63 last\n0 0\n[]\nNOTREADY\nb\rc 8\n1\n1\n0 5\n0 4\n0\n610D0A5A0A630A6C617374\n'
    for i, command in enumerate(('BOGUS','QUERY BOGUS','OPEN ENCODING BOGUS','OPEN ERROR BOGUS')):
        source = f"options levelc\nsignal on syntax name caught\nsay stream('unopened','C',{literal(command)})\nexit 99\ncaught:\nsay rc sigl\nexit\n"
        assert execute('command_error_'+str(i),source) == '40 3\n'
    source = "options levelc\nsignal on syntax name caught\nsay charin('a' || '00'x || 'b')\nexit 99\ncaught:\nsay rc sigl\nexit\n"
    assert execute('nul_name',source) == '40 3\n'
    source = "options levelc\nsignal on syntax name caught\nsay charin('',1)\nexit 99\ncaught:\nsay rc sigl condition('E')\nexit\n"
    assert execute('transient_position',source,stdin=b'a\n') == '40 3 40.42\n'
    source = "options levelc\nsignal on syntax name caught\nsay linein('',,2)\nexit 99\ncaught:\nsay rc sigl condition('E')\nexit\n"
    assert execute('line_count_error',source) == '40 3 40.39\n'
    source = "options levelc\nsignal on syntax name caught\nsay encode('a','BOGUS')\nexit 99\ncaught:\nsay rc sigl condition('E')\nexit\n"
    assert execute('unicode_option_error',source) == '40 3 40\n'
    for encoding in ('ASCII','BYTE'):
        path = work / ('atomic-'+encoding+'.dat')
        path.write_bytes(b'AB')
        source = f"""options levelc
signal on syntax name caught
file={literal(path)}
say stream(file,'C','OPEN BOTH ENCODING {encoding}')
say charout(file,'€',1)
exit 99
caught:
say rc sigl condition('E')
say stream(file,'C','QUERY WRITE POSITION')
exit
"""
        assert execute('atomic_'+encoding,source) == 'READY:\n23 5 23.1\n3\n'
        assert path.read_bytes() == b'AB'
    bom = work / 'bom.dat'
    bom.write_bytes(b'\xef\xbb\xbfA')
    source = f"options levelc\nsay stream({literal(bom)},'C','OPEN READ ENCODING UTF-8')\nsay length(charin({literal(bom)},,2))\nsay charin({literal(bom)},1,1) == '﻿'\nexit\n"
    assert execute('bom',source) == 'READY:\n2\n1\n'
    source = """options levelb
import rexxvalue
import rexxclassicconfig
import rexxclassicbifs
import rexxclassicdatatype
import rexxclassicencoding
import rexxclassicbifx2c
import rexxclassicbifx2b
import rexxclassicbifx2d
import rexxclassicbifb2x
config = .RexxClassicConfig('UTF8')
call config.setTextDatatypeDigits('٠١٢٣٤٥٦٧٨٩')
call config.setTextOtherBlanks('·')
args = .RexxValue[]
exists = .int[]
exists[1] = 1
args[1] = .RexxValue('٠١·٠٢')
context = .RexxBifCallContext('X2C')
call context.setConfig(reference config)
call context.setArguments(args,exists)
result = rexxclassicbif_x2c(reference context)
say rexxclassic_bytes_to_hex(result.asBinary()) context.hasError()
context = .RexxBifCallContext('X2B')
call context.setConfig(reference config)
call context.setArguments(args,exists)
result = rexxclassicbif_x2b(reference context)
say result.asString() context.hasError()
args[1] = .RexxValue('٠١')
context = .RexxBifCallContext('X2D')
call context.setConfig(reference config)
call context.setArguments(args,exists)
result = rexxclassicbif_x2d(reference context)
say result.asString() context.hasError()
args[1] = .RexxValue('٠٠٠١·٠٠١٠')
context = .RexxBifCallContext('B2X')
call context.setConfig(reference config)
call context.setArguments(args,exists)
result = rexxclassicbif_b2x(reference context)
say result.asString() context.hasError()
byte_config = .RexxClassicConfig()
call byte_config.setByteDatatypeDigits('80818283848586878889'x as .binary)
call byte_config.setByteOtherBlanks('09'x as .binary)
args[1] = .RexxValue.fromBinary('8081098082'x as .binary)
context = .RexxBifCallContext('X2C')
call context.setConfig(reference byte_config)
call context.setArguments(args,exists)
result = rexxclassicbif_x2c(reference context)
say rexxclassic_bytes_to_hex(result.asBinary()) context.hasError()
return 0
"""
    assert execute('radix_validator_parity',source,classic=False) == '0102 0\n0000000100000010 0\n1 0\n12 0\n0102 0\n'
    source = f"""options levelb
import rexxpool
import rexxclassicstream
old = .RexxVariablePool()
modern = .RexxVariablePool(1)
say old.normalizeName('ſ') modern.normalizeName('ſ')
first = .RexxClassicStream({literal(encoded)},'READ','Windows-1252')
second = .RexxClassicStream({literal(encoded)},'READ','Windows-1252')
say first.readCharacters(-1,1) second.readCharacters(-1,1)
say first.readCharacters(-1,1)
say first.readPosition() second.readPosition()
alias = first
call alias.close()
say first.ready() second.ready()
say second.readCharacters(-1,1)
return 0
"""
    assert execute('ownership_byte_policy', source, classic=False) == 'ſ S\n€ €\né\n3 2\n0 1\né\n'
    invalid = work / 'invalid_import'
    invalid.mkdir()
    (invalid / 'bad.crexx').write_text("options levelb\nnamespace bad expose letter\nconstant data='00'x\nletter: procedure=.int\nreturn 1\n")
    (invalid / 'main.crexx').write_text("options levelb\nimport bad\nsay letter()\n")
    output = run([a.bindir/'rxc','--no-exe-import','-i',a.bindir,'-o',invalid/'main',invalid/'main.crexx'],expected=2).decode()
    assert 'CONSTANT_OUTSIDE_ROUTINE' in output and 'bad.crexx @ 3:1' in output
    print(f'PASS: Classic Unicode contract ({len(commands)} commands, {time.monotonic()-start:.2f}s); {work}')
finally:
    (work / 'commands.json').write_text(json.dumps(commands, indent=2))
