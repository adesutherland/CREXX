#!/usr/bin/env python3
"""Finite static-language crosswalk in one prepared C-opt/rxbvm component lane."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


# This independent name inventory is the maintained language contract, rather
# than a catalogue discovered from the implementation being checked.
BIFS = '''ABBREV ABS ADDRESS ARG B2X BITAND BITOR BITXOR C2D C2X CENTER CENTRE
CHANGESTR COMPARE CONDITION COPIES COUNTSTR D2C D2X DATATYPE DATE DELSTR DELWORD
DIGITS ERRORTEXT FORM FORMAT FUZZ INSERT LASTPOS LEFT LENGTH LOWER MAX MIN
OVERLAY POS QUEUED RANDOM REVERSE RIGHT SIGN SOURCELINE SPACE STRIP SUBSTR SUBWORD
SYMBOL TIME TRACE TRANSLATE TRUNC UPPER VALUE VERIFY WORD WORDINDEX WORDLENGTH
WORDPOS WORDS X2B X2C X2D XRANGE CHARIN CHAROUT CHARS LINEIN LINEOUT LINES QUALIFY
STREAM UNICODEVERSION TONFD TONFC TONFKD TONFKC ISNFD ISNFC ISNFKD ISNFKC
TOUPPERCASE TOLOWERCASE TOCASEFOLD TOSIMPLECASEFOLD TOTURKICCASEFOLD
TOTURKICSIMPLECASEFOLD GRAPHEMECOUNT GRAPHEMEREVERSE GRAPHEMESUBSTR GRAPHEMEPOS
ENCODE DECODE ISDECODABLE ISENCODINGSUPPORTED'''.split()
assert len(BIFS) == len(set(BIFS)) == 95


def error_panel(cases, prelude=(), base_index=0):
    """A single local activation/handler serves every independently named case."""
    rows = ['options levelc', *prelude, f'do probe={base_index + 1} to {base_index + len(cases)}', '  call check', 'end',
            "say 'ERROR_PANEL_OK'", 'exit 0', 'check:', 'signal on syntax name caught', 'select']
    first_case = len(rows) + 1
    expected = []
    for index, (expression, code) in enumerate(cases, base_index + 1):
        rows.append(f'  when probe={index} then answer={expression}')
        expected.append(f'{index}|{code}\n')
    rows += ['end', "say 'MISSED_ERROR' probe", 'exit 99', 'caught:',
             "if condition('C')\\=='SYNTAX' | condition('I')\\=='SIGNAL' then exit 98",
             f'if sigl\\={first_case}+probe-{base_index + 1} then exit 97',
             "if .MN\\==condition('E') | .SIGL\\=sigl then exit 96",
             "say probe || '|' || condition('E')", 'return']
    return '\n'.join(rows) + '\n', ''.join(expected) + 'ERROR_PANEL_OK\n'


def arity_panel(names, base_index):
    # Every bounded BIF has fewer than eight formal operands. Deliberately
    # supply eight present values, so arity precedes domain/host side effects.
    # MAX/MIN are unbounded and instead require at least one actual operand.
    return error_panel([(name + ('()' if name in ('MAX', 'MIN') else "('x','x','x','x','x','x','x','x')"),
                         '40.3' if name in ('MAX', 'MIN') else '40.4') for name in names], base_index=base_index)


def cmake_oracle(root, stem):
    """Reuse the checked native runtime expectation, without native execution."""
    text = (root / 'compiler/tests/CMakeLists.txt').read_text()
    found = re.search(r'add_levelc_runtime_slice_test\(' + re.escape(stem) +
                      r'\s+rexx_src/' + re.escape(stem) + r'\.rexx\s+"((?:[^"\\]|\\.)*)"', text)
    assert found, ('maintained checked native expectation', stem)
    return re.sub(r'\\([nrt"\\])', lambda m: {'n': '\n', 'r': '\r', 't': '\t', '"': '"', '\\': '\\'}[m[1]], found[1])


def source_shapes(text, *, simple=False):
    calls = re.findall(r'^\s*call\s+[^,]+,([^,(]+)\(', text, re.M)
    assert not any(name.startswith(('llccompiler.', 'llbinmeta.', 'llcsrcmap.', 'rxcontract.')) for name in calls), 'static C has no compiler/decoder call'
    if simple:
        assert not any(name.startswith(('llctarget.binaryop', 'llctarget.unaryop', 'llctarget.poolread', 'rexxclassicbif')) for name in calls), 'constant expression has no dynamic value/BIF path'
        assert calls.count('llctarget.poolnew') <= 1, 'one admitted root pool owner, no per-expression owners'
        assert not any(name.endswith(('.clearsourcelines', '.appendsourceline')) for name in calls), 'unobservable source inventory has no setup'
    else:
        assert any(name == 'llctarget.binaryop' for name in calls), 'nonconstant numeric operation retains its shared runtime owner'
        assert any(name.startswith('rexxclassicbifsubstr.') for name in calls), 'checked direct BIF entry remains visible'
        for required in ('rexxclassicbifs.rexxbifcallcontext.haserror', 'rexxclassicbifs.rexxbifcallcontext.errordetail',
                         'rexxclassicbifs.rexxbifcallcontext.conditionid', 'rexxclassicbifs.rexxbifcallcontext.conditiondescription',
                         'rexxactivation.rexxactivationarguments.queuecallcondition', 'rexxactivation.rexxactivationarguments.signalpolicy'):
            assert required in calls, ('existing checked BIF error/condition transport', required)
        assert 'signal "CLASSIC_SYNTAX"' in text, 'authored BIF syntax path remains explicit'
        assert calls.count('llctarget.poolnew') <= 1, 'BIF operands share the root pool owner'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--phase', choices=('all', 'preview', 'prepare', 'run', 'emit'), default='all')
    parser.add_argument('--reuse-c', action='append', default=[])
    parser.add_argument('--skip-check', action='append', default=[])
    parser.add_argument('--emit-case', action='append', default=[], help='Select maintained request stems only in --phase emit')
    parser.add_argument('--ir', choices=('on', 'off'), help='Semantic IR selection only for --phase emit (default: on)')
    args = parser.parse_args()
    emit_only = args.phase == 'emit'
    if args.ir and not emit_only:
        parser.error('--ir requires --phase emit')
    semantic_ir = args.ir or 'on'
    if args.emit_case and not emit_only:
        parser.error('--emit-case requires --phase emit')
    if emit_only and args.reuse_c:
        parser.error('--phase emit compiles current requests; --reuse-c is functional preparation only')
    if args.skip_check and args.phase != 'run':
        parser.error('--skip-check requires a prepared --phase run')
    root = Path(__file__).resolve().parents[5]
    fixtures = Path(__file__).resolve().parent
    native = root / 'compiler/tests/rexx_src'
    build = args.build.resolve(); work = args.work.resolve(); work.mkdir(parents=True, exist_ok=True)
    temporary = None
    if args.phase == 'all':
        temporary = tempfile.TemporaryDirectory(prefix='group3-', dir=work)
        work = Path(temporary.name)
    if emit_only:
        # Keep emission inputs/products apart from functional preparation.
        work = work / 'emission'; work.mkdir(exist_ok=True)
    backend = build / 'research/level-l/compilers/backend/opt'
    suffix = '.exe' if os.name == 'nt' else ''
    runtime = work / 'runtime'; app = work / 'application'; host = work / 'host'; empty = work / 'empty PATH'
    for directory in (runtime, app, host, empty):
        directory.mkdir(exist_ok=True)
    env = dict(os.environ, PATH=str(empty)); env.pop('CREXX_PROVIDER_PATH', None)
    vm = app / ('rxbvm' + suffix); hostvm = host / vm.name
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    manifest = work / 'prepared.json'
    requests = []; pending = []; negatives = []; checks = []; inputs = {Path(__file__), fixtures / 'compile-fixtures.crexx'}
    positive_sources = []

    def tool(name):
        return build / 'bin' / (name + suffix)

    def run(label, command, *, cwd=app, failure=False, silent=True, timeout=1200, stdin=None):
        result = subprocess.run([str(item) for item in command], cwd=cwd, env=env,
                                input=stdin, capture_output=True, timeout=timeout)
        diagnostic = result.stdout + result.stderr
        assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-5000:])
        assert (result.returncode != 0 if failure else result.returncode == 0), (label, result.returncode, diagnostic[-8000:])
        if silent:
            assert not diagnostic, (label, diagnostic[-5000:])
        return result

    def assemble(path):
        run('assemble ' + path.name, [tool('rxas'), '-o', path.with_suffix('.rxbin'), path])
        return path.with_suffix('.rxbin')

    def link(stem, image, extra=(), *, compiler=False):
        output = (host if compiler else app) / (stem + '-linked')
        control = work / (stem + '.ctl')
        members = sorted((backend / 'imports-llrxc-c').glob('*.rxbin')) if compiler else [runtime / (name + '.rxbin') for name in libraries]
        control.write_text(''.join(f'INPUT {item}\n' for item in [image, *extra, *members]) + f'ROOT {image.stem}\nOUTPUT {output}\n')
        run('link ' + stem, [tool('rxlink'), '-s', '-c', control])
        return output.with_suffix('.rxbin')

    def bcompile(source, stem, *, compiler=False):
        isolated = work / 'typed-sources' / stem; isolated.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, isolated / source.name)
        assembly = (host if compiler else app) / (stem + '.rxas')
        imports = backend / 'imports-llrxc-c' if compiler else runtime
        run('typed consumer ' + stem, [tool('rxc'), '-x', '--no-exe-import', '-i', imports, '-o', assembly, isolated / source.name])
        return assemble(assembly)

    def request(source, stem, *, expected=None, host_export='', refusal='', limits=(1000000, 512, 67108864, 65536), offset=None, logical=None, contains=()):
        payload = source.encode() if isinstance(source, str) else source
        sourcefile = work / (stem + '.rexx')
        if stem in args.reuse_c:
            assert sourcefile.exists() and sourcefile.read_bytes() == payload, ('unchanged prepared C source', stem)
        else:
            sourcefile.write_bytes(payload)
        assembly = (runtime if host_export else app) / (stem + '.rxas')
        if refusal:
            assembly.write_bytes(b'previous output\n'); negatives.append((stem, assembly, refusal, offset, contains))
        elif expected is not None:
            pending.append((stem, assembly, expected))
        if not refusal:
            positive_sources.append(payload.decode('utf-8'))
        if stem in args.reuse_c:
            assert not refusal and assembly.exists(), ('reviewed prepared C assembly', stem)
            return assembly
        fields = [str(sourcefile), str(assembly), logical or sourcefile.name, host_export, refusal,
                  *(str(value) for value in limits), str(int(semantic_ir == 'on')) if emit_only else '']
        assert len(fields) == 10 and not any('|' in value or '\n' in value for value in fields)
        requests.append((stem, '|'.join(fields)))
        return assembly

    def native_case(stem):
        source = native / (stem + '.rexx'); inputs.add(source)
        expected = source.with_suffix('.expected')
        if expected.exists():
            inputs.add(expected); oracle = expected.read_text()
        else:
            inputs.add(root / 'compiler/tests/CMakeLists.txt'); oracle = cmake_oracle(root, stem)
        return request(source.read_bytes(), stem, expected=oracle)

    def preview_cases():
        request('options levelc\nsay 1\n', 'simple-constant', expected='1\n')
        request("options levelc\na=arg(1)\nsay a**2\nsay substr('abc',2)\n", 'effectful-preview', expected='0\nbc\n')
        request("options levelc\nsay 'A\x00é'\nsay sourceline(2)\n", 'source-preview', expected="A\x00é\nsay 'A\x00é'\n")

    if args.phase != 'run':
        for name in libraries:
            origin = backend if name == 'llctarget' else build / 'bin'
            shutil.copy2(origin / (name + '.rxbin'), runtime / (name + '.rxbin'))
        for directory in (host, app):
            shutil.copy2(tool('rxbvm'), directory / vm.name)
            providers = directory / 'providers'; providers.mkdir(exist_ok=True)
            for alias in ('rxfs', 'rxcstream', 'rxcontract', 'rx_fs', 'rx_cstream', 'rx_contract'):
                for source in (build / 'bin' / (alias + '.rxplugin'), build / 'bin/providers' / (alias + '.rxplugin')):
                    if source.exists():
                        shutil.copy2(source, providers / source.name); shutil.copy2(source, runtime / source.name)
            if os.name == 'nt':
                for source in (build / 'bin').glob('*.dll'):
                    shutil.copy2(source, directory / source.name); shutil.copy2(source, runtime / source.name)
        preview_cases()
        if args.phase != 'preview':
            for stem in ('levelc_bif_reference_audit', 'levelc_unicode_services', 'levelc_unicode_symbols',
                         'levelc_unicode_character_bifs', 'levelc_bif_numeric_context', 'levelc_bif_whole',
                         'levelc_bif_integer_limits', 'levelc_bif_errortext', 'levelc_bif_sourceline',
                         'levelc_bif_queued', 'levelc_numeric_whole', 'levelc_numeric_invalid_context', 'levelc_if_instruction',
                         'levelc_options_instruction', 'levelc_options_dynamic_first', 'levelc_options_source',
                         'levelc_slice45_constant_symbols', 'levelc_slice42_abuttal', 'levelc_say_pool_reads',
                         'levelc_latin1_ordinals', 'levelc_literal_ordinals', 'levelc_translate_implicit',
                         'levelc_bif_config_lifecycle'):
                native_case(stem)
            assignment = fixtures.parent / 'assignment/source-c.crexx'
            inputs.update((assignment, assignment.with_suffix('.expected')))
            request(assignment.read_bytes(), 'shared-pool-stem', expected=assignment.with_suffix('.expected').read_text())
            # Keep all95 exact cases, IDs and state/error oracles while bounding
            # each generated IR owner. A single huge SELECT magnifies the
            # separately owned VM reference/construction cost.
            for start in range(0, len(BIFS), 10):
                content, oracle = arity_panel(BIFS[start:start + 10], start)
                request(content, f'all95-errors-{start // 10 + 1:02d}', expected=oracle)
            numeric_errors = [("'x'+1", '41.1'), ("1+'x'", '41.2'), ("+'x'", '41.3'), ("-'x'", '41.3'),
                              ('1/0', '42.3'), ('1%0', '42.3'), ('1//0', '42.3'), ('2**1.5', '26.8'),
                              ('2&&1', '34.5'), ('1&&2', '34.6'), ('2&1', '34.5'), ('1&2', '34.6'),
                              ('2|1', '34.5'), ('1|2', '34.6'), ('\\2', '34.6')]
            content, oracle = error_panel(numeric_errors); request(content, 'numeric-errors', expected=oracle)
            family_errors = [("substr('abc',0)", '40.14'), ("word('a',0)", '40.14'),
                             ("left('a',-1)", '40.13'), ("left('a',2,'xx')", '40.23'),
                             ("abbrev('a',,'')", '40.5'), ("c2x('€')", '23.1'), ("c2d('€')", '23.1'),
                             ("bitand('a','€')", '23.1'), ("bitor('€')", '23.1'), ("bitxor('a','b','€')", '23.1'),
                             ("xrange('€','€')", '23.1'), ("decode('€')", '23.1'),
                             ("encode('€','Latin1')", '23.1'), ("decode('FF'x,'UTF-8')", '23.1')]
            content, oracle = error_panel(family_errors); request(content, 'family-errors', expected=oracle)
            values = [('-2**2', '4'), ('2**3**2', '64'), ('-2**3**2', '64'), ('2**-2', '0.25'), ('0**0', '1'),
                      ('1**9223372036854775808', '1'), ('-9%4', '-2'), ('-9//4', '-1'), ('9//-4', '1'),
                      ('1.0001**1000000000', '1.89174533E+43427'), ('1.0001**-1000000000', '5.28612380E-43428'),
                      ('1+2*3-4/2', '5'), ("' 07 '=7", '1'), ("' 07 '==7", '0'), ("'a'='a '", '1'),
                      ("'a'\\='b'", '1'), ("'a'<>'b'", '1'), ("'a'><'b'", '1'),
                      ("'a'<'b'", '1'), ("'b'>'a'", '1'), ("'a'<='a'", '1'), ("'b'>='a'", '1'),
                      ("'a'\\>'b'", '1'), ("'b'\\<'a'", '1'), ("'a'<<'b'", '1'),
                      ("''==' '", '0'), ("'a'>>'A'", '1'), ("'a'\\>>'A'", '0'), ("'a'\\<<'A'", '1'),
                      ("'a'>>='a'", '1'), ("'a'<<='a'", '1'), ("'a'\\=='a'", '0'),
                      ("'é'>'z'", '1'), ("'é'>>'z'", '1'),
                      ("'a'||'b' 'c'", 'ab c'), ('\\0 & 1 | 0 && 1', '0')]
            request('options levelc\n' + ''.join('say ' + expr + '\n' for expr, _ in values),
                    'expression-values', expected=''.join(value + '\n' for _, value in values))
            request("options levelc\nflag=0\nsay 0 & mark()\nsay 1 | mark()\nsay 1 && mark()\nsay flag\nexit\nmark: procedure expose flag\nflag=flag+1\nreturn 1\n",
                    'eager-logical', expected='0\n1\n0\n3\n')
            request("options levelc\nnumeric digits 3\nsay 1.234-1.233\nsay 2.9999%1\nsay 2.9999//1\nsay 999.9%1\n",
                    'numeric-rounding', expected='0\n2\n0.999\n999\n')
            content, oracle = error_panel([('999%1', '26.11'), ('999//1', '26.12'), ('2**1e20', '42.1'), ('0.5**1e20', '42.2')], ('numeric digits 2',))
            request(content, 'numeric-limits', expected=oracle)
            lost = ['options levelc', 'do probe=1 to 6', 'call check', 'end', "say 'LOSTDIGITS_OK'", 'exit',
                    'check:', 'numeric digits 3', 'signal on lostdigits name caught', 'select']
            for ordinal, expression in enumerate(('1234**1', '1234%1', '1234//1', '1**1234', '1%1234', '1//1234'), 1):
                lost.append(f'when probe={ordinal} then answer={expression}')
            lost += ['end', 'exit 99', 'caught:', "say condition('C') || '|' || condition('D') || '|' || condition('I')", 'return']
            request('\n'.join(lost) + '\n', 'arithmetic-lostdigits', expected='LOSTDIGITS|1234|SIGNAL\n' * 6 + 'LOSTDIGITS_OK\n')
            lexical = "options levelc\nsay=3\nif=4\nparse=5\nsay say if parse\nsay .5+1e-1\nsay 7d 7D 7dogs 7e2\nsay +7e2\nsay 'a''b'\nsay '['||''x||']'\nsay c2x('F F0'x)\nsay c2x('1 1010'b)\nsay 1 +, /* continuation */\n2\n/* outer /* nested */ comment */\nlabel1: ; label2: ; say 'labels'\nsay 'a'/* comment */'b'\nsay 'a' /* comment */ 'b'\nsay 2 * * 3 * * 2\nsay 9 / / 4\nsay 'a' \\ = = 'b'\nsay 'b' > > = 'a'\nsay 1 & & 0\nsay 'a' | | 'b'\nsay\v1\fsay\t2\n"
            # VT/FF are blanks, not clause terminators.
            lexical = lexical.replace('say\v1\fsay\t2', 'say\v1\f2')
            request(lexical, 'lexical-source', expected="3 4 5\n0.6\n7D 7D 7DOGS 7e2\n700\na'b\n[]\n0FF0\n1A\n3\nlabels\nab\na b\n64\n1\n1\n1\n1\nab\n1 2\n")
            request("options levelc\n@name=1\n#name=2\n$name=3\nsay @name #name $name\nſ=11\nı=12\nɐ=13\nsay s i Ɐ\nsay value('ſ') value('ı') value('ɐ')\nsay symbol('ɐ') datatype('ɐ','S')\nsignal value 'ſTOP'\nexit 99\nstop:\nsay 'width'\ncall ɐlabel\nexit\nⱯLABEL:\nsay 'call'\nreturn\n",
                    'unicode-name-width', expected='1 2 3\n11 12 13\n11 12 13\nVAR 1\nwidth\ncall\n')
            request("options levelc\nsay c2x(encode('€','ASCII','?'))\nsay decode('FF41'x,'UTF-8','?')\nsay graphemesubstr('a',1,3,'🙂')\nsay graphemesubstr('a',1,,'🙂')\nsay isencodingsupported('not-an-encoding')\n",
                    'unicode-presence', expected='3F\n?A\na🙂🙂\na\n0\n')
            allhex = bytes(range(256)).hex().upper()
            request("options levelc\nall=xrange()\nsay length(all)\nsay c2x(all)\ndo n=0 to 255\nif c2d(d2c(n))\\=n then exit 99\nend\nsay c2x(bitand(all,copies('FF'x,256)))==c2x(all)\nsay c2x(bitor(all,copies('00'x,256)))==c2x(all)\nsay c2x(bitxor(all,copies('00'x,256)))==c2x(all)\nsay 'é'=='é'\n",
                    'all-ordinals', expected='256\n' + allhex + '\n1\n1\n1\n0\n')
            casing_source = fixtures / 'classic-casing.crexx'; casing_oracle = fixtures / 'classic-casing.expected'
            inputs.update((casing_source, casing_oracle))
            request(casing_source.read_bytes(), 'classic-casing', expected=casing_oracle.read_text())
            # All eight stream BIFs and ADDRESS/TRACE query use their existing
            # language/configuration owners. File names arrive as argv data.
            stream = "options levelc\nfile=arg(1)\nsay address()\nsay trace()\nsay stream(file,'C','OPEN WRITE REPLACE ENCODING CP1252 CODEPOINTS')\nsay charout(file,'€é')\nsay lineout(file,'Z')\nsay stream(file,'C','CLOSE')\nsay stream(file,'C','OPEN READ ENCODING Windows-1252')\nsay chars(file) lines(file)\nsay charin(file,,1)\nsay linein(file)\nsay chars(file) lines(file)\nsay qualify(file)==file\nsay stream(file,'C','CLOSE')\n"
            request(stream, 'stream-language', expected='CREXX\nN\nREADY:\n0\n0\nREADY:\nREADY:\n4 1\n€\néZ\n0 0\n1\nREADY:\n')
            for index, newline in enumerate((b'\r\n', b'\r', b'\n')):
                payload = newline.join((b'/* header\x00 */', b'options levelc', b"say 'a\x00b'", b'say sourceline()', b"say sourceline(1)"))
                request(payload, 'source-newline-' + str(index), expected='a\x00b\n5\n/* header\x00 */\n')
            mapped = 'options levelc srcmap\n@"original.rexx"@7l"  say \'mapped\'"@3+12{say \'mapped\'@}\nsay sourceline()\n'
            request(mapped, 'mapped-source', expected='mapped\n0\n')
            mapped_error = 'options levelc srcmap\n@"original-error.rexx"@42l"  say .unknown"@3+12{say .unknown@}\n'
            request(mapped_error, 'mapped-error', refusal='50.1', contains=('original-error.rexx', '42'))
            runtime_source = fixtures / 'runtime-c.crexx'; inputs.add(runtime_source)
            request(runtime_source.read_bytes(), 'g3runtime', host_export='g3runtime.run')
            request("options levelc\nsay 'callback'\nreturn 'complete'\n", 'g3inventory', host_export='g3inventory.run')
            static_errors = [("say .unknown", '50.1'), ('say name.()', '51.1'), ("say 'GG'x", '15.3'),
                             ("say '12'b", '15.4'), ("say '12 1'x", '15.1'), ("say '1 1'b", '15.2'),
                             ("say 'unclosed", '6.2'), ('/* unclosed', '6.1'), ('say 1 +', '35.1'),
                             ('nop extra', '21.1'), ('drop 1', '20.1'), ('1=2', '31.1'),
                             ('if 1 say 2', '18.1'), ('select; nop; when 1 then nop; end', '7.2')]
            for index, (body, code) in enumerate(static_errors):
                source = 'options levelc\n' + body + '\n'
                bad_at = {0: '.unknown', 1: 'name.', 2: "'GG'", 3: "'12'", 4: "'12 1'", 5: "'1 1'", 6: "'unclosed", 7: '/*', 8: '+', 11: '1=2'}
                offset = source.index(bad_at[index]) if index in bad_at else None
                request(source, f'static-error-{index}', refusal=code, offset=offset)
                if index in (0, 1, 2, 8, 9, 10, 11):
                    dead = 'options levelc\nexit\n' + body + '\n'
                    offset = dead.index(bad_at[index]) if index in bad_at else None
                    request(dead, f'dead-error-{index}', refusal=code, offset=offset)
            for stem in ('levelc_options_numeric_common', 'levelc_options_comments_slash', 'levelc_options_conflicting_comments'):
                path = native / (stem + '.rexx'); inputs.add(path)
                request(path.read_bytes(), stem, refusal='incompatible source option')
            # Native checked source rows cover the changed fail-first generic
            # diagnostic branches. Multi-error input remains unchanged; only
            # its independently checked first error is required here.
            for stem, code in (
                ('levelc_if_stray_else', '8.2'), ('levelc_select_stray_when', '9.1'),
                ('levelc_select_stray_otherwise', '9.2'), ('levelc_select_missing_when', '7.1'),
                ('levelc_if_then_missing_arm', '14.3'), ('levelc_if_else_missing_arm', '14.4'),
                ('levelc_do_missing_end', '14.1'), ('levelc_select_missing_end', '14.2'),
                ('levelc_do_group_named_end', '10.3'), ('levelc_select_end_symbol', '10.4'),
                ('levelc_do_end_bad_symbol', '10.2'), ('levelc_when_missing_then', '18.2'),
                ('levelc_expression_bad_paren', '36'), ('levelc_expression_bad_rparen', '37.2'),
                ('levelc_expression_bad_comma', '37.1'),
            ):
                path = native / (stem + '.rexx'); inputs.add(path)
                request(path.read_bytes(), stem, refusal=code)
            request(b'options levelc\nsay "\xff"\n', 'invalid-utf8', refusal='22.1', offset=20)
            request('options levelc\nsay "unclosed\n', 'unmatched-double-quote', refusal='6.3', offset=19)
            request(b'options levelc\n\x00\n', 'nul-token', refusal='13.1', offset=15)
            request("options levelc\nsay 'é' .unknown\n", 'unicode-error-column', refusal='50.1', offset=24, contains=('line 2, column 9',))
            for stem, body in (('signed-exponent-label', '7e+2: nop'), ('quoted-label', "'quoted': nop"), ('comment-label-gap', 'NAME/*gap*/: nop')):
                for dead in (False, True):
                    source = 'options levelc\n' + ('exit\n' if dead else '') + body + '\n'
                    request(source, stem + ('-dead' if dead else ''), refusal='13.1', offset=source.index(':'))
            for stem, expression in (('power-comment-gap', '2*/*x*/*3'), ('strict-comment-gap', "'a'=/*x*/='b'"), ('xor-comment-gap', '1&/*x*/&0')):
                request('options levelc\nsay ' + expression + '\n', stem, refusal='35.1')
                request('options levelc\nexit\nsay ' + expression + '\n', stem + '-dead', refusal='35.1')
            for stem, limits in [('work-limit', (1, 512, 67108864, 65536)), ('output-limit', (1000000, 512, 16, 65536)), ('register-limit', (1000000, 512, 67108864, 1))]:
                request('options levelc\nsay 1\n', stem, refusal='LIMIT', limits=limits)
            request('options levelc\nsay 7//3\n', 'recovery-after-failures', expected='1\n')
            # Every name must appear in a successful, observable-value source,
            # separately from the uniform all95 arity transport fixture.
            observed = set()
            for source in positive_sources:
                if 'ERROR_PANEL_OK' not in source:
                    observed.update(name.upper() for name in re.findall(r'\b([A-Za-z][A-Za-z0-9]*)\s*\(', source))
            assert set(BIFS) <= observed, ('missing per-name positive source', sorted(set(BIFS) - observed))
        selected = set()
        if emit_only:
            known = {stem for stem, _ in requests}
            selected = set(args.emit_case) or known
            assert selected <= known, ('unknown emission request', sorted(selected - known))
            requests = [item for item in requests if item[0] in selected]
            negatives = [item for item in negatives if item[0] in selected]
        worker = bcompile(fixtures / 'compile-fixtures.crexx', 'compile-group3', compiler=True)
        worker = link('compile-group3', worker, compiler=True)
        request_manifest = work / 'compile.manifest'; request_manifest.write_text(''.join(row + '\n' for _, row in requests))
        log = work / 'compile-batch.log'
        print('GROUP3_BATCH|' + str(log), flush=True)
        with log.open('wb') as output:
            result = subprocess.run([str(hostvm), '--no-autoload', str(worker), '-a', str(runtime), str(request_manifest), str(tool('rxas')), *(['emit-only'] if emit_only else [])],
                                    cwd=host, env=env, stdout=output, stderr=subprocess.STDOUT, timeout=7200)
        diagnostic = log.read_bytes()
        if emit_only:
            marker = f'GROUP3_EMISSION|requests={len(requests)}|EMISSION'.encode()
            rows = diagnostic.replace(b'\r\n', b'\n').splitlines()
            assert result.returncode == 0 and rows.count(marker) == 1, ('warm emission batch', diagnostic[-10000:])
            emitted = [('EMITTED|' + row.split('|')[2]).encode() for _, row in requests if not row.split('|')[4]]
            assert sorted(row for row in rows if row.startswith(b'EMITTED|')) == sorted(emitted), 'exact emitted requests'
            assert not any(row.startswith((b'COMPILED|', b'GROUP3_FIXTURES|')) for row in rows), 'emission is not assembly qualification'
        else:
            assert result.returncode == 0 and b'|PASS\n' in diagnostic, ('warm compiler batch', diagnostic[-10000:])
        assert not re.search(rb'Warning in |warning:|#WARNING|Sanitizer|runtime error:', diagnostic), diagnostic[-5000:]
        for stem, assembly, code, offset, contains in negatives:
            assert assembly.read_bytes() == b'previous output\n', ('atomic previous output', stem)
            prefix = ('REFUSAL|' + stem + '.rexx|').encode()
            row = next((line for line in diagnostic.splitlines() if line.startswith(prefix)), b'')
            expected_code = ('RXC-LC-' + code + ':' if re.fullmatch(r'\d+(?:\.\d+)?', code) else code).encode()
            assert row and expected_code in row, (stem, diagnostic[-5000:])
            if offset is not None:
                assert row.startswith(prefix + str(offset).encode() + b'|'), (stem, row)
            for part in contains:
                assert part.encode() in row, (stem, row, part)
        if not emit_only or 'simple-constant' in selected:
            source_shapes((app / 'simple-constant.rxas').read_text(), simple=True)
        if not emit_only or 'effectful-preview' in selected:
            source_shapes((app / 'effectful-preview.rxas').read_text())
        if emit_only:
            requests_identity = []
            for stem, row in requests:
                fields = row.split('|'); source, output = Path(fields[0]), Path(fields[1])
                if not fields[4]:
                    assert output.stat().st_size > 0, ('emitted RXAS is nonempty', stem)
                requests_identity.append(dict(stem=stem, row=row, source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
                                              rxas_sha256=hashlib.sha256(output.read_bytes()).hexdigest(), refusal=bool(fields[4])))
            expected_starts = [('START|' + row.split('|')[2]).encode() for _, row in requests]
            assert [row for row in rows if row.startswith(b'START|')] == expected_starts, 'exact ordered request records'
            expected_refusals = [row.split('|')[2].encode() for _, row in requests if row.split('|')[4]]
            assert sorted(row.split(b'|', 2)[1] for row in rows if row.startswith(b'REFUSAL|')) == sorted(expected_refusals), 'exact refusal records'
            identity = dict(kind='EMISSION', compiler_mode='opt', semantic_ir=semantic_ir, producer_vm='rxbvm', requests=requests_identity,
                            worker_sha256=hashlib.sha256(worker.read_bytes()).hexdigest(),
                            vm_sha256=hashlib.sha256(hostvm.read_bytes()).hexdigest(),
                            seed_tools={name: hashlib.sha256(tool(name).read_bytes()).hexdigest() for name in ('rxc', 'rxas', 'rxlink')},
                            imports={path.name: hashlib.sha256(path.read_bytes()).hexdigest() for path in runtime.iterdir() if path.is_file() and path.suffix in ('.rxbin', '.rxplugin')})
            (work / 'emission.json').write_text(json.dumps(identity), encoding='utf-8')
            print('GROUP3_EMISSION|requests=' + str(len(requests_identity)) + '|EMISSION|' + str(work), flush=True)
            return
        for stem, assembly, oracle in pending:
            checks.append(dict(stem=stem, image=str(link(stem, assembly.with_suffix('.rxbin'))), expected=oracle, argv=[]))
            if stem == 'effectful-preview':
                checks[-1]['argv'] = ['0']
            if stem == 'stream-language':
                checks[-1]['argv'] = [str(work / 'language-stream.dat')]
        if args.phase != 'preview':
            host_source = fixtures / 'runtime-host.crexx'; inputs.add(host_source)
            consumer = bcompile(host_source, 'runtime-host')
            checks.append(dict(stem='configured-language-host', image=str(link('runtime-host', consumer, (runtime / 'g3runtime.rxbin', runtime / 'g3inventory.rxbin'))), expected='GROUP3_RUNTIME_OK\n', argv=[]))
            for stem, oracle in (('srcmap-contracts', 'G3_SRCMAP_OK\n'), ('ir-contracts', 'G3_IR_OK\n')):
                contract_source = fixtures / (stem + '.crexx'); inputs.add(contract_source)
                image = bcompile(contract_source, stem, compiler=True)
                image = link(stem, image, compiler=True)
                checks.append(dict(stem=stem, image=str(image), expected=oracle, argv=[str(runtime)] if stem == 'ir-contracts' else [], host=True))
        manifest.write_text(json.dumps(dict(checks=checks, inputs={str(path): hashlib.sha256(path.read_bytes()).hexdigest() for path in inputs})))
        print('GROUP3_PREPARED|' + str(work), flush=True)
    if args.phase != 'prepare':
        prepared = json.loads(manifest.read_text())
        known = {item['stem'] for item in prepared['checks']}
        assert set(args.skip_check) <= known, 'known prepared checks'
        for path, digest in prepared['inputs'].items():
            assert hashlib.sha256(Path(path).read_bytes()).hexdigest() == digest, ('changed prepared source', path)
        for item in prepared['checks']:
            if item['stem'] in args.skip_check:
                continue
            command = [hostvm if item.get('host') else vm, '--no-autoload', item['image']]
            if item['argv']:
                command += ['-a', *item['argv']]
            result = run(item['stem'], command, silent=False)
            assert result.stdout.replace(b'\r\n', b'\n') == item['expected'].encode() and not result.stderr, (item['stem'], result.stdout, result.stderr, item['expected'])
            print('GROUP3_CHECK|' + item['stem'] + '|PASS', flush=True)
        if 'stream-language' in known:
            assert (work / 'language-stream.dat').read_bytes() == b'\x80\xe9Z\n', 'exact external encoding and close'
        print('GROUP3_STATIC|checks=' + str(len(prepared['checks'])) + '|PASS', flush=True)
    if temporary:
        temporary.cleanup()


if __name__ == '__main__':
    main()
