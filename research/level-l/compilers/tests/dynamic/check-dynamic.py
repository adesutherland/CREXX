#!/usr/bin/env python3
"""Independent dynamic C contracts in one prepared C-opt/IR-on/rxbvm lane.

The native fixtures retain their checked expectations; an empty INTERPRET
selects the dynamic body ABI, and one unchanged recursive program runs as a
source child. New dynamic behavior has authored oracles and is never executed
by the native C compiler. Preparation uses one warm compiler
worker. Application runs have source data, providers and bytecode, with no
compiler or assembler on PATH. Low-level resource/ledger/neutral contracts are
registered separately, rather than duplicated in this language driver.
"""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import queue
import re
import runpy
import shutil
import subprocess
import tempfile
import textwrap
import threading


DIAGNOSTIC = re.compile(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING')


def program(body):
    return 'options levelc\n' + textwrap.dedent(body).strip() + '\n'


def checked_native_oracle(root, stem):
    expected = root / 'compiler/tests/rexx_src' / (stem + '.expected')
    if expected.exists():
        return expected.read_text(), expected
    cmake = root / 'compiler/tests/CMakeLists.txt'
    match = re.search(r'add_levelc_runtime_slice_test\(' + re.escape(stem) +
                      r'\s+rexx_src/' + re.escape(stem) + r'\.rexx\s+"((?:[^"\\]|\\.)*)"', cmake.read_text())
    assert match, ('maintained expectation', stem)
    return re.sub(r'\\([nrt"\\])', lambda m: {'n': '\n', 'r': '\r', 't': '\t', '"': '"', '\\': '\\'}[m[1]], match[1]), cmake


def package_shape(control):
    """Check the real generated optional product, including retained roots."""
    inputs = set(); roots = set()
    for row in control.read_text().splitlines():
        command, _, value = row.strip().partition(' ')
        if command == 'INPUT':
            inputs.add(Path(value.strip()).stem.lower())
        elif command == 'INCLUDE':
            roots.add(value.strip().lower())
    assert {'llcgraph', 'llcfrontend', 'rxfnsl-execute'} <= inputs, ('dynamic frontend/executor inputs', inputs)
    assert {'llcgraph', 'llcfrontend', 'llirexecute'} <= roots, ('dynamic frontend/executor roots', roots)
    forbidden = {'llcrxas', 'llccompiler', 'llrxc-c', 'llrxc-c-linked', 'llccli',
                 'rxfnsl-rxas', 'llrxas', 'rxas', 'rxlink', 'rxc', 'rxc-c',
                 'rxcc_parse', 'rxcc', 'rxcp', 'rxclib', 'rxcc_lib'}
    assert not (inputs | roots) & forbidden, ('dynamic product contains selector/compiler/assembler', (inputs | roots) & forbidden)
    assert not any(name.startswith(('llrxc-', 'rxcc_', 'rxcp_')) for name in inputs | roots), 'dynamic product has no CLI or native source-frontend root'


def authored_trace(completed, case, root):
    records = runpy.run_path(str(root / 'research/level-l/compilers/tests/trace/check-trace.py'))['records']
    plain, events = records(completed)
    interpreted = {'x=x+2': 1, 'say x': 2}
    authored = set(case['source'].splitlines()) | set(interpreted)
    seen = set(); current = None; actual_value = False
    for kind, line, value in events:
        if kind == 'source':
            assert value in authored, ('TRACE exposes implementation source', line, value)
            current = value
            if value in interpreted:
                assert line == interpreted[value], ('truthful interpreted line', line, value)
                seen.add(value)
        elif kind.startswith('>'):
            if current in interpreted and value == '42':
                actual_value = True
    assert set(interpreted) <= seen and actual_value, ('authored IR source and computed/read value', events)
    assert not any(name in (completed.stdout + completed.stderr).lower()
                   for name in (b'llcdynamic.', b'llirexecute.', b'rxvminvoke.', b'__rxtrace_handler')), 'TRACE hides coordinator/helper machinery'
    # Practical TRACE can omit literal-only events; no requirement for a `2`
    # literal record is imposed by this actual operation/read control.
    return plain


def optional_windows(text):
    """Fixed B ABI slots remain mapped even when an optional is absent."""
    lines = text.splitlines()
    for index, row in enumerate(lines):
        match = re.fullmatch(r'call [^,]+,llcdynamic\.(scope\.close|resume)\(\),r(\d+)', row.strip())
        if not match:
            continue
        base = int(match[2]); physical = 2 if match[1] == 'scope.close' else 3
        setup = '\n'.join(lines[max(0, index - physical - 3):index])
        assert f'load r{base},{physical}' in setup, ('complete optional window', row, setup)
        assert re.search(rf'\bsettp r{base + physical},(?:512|768)\b', setup), ('optional presence flag', row, setup)


def catalogue_shape(text):
    """Only pure module data is prepared once; invocation ownership stays local."""
    globals_count = int(re.search(r'^\.globals=(\d+)$', text, re.M)[1])
    boot = re.search(r'^llcatalogues\(\) \.locals=\d+\n([\s\S]*?)(?=^\S+\(\) \.expose=|\Z)', text, re.M)
    if globals_count == 0:
        assert boot is None and not re.search(r'^\.meta .*="\.initializer"', text, re.M), 'static output has no catalogue initializer'
        assert not re.search(r'^llcdynamic\.|^call [^,]+,llcdynamic\.', text, re.M), 'static output imports no dynamic runtime or prepared scope'
        return
    assert globals_count >= 2 and boot, 'one declaration owner plus function catalogues'
    assert len(re.findall(r'="\.initializer" "\.void" llcatalogues\(\) ""$', text, re.M)) == 1, 'one private declared module initializer'
    assert re.search(r'^\.meta "([^"\n]+)"="b" "\.void" llcatalogues\(\) ""\n'
                     r'\.meta "\1"="\.initializer" "\.void" llcatalogues\(\) ""$', boot[1], re.M), 'same-symbol local callable metadata accompanies initializer'
    assert not re.search(r'^llcatalogues\(\).*\.expose=', text, re.M), 'initializer remains private'
    before, after = text[:boot.start()], text[boot.end():]
    assert not re.search(r'^\S+\(\) \.locals=', after, re.M), 'initializer follows application and private helper bodies'
    assert not re.search(r'^\s*(?:load|copy|move|null|setattrs|call) g\d+(?:,|$)', before + after, re.M), 'globals are never rewritten after module publication'
    body = boot[1]
    assert not re.search(r'^\.traceevent\b|^call [^,]+,rxinvoke\.', body, re.M), 'pure initializer has no authored observations or live native bindings'
    assert all(re.fullmatch(r'\.srcstep 0 0 6 "" 0 0 0 ""', row.strip())
               for row in body.splitlines() if row.strip().startswith('.srcstep ')), 'initializer source is synthetic'
    catalogues = re.findall(r'^call g(\d+),llcdynamic\.catalogue\.§factory\(\),r\d+$', body, re.M)
    assert sorted(map(int, catalogues)) == list(range(1, globals_count)), 'every function catalogue is constructed exactly once'
    ready = re.findall(r'^call (r\d+),llcdynamic\.(declaration|catalogue)\.ready\(\),r\d+$', body, re.M)
    declarations = len(re.findall(r'^call r\d+,llcdynamic\.declaration\.§factory\(\),r\d+$', body, re.M))
    assert len(ready) == declarations + len(catalogues), 'every immutable constructor has an explicit readiness check'
    for flag, kind in ready:
        assert re.search(r'^brf llcatalogues_invalid,' + flag + r'$', body, re.M), ('readiness failure refuses publication', kind)
    uses = list(re.finditer(r'^call (r\d+),llcdynamic\.scope\.§factory\.prepared\(\),r(\d+)$', before, re.M))
    assert len(uses) == len(catalogues), 'each dynamic function uses its prepared catalogue'
    for use in uses:
        scope, base = use.groups(); base = int(base)
        setup = before[max(0, use.start() - 400):use.start()]
        assert re.search(rf'^mkref r{base + 2},g[1-9]\d*$', setup, re.M), 'scope borrows a stable catalogue global'
        assert f'load r{base},3' in setup and f'settp r{base + 3},512' in setup, 'resource-base optional argument remains absent'
        tail = before[use.end():]
        bound = re.search(r'^call r\d+,llcdynamic\.scope\.bindcatalogue\(\),r\d+$', tail, re.M)
        if declarations:
            assert bound and re.search(r'^link r\d+,' + scope + r'$', tail[:bound.start()], re.M), 'native binding starts only after scope publication'
        else:
            assert not bound, 'an empty declaration catalogue introduces no binding frame'
    assert not re.search(r'^call [^,]+,llcdynamic\.scope\.§factory\(\)', before, re.M), 'compiled entry has no repeated legacy catalogue construction'


def dynamic_shape(text):
    """Check actual idle paths and finite routing, without executing applications."""
    catalogue_shape(text)
    rows = [row.strip() for row in text.splitlines()]
    labels = {row[:-1]: index for index, row in enumerate(rows) if re.fullmatch(r'\w+:', row)}
    sites = {}
    for match in re.finditer(r'^load (r\d+),(\d+)\nbr lldispatch(\d+)_dispatch\n(\w+)_linkchild:$', text, re.M):
        register, site, function, prefix = match.groups()
        key = (function, int(site))
        assert key not in sites, ('duplicate continuation', key)
        sites[key] = (register, prefix)
    predicates = [index for index, row in enumerate(rows) if re.fullmatch(r'call r\d+,llctarget\.clausepending\(\),r\d+', row)]
    checkpoints = [index for index, row in enumerate(rows) if re.fullmatch(r'call r\d+,llcdynamic\.checkpoint\(\),r\d+', row)]
    assert len(predicates) == len(checkpoints), 'every dynamic checkpoint has an idle predicate'
    for index in predicates:
        flag, base = re.fullmatch(r'call (r\d+),llctarget\.clausepending\(\),(r\d+)', rows[index]).groups()
        argument = 'r' + str(int(base[1:]) + 1)
        assert re.fullmatch(r'copy ' + argument + r',r\d+', rows[index - 3]), 'one existing context-reference copy'
        assert rows[index - 2:index] == [f'load {base},1', f'settp {argument},768'], 'one required predicate argument'
        assert rows[index + 1] == f'null {argument}', 'predicate argument cleanup'
        idle = re.fullmatch(r'brf (llcomplete\d+_\d+_idle),' + flag, rows[index + 2])
        assert idle and rows[index + 3] == 'bpoff', 'idle branches before TRACE suspension'
        prefix = idle[1][:-5]
        end = labels[prefix + '_finished']
        point = labels[idle[1]]
        assert end == point + 3, 'idle has only boundary and carrier clears'
        assert all(re.fullmatch(r'null r\d+', row) for row in rows[point + 1:end]), 'idle constructs no packet or boundary'
        checkpoint = next((rows[p] for p in checkpoints if index < p < point), '')
        boundary = re.fullmatch(r'call (r\d+),llcdynamic\.checkpoint\(\),r\d+', checkpoint)
        assert boundary and rows[point + 1] == f'null {boundary[1]}', 'idle releases a preceding static boundary'
    families = {}
    for start, row in enumerate(rows):
        root = re.fullmatch(r'(llroute(\d+)_\d+):', row)
        if not root:
            continue
        prefix, function = root.groups()
        selected = {site: value for (fn, site), value in sites.items() if fn == function}
        count = len(selected)
        assert count and set(selected) == set(range(1, count + 1)), 'dense checked continuation IDs'
        registers = {value[0] for value in selected.values()}
        assert len(registers) == 1, 'one frame-local continuation register'
        register = registers.pop()
        invalid = f'lldispatch{function}_invalid'
        code = {}
        equalities = {}
        suffixes = set()
        point = start + 1
        while point < len(rows):
            current = rows[point]
            if re.fullmatch(re.escape(prefix) + r'_left_\d+_\d+:', current):
                point += 1
                continue
            branch = re.fullmatch(r'(beq|blt) (\w+),' + register + r',(-?\d+)', current)
            if branch:
                opcode, target, number = branch.groups(); number = int(number)
                code[point] = (opcode, target, number)
                if opcode == 'beq':
                    assert number in selected and number not in equalities, 'one equality per admitted site'
                    owner = selected[number][1] + '_'
                    assert target.startswith(owner), 'route cannot select a foreign site'
                    suffixes.add(target[len(owner):]); equalities[number] = target
                else:
                    assert target.startswith(prefix + '_left_') and target in labels, 'split remains within its route'
            elif current == f'br {invalid}':
                code[point] = ('br', invalid, 0)
            else:
                break
            point += 1
        assert set(equalities) == set(selected) and len(suffixes) == 1, 'complete single-suffix map'
        suffix = suffixes.pop()
        assert suffix not in families.setdefault(function, set()), 'one shared map per route family'
        families[function].add(suffix)
        groups = (count + 3) // 4
        assert len(code) == count + 2 * groups - 1, 'bounded flat-leaf route size'
        limit = math.ceil(math.log2(groups)) + 4
        for value in [-1, 0, *range(1, count + 1), count + 1]:
            pc = start + 1; seen = set(); comparisons = 0
            while True:
                while pc not in code and pc < point and rows[pc].endswith(':'):
                    pc += 1
                assert pc in code and pc not in seen, 'finite owned route'
                seen.add(pc); opcode, target, number = code[pc]
                if opcode == 'br':
                    result = target; break
                comparisons += 1
                if opcode == 'beq' and value == number:
                    result = target; break
                pc = labels[target] if opcode == 'blt' and value < number else pc + 1
            expected = equalities[value] if 1 <= value <= count else invalid
            assert result == expected and comparisons <= limit, ('checked bounded route', function, suffix, value)
    for function in {fn for fn, site in sites}:
        assert {'linkchild', 'local', 'resume', 'failure', 'done'} <= families.get(function, set()), 'all cold route families checked'
        assert rows.count(f'lldispatch{function}_dispatch:') == 1, 'one shared same-frame dispatcher'
        assert rows.count(f'llterminal{function}:') == 1, 'one equivalent normal terminal'


def inventory(root):
    cases = []
    inputs = {Path(__file__)}

    def add(stem, body, expected, *, files=None, status=0, exchange=None, static=False, trace=False):
        cases.append(dict(stem=stem, source=program(body), expected=expected,
                          files=files or {}, status=status, exchange=exchange, static=static, trace=trace))

    add('static-optionality', 'say 1', '1\n', static=True)

    # Reuse the maintained sources as complete programs, preserving legal labels
    # and every original assertion. Do not wrap labelled programs in a group.
    for stem in ('levelc_call_resolution_result', 'levelc_procedure_whole',
                 'levelc_procedure_lifecycle', 'levelc_frame_recursive_call'):
        source = root / 'compiler/tests/rexx_src' / (stem + '.rexx')
        oracle, owner = checked_native_oracle(root, stem)
        inputs.update((source, owner))
        text = source.read_text()
        if text.startswith('options levelc\n'):
            text = text[len('options levelc\n'):]
        add('existing-' + stem, "interpret ''\n" + text, oracle)

    recursive = root / 'compiler/tests/rexx_src/levelc_frame_recursive_call.rexx'
    oracle, owner = checked_native_oracle(root, 'levelc_frame_recursive_call')
    inputs.update((recursive, owner))
    add('source-program-native-recursion', "call 'maintained-recursive.rexx'", oracle,
        files={'maintained-recursive.rexx': recursive.read_text()})

    add('live-once', '''
        count=0; x=40
        interpret source()
        say x || '|' || count
        exit
        source:
          count=count+1
          return 'x=x+2'
    ''', '42|1\n')
    add('empty-nested-exact-source', '''
        x=0
        interpret ''
        interpret '/* empty comment */'
        interpret "interpret 'x=x+1'; x=x+1"
        interpret "say c2x('A" || '00'x || "é')"
        say x
    ''', '4100E9\n2\n')
    add('compiled-call-and-expression', '''
        count=0
        interpret "call compiled actual(),,''; say result; say compiled(actual(),,'')"
        say count
        exit
        actual:
          count=count+1
          return 'A'
        compiled:
          return arg() || '|' || arg(1,'E') || '|' || arg(2,'E') || '|' || arg(3,'E') || '|' || arg(1) || '|' || arg(3)
    ''', '3|1|0|1|A|\n3|1|0|1|A|\n2\n')
    add('lexical-and-quoted-bif', '''
        interpret "call length 'abc'; say result; say length('abc'); call 'LENGTH' 'abc'; say result; say 'LENGTH'('abc')"
        exit
        length:
          return 'local'
    ''', 'local\nlocal\n3\n3\n')
    add('return-owner', '''
        call child
        say result
        say 'parent'
        exit
        child:
          interpret "interpret 'return 42'; say 'unreachable'"
          say 'unreachable'
          return 99
    ''', '42\nparent\n')
    add('signal-owner-and-loop-cleanup', '''
        do i=1 to 3
          interpret 'signal finished'
          say 'unreachable'
        end
        exit 99
        finished:
          say 'landed'
          do j=1 to 2; say j; end
    ''', 'landed\n1\n2\n')
    add('child-signal-resumes-group', '''
        interpret "call child; say result; say 'resumed'"
        exit
        child:
          signal landed
          return 'unreachable'
        landed:
          return 'child'
    ''', 'child\nresumed\n')
    add('recursive-groups', '''
        count=0
        call child 3
        say count
        exit
        child: procedure expose count
          parse arg depth
          interpret 'count=count+1; if depth>0 then call child depth-1; count=count+1'
          return
    ''', '8\n')
    add('generated-procedure-lifetime', '''
        local='parent'; visible='before'
        call child
        say local || '|' || visible
        exit
        child:
          interpret "interpret ''; procedure expose visible; local='owned'; visible='shared'"
          say local
          interpret "local=local||'!'; signal landed"
          return 99
        landed:
          say local
          return
    ''', 'owned\nowned!\nparent|shared\n')
    add('generated-procedure-repeat-error', '''
        signal on syntax name caught
        call child
        exit 99
        child:
          interpret 'procedure'
          interpret 'procedure'
          return 99
        caught:
          say condition('C')
          return
    ''', 'SYNTAX\n', status=99)
    add('generated-procedure-placement-error', '''
        signal on syntax name caught
        call child
        exit 99
        child:
          x=1
          interpret 'procedure'
          return 99
        caught:
          say condition('C')
          return
    ''', 'SYNTAX\n', status=99)
    add('signal-policy-survives-group', '''
        interpret 'signal on novalue name caught'
        say missing
        exit 99
        caught:
          say condition('C') || '|' || condition('I')
    ''', 'NOVALUE|SIGNAL\n')
    add('call-policy-survives-group-and-off', '''
        handled=0; result='prior'
        interpret 'call on notready name handler'
        x=missing; ignored=linein('dynamic-missing-input-a')
        say handled || '|' || x || '|' || result
        interpret 'call off notready'
        y=other; ignored=linein('dynamic-missing-input-b')
        say handled || '|' || y
        exit
        handler: procedure expose handled
          handled=handled+1
          return
    ''', '1|MISSING|prior\n1|OTHER\n')
    add('dynamic-policy-internal-inheritance', '''
        interpret 'signal on novalue name caught'
        call child
        exit
        child:
          say missing
          return 99
        caught:
          say condition('C')
          return
    ''', 'NOVALUE\n')
    add('numeric-operation-and-restoration', '''
        numeric digits 6
        interpret 'numeric digits 4; say 12345+0; call child; say digits()'
        say digits()
        exit
        child: procedure
          say digits()
          numeric digits 3
          interpret 'say 12345+0'
          return
    ''', '1.235E+4\n4\n1.23E+4\n4\n4\n')
    add('authored-trace', '''
        group="x=x+2" || '0A'x || "say x"
        x=40
        trace i
        interpret group
        trace off
        say 'done'
    ''', '42\ndone\n', trace=True)
    add('group-failure-atomic', '''
        x=0; effects=0
        signal on syntax name caught
        interpret source()
        exit 99
        source:
          effects=effects+1
          return 'x=9; if 0 then say ('
        caught:
          say condition('C') || '|' || x || '|' || effects
    ''', 'SYNTAX|0|1\n')
    for stem, group in (('group-label-refused', 'label: x=9'),
                        ('outer-leave-refused', 'leave'),
                        ('outer-iterate-refused', 'iterate'),
                        ('nul-token-refused', "'00'x")):
        operand = group if stem == 'nul-token-refused' else "'" + group + "'"
        add(stem, f"signal on syntax name caught\ndo 2\ninterpret {operand}\nend\nexit 99\ncaught:\nsay condition('C')", 'SYNTAX\n')
    add('interpreted-exit-root', "interpret 'exit 7'\nsay 'unreachable'", '', status=7)

    add('source-call-expression-arguments', '''
        count=0
        call sourcechild actual('A'),,''
        say result
        say sourcechild(actual('B'),,'')
        say count
        exit
        actual:
          count=count+1
          return arg(1)
    ''', '3|1|0|1|A|\n3|1|0|1|B|\n2\n', files={'sourcechild.rexx': program("return arg() || '|' || arg(1,'E') || '|' || arg(2,'E') || '|' || arg(3,'E') || '|' || arg(1) || '|' || arg(3)")})
    add('source-exact-nul-and-empty', '''
        say c2x(sourcechild('A' || '00'x || 'é'))
        say '[' || sourcechild('') || ']'
    ''', '4100E9\n[]\n', files={'sourcechild.rexx': program('return arg(1)')})
    add('source-root-isolation-and-exit', '''
        x='parent'
        numeric digits 5
        say sourcechild()
        say x || '|' || digits()
    ''', 'LIT|5\n42\nparent|5\n', files={'sourcechild.rexx': program("say symbol('X') || '|' || digits()\nx='child'\nnumeric digits 3\nexit 42\nsay 'unreachable'")})
    child_fault = program('''
        parse arg numerator, denominator
        numerator=numerator+0
        denominator=denominator+0
        answer=numerator/denominator
        return answer
    ''')
    assert child_fault.splitlines()[4] == 'answer=numerator/denominator'
    add('source-child-error-sigl', '''
        interpret 'signal on syntax name caught'
        call sourcechild 1,0
        say 'unreachable'
        exit 99
        caught:
          say condition('C') || '|' || condition('E') || '|' || condition('I') || '|' || sigl
          remembered=condition('E')
          say .MN || '|' || .SIGL || '|' || remembered
    ''', 'SYNTAX|42.3|SIGNAL|5\n42.3|5|42.3\n', files={'sourcechild.rexx': child_fault})
    add('source-empty-and-absent-call', '''
        result='old'
        call emptyfile
        say symbol('RESULT') || '|' || symbol('.RESULT')
        call emptyresult
        say '[' || result || ']|' || symbol('RESULT')
    ''', 'LIT|LIT\n[]|VAR\n', files={'emptyfile.rexx': '', 'emptyresult.rexx': program("return ''")})
    for stem, child, code in (('source-function-eof', '', '44.1'),
                              ('source-function-bare-exit', program('exit'), '44.1'),
                              ('source-function-bare-return', program('return'), '45.1')):
        add(stem, "signal on syntax name caught\nsay sourcechild()\nexit 99\ncaught:\nsay condition('E')", code + '\n', files={'sourcechild.rexx': child})
    add('source-suffix-and-exact-path', '''
        say choose()
        say 'choose.crexx'()
        say './choose'()
        say 'MiXeD.rexx'()
    ''', 'suffix\nexplicit\nbare\ncase\n', files={
        'choose.rexx': program("return 'suffix'"), 'choose': program("return 'bare'"),
        'choose.crexx': program("return 'explicit'"), 'MiXeD.rexx': program("return 'case'")})
    add('source-child-resource-base', "say 'nested/child.rexx'()", 'nested\n', files={
        'nested/child.rexx': program('return sibling()'),
        'nested/sibling.rexx': program("return 'nested'"),
        'sibling.rexx': program("return 'cwd'")})
    for stem, invalid in (('source-found-invalid-stops', program('if then')),
                          ('source-nonc-header-stops', 'options levelb\nsay 99\n')):
        add(stem, "signal on syntax name caught\nsay choose()\nexit 99\ncaught:\nsay condition('C')", 'SYNTAX\n',
            files={'choose.rexx': invalid, 'choose': program("return 'wrong fallback'")})
    add('source-no-implicit-path', "signal on syntax name caught\ncall pathonly\nexit 99\ncaught:\nsay condition('E')", '43.1\n')
    add('source-created-after-miss', '''
        signal on syntax name missing
        call late
        exit 99
        missing:
          if condition('E') \\== '43.1' then exit 98
          say 'READY'
          pull gate
          signal off syntax
          say late()
    ''', 'READY\ncreated\n', exchange={'path': 'late.rexx', 'bytes': program("return 'created'"), 'same_time': False})
    add('source-same-path-same-time-change', '''
        say changing()
        say 'READY'
        pull gate
        say changing()
    ''', 'first\nREADY\nlater\n', files={'changing.rexx': program("return 'first'")},
        exchange={'path': 'changing.rexx', 'bytes': program("return 'later'"), 'same_time': True})
    return cases, inputs


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--phase', choices=('all', 'preview', 'prepare', 'run'), default='all')
    parser.add_argument('--only', action='append', default=[], help='select named checks, including during preparation')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[5]
    fixtures = Path(__file__).resolve().parent
    worker_source = fixtures.parent / 'group3/compile-fixtures.crexx'
    cases, inputs = inventory(root); inputs.add(worker_source)
    if args.only and args.phase != 'run':
        known = {case['stem'] for case in cases}
        if not set(args.only) <= known:
            parser.error('unknown selected check: ' + ', '.join(sorted(set(args.only) - known)))
        cases = [case for case in cases if case['stem'] in args.only]
    inputs.update((fixtures.parent / 'trace/check-trace.py', fixtures.parent / 'address/check-address.py'))
    for owner in ('interpreter/rxvminvoke.c', 'interpreter/rxvminvoke.h',
                  'interpreter/rxvmintp.c', 'interpreter/rxvmintp.h',
                  'interpreter/rxvmhandlers_core.inc', 'interpreter/rxvmhandlers_control.inc',
                  'interpreter/rxvmload.c', 'interpreter/rxvml/rxvm_run.c',
                  'interpreter/rxcrexxcmd.c', 'interpreter/rxcrexxcmd.h',
                  'lib/plugins/fs/rxfs.c', 'lib/plugins/fs/rxfs_source.h',
                  'lib/plugins/fs/rxfs_ops.h', 'rxpa/crexxpa.h'):
        inputs.add(root / owner)
    build = args.build.resolve(); work = args.work.resolve(); work.mkdir(parents=True, exist_ok=True)
    temporary = None
    if args.phase == 'all':
        temporary = tempfile.TemporaryDirectory(prefix='dynamic-', dir=work)
        work = Path(temporary.name)
    backend = build / 'research/level-l/compilers/backend/opt'
    suffix = '.exe' if os.name == 'nt' else ''
    app = work / 'application'; host = work / 'host'; runtime = work / 'runtime'; empty = work / 'tool-free PATH'
    for directory in (app, host, runtime, empty):
        directory.mkdir(exist_ok=True)
    env = dict(os.environ, PATH=str(empty), CREXX_TRACE_TO='stdout')
    env.pop('CREXX_PROVIDER_PATH', None); env.pop('CREXX_TRACE', None)
    vm = app / ('rxbvm' + suffix); hostvm = host / vm.name
    manifest = work / 'prepared.json'

    def tool(name):
        return build / 'bin' / (name + suffix)

    def run(label, command, *, cwd=app, silent=True, timeout=1200):
        result = subprocess.run([str(item) for item in command], cwd=cwd, env=env,
                                capture_output=True, timeout=timeout)
        diagnostic = result.stdout + result.stderr
        assert result.returncode == 0 and not DIAGNOSTIC.search(diagnostic), (label, result.returncode, diagnostic[-8000:])
        if silent:
            assert not diagnostic, (label, diagnostic[-5000:])
        return result

    def link(stem, image, *, compiler=False, static=False):
        output = (host if compiler else app) / (stem + '-linked')
        members = sorted((backend / 'imports-llrxc-c').glob('*.rxbin')) if compiler else sorted(runtime.glob('*.rxbin'))
        if static:
            members = [path for path in members if path.stem != 'llcdynamic']
        control = work / (stem + '.ctl')
        control.write_text(''.join(f'INPUT {item}\n' for item in [image, *members]) + f'ROOT {image.stem}\nOUTPUT {output}\n')
        run('link ' + stem, [tool('rxlink'), '-s', '-c', control])
        return output.with_suffix('.rxbin')

    if args.phase != 'run':
        control = build / 'research/level-l/compilers/dynamic/opt/dynamic.ctl'
        package_shape(control); inputs.add(control)
        for name in ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core', 'llcdynamic'):
            origin = backend if name == 'llctarget' else build / 'bin'
            if name == 'llcdynamic':
                origin = build / 'research/level-l/compilers/products'
            source = origin / (name + '.rxbin')
            shutil.copy2(source, runtime / source.name)
        for directory in (app, host):
            shutil.copy2(tool('rxbvm'), directory / vm.name)
            providers = directory / 'providers'; providers.mkdir(exist_ok=True)
            for alias in ('rxfs', 'rxcstream', 'rxcontract', 'rx_fs', 'rx_cstream', 'rx_contract'):
                for source in (build / 'bin' / (alias + '.rxplugin'), build / 'bin/providers' / (alias + '.rxplugin')):
                    if source.exists():
                        shutil.copy2(source, providers / source.name)
            if os.name == 'nt':
                for source in (build / 'bin').glob('*.dll'):
                    shutil.copy2(source, directory / source.name)
        isolated = work / 'typed-sources'; isolated.mkdir(exist_ok=True)
        shutil.copy2(worker_source, isolated / worker_source.name)
        assembly = host / 'compile-dynamic.rxas'
        run('prepare warm compiler', [tool('rxc'), '-x', '--no-exe-import', '-i', backend / 'imports-llrxc-c', '-o', assembly, isolated / worker_source.name])
        run('assemble warm compiler', [tool('rxas'), '-o', assembly.with_suffix('.rxbin'), assembly])
        worker = link('compile-dynamic', assembly.with_suffix('.rxbin'), compiler=True)
        if args.phase == 'preview':
            cases = [case for case in cases if case['stem'] in ('static-optionality', 'live-once', 'compiled-call-and-expression', 'source-call-expression-arguments')]
        requests = []
        for case in cases:
            stem = case['stem']; source = work / (stem + '.rexx'); source.write_text(case['source'], encoding='utf-8')
            assembly = app / (stem + '.rxas')
            fields = [str(source), str(assembly), stem + '.rexx', '', '', '1000000', '512', '67108864', '65536', '']
            assert not any('|' in field or '\n' in field for field in fields)
            requests.append('|'.join(fields))
            inputs.add(source)
        request_manifest = work / 'compile.manifest'; request_manifest.write_text('\n'.join(requests) + '\n')
        log = work / 'compile-batch.log'
        print('DYNAMIC_BATCH|' + str(log), flush=True)
        with log.open('wb') as output:
            compiled = subprocess.run([str(hostvm), '--no-autoload', str(worker), '-a', str(runtime), str(request_manifest), str(tool('rxas'))],
                                      cwd=host, env=env, stdout=output, stderr=subprocess.STDOUT, timeout=14400)
        diagnostic = log.read_bytes()
        assert compiled.returncode == 0 and b'|PASS\n' in diagnostic and not DIAGNOSTIC.search(diagnostic), ('warm dynamic compiler', diagnostic[-10000:])
        for case in cases:
            stem = case['stem']; assembly = app / (stem + '.rxas')
            text = assembly.read_text(encoding='utf-8')
            optional_windows(text)
            dynamic_shape(text)
            if case['static']:
                assert not re.search(r'llcdynamic|llirexecute|llcgraph|llcfrontend|llccompiler', text, re.I), 'trivial static application has no dynamic/compiler/executor dependency'
            if stem == 'live-once':
                assert 'llcdynamic.interpret' in text and 'lldynamic' in text, 'inline owner dispatcher and shared dynamic service'
                assert not re.search(r'\bcall\s+[^,]+,(?:llccompiler|rxas|rxcc_parse)\.', text), 'application has no source compiler or assembler call'
            case['image'] = str(link(stem, assembly.with_suffix('.rxbin'), static=case['static']))
            inputs.update((assembly, Path(case['image'])))
        # Hash the actual isolated execution snapshots. A later build elsewhere
        # neither replaces them nor changes which VM/provider qualified a case.
        inputs.add(worker)
        for directory in (runtime, app / 'providers', host / 'providers'):
            inputs.update(path for path in directory.rglob('*') if path.is_file())
        inputs.update((vm, hostvm))
        if os.name == 'nt':
            inputs.update(app.glob('*.dll')); inputs.update(host.glob('*.dll'))
        manifest.write_text(json.dumps(dict(cases=cases, inputs={str(path): hashlib.sha256(path.read_bytes()).hexdigest() for path in inputs})))
        print('DYNAMIC_PREPARED|' + str(work), flush=True)
    if args.phase != 'prepare':
        prepared = json.loads(manifest.read_text())
        known = {case['stem'] for case in prepared['cases']}
        assert set(args.only) <= known, ('known prepared cases', args.only)
        for path, digest in prepared['inputs'].items():
            assert hashlib.sha256(Path(path).read_bytes()).hexdigest() == digest, ('changed prepared input', path)
        # A source file on PATH proves that source search does not consult PATH;
        # this directory contains no executable tools.
        (empty / 'pathonly.rexx').write_text(program("return 'wrong PATH'"))
        count = 0
        for case in prepared['cases']:
            if args.only and case['stem'] not in args.only:
                continue
            cwd = work / 'runs' / case['stem']
            if cwd.exists():
                shutil.rmtree(cwd)
            cwd.mkdir(parents=True)
            for name, payload in case['files'].items():
                destination = cwd / name; destination.parent.mkdir(parents=True, exist_ok=True); destination.write_text(payload, encoding='utf-8')
            command = [str(vm), '--no-autoload', case['image']]
            if case['exchange']:
                result = exchange(command, cwd, env, case['exchange'])
            else:
                result = subprocess.run(command, cwd=cwd, env=env, capture_output=True, timeout=300)
            diagnostic = result.stdout + result.stderr
            assert not DIAGNOSTIC.search(diagnostic), (case['stem'], diagnostic[-8000:])
            plain = authored_trace(result, case, root) if case.get('trace') else result.stdout
            assert result.returncode == case['status'] and plain.replace(b'\r\n', b'\n') == case['expected'].encode() and not result.stderr, (case['stem'], result.returncode, result.stdout, result.stderr, case['expected'])
            print('DYNAMIC_CHECK|' + case['stem'] + '|PASS', flush=True); count += 1
        print('DYNAMIC_C|checks=' + str(count) + '|PASS', flush=True)
    if temporary:
        temporary.cleanup()


def exchange(command, cwd, env, action):
    """Change a source in a suspended live VM, without an unbounded pipe read."""
    process = subprocess.Popen(command, cwd=cwd, env=env, stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    lines = queue.Queue(); captured = []

    def consume():
        for line in process.stdout:
            lines.put(line)
        lines.put(None)

    reader = threading.Thread(target=consume, daemon=True); reader.start()
    try:
        while True:
            line = lines.get(timeout=300)
            assert line is not None, ('source exchange ended before READY', process.poll(), b''.join(captured))
            captured.append(line)
            if line.rstrip(b'\r\n') == b'READY':
                break
        path = cwd / action['path']; original = path.stat() if action['same_time'] else None
        payload = action['bytes'].encode()
        if original:
            assert len(payload) == original.st_size, 'same-size cache invalidation control'
        path.write_bytes(payload)
        if original:
            os.utime(path, ns=(original.st_atime_ns, original.st_mtime_ns))
            assert path.stat().st_mtime_ns == original.st_mtime_ns, 'unchanged resource timestamp'
        process.stdin.write(b'continue\n'); process.stdin.flush(); process.stdin.close()
        process.wait(timeout=300); reader.join(timeout=5)
        assert not reader.is_alive(), 'completed source exchange output'
        while True:
            line = lines.get_nowait()
            if line is None:
                break
            captured.append(line)
        return subprocess.CompletedProcess(command, process.returncode, b''.join(captured), process.stderr.read())
    finally:
        if process.poll() is None:
            process.kill(); process.wait()
        process.stdout.close(); process.stderr.close()
        if not process.stdin.closed:
            process.stdin.close()


if __name__ == '__main__':
    main()
