#!/usr/bin/env python3
"""C22 component checks, starting from unchanged native C fixtures/oracles.

The compiler service processes an ordered manifest in one warm B/L worker.
Existing native C sources are never rewritten. Only their compiler launch,
controlled queue injection and checked TRACE presentation boundary are adapted.
Explicit --prepared storage is active scratch for early review/focused reruns;
its exact relevant inputs must match and it must be removed after qualification.
"""
import argparse
import ast
from contextlib import nullcontext
import hashlib
import inspect
import json
import os
from pathlib import Path
import re
import runpy
import shlex
import shutil
import subprocess
import tempfile


FIXTURES = Path(__file__).resolve().parent
REPOSITORY = FIXTURES.parents[4]
NATIVE = REPOSITORY / 'compiler/tests'
SOURCES = NATIVE / 'rexx_src'
LIBRARIES = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
DELAYED = {
    'policy': 'levelc_call_delayed_handler',
    'conditions': 'levelc_call_condition_matrix',
    'boundaries': 'levelc_call_clause_boundaries',
    'transfers': 'levelc_call_transfer_boundaries',
    'lifecycle': 'levelc_call_policy_lifecycle',
    'buffered_halt': 'levelc_call_buffered_halt',
    'missing': 'levelc_call_missing_delayed_handler',
}
ORDINARY = (
    'levelc_condition_state', 'levelc_condition_nested_restore',
    'levelc_numeric_lostdigits', 'levelc_parse_whole',
    'levelc_arg_static_templates', 'levelc_pull_instruction',
)
EXTERNAL = ('levelc_call_external_trap', 'levelc_call_signal_override')
HOSTED = ('levelc_address_host_callback', 'levelc_address_notready')
FATAL = {'levelc_condition_bad_option': '40.28',
         'levelc_condition_bad_empty': '40.21',
         'levelc_condition_bad_arity': '40.4'}


def native_assignment(path, name):
    """Read one maintained literal oracle without executing a native driver."""
    tree = ast.parse(path.read_text(encoding='utf-8'), filename=str(path))
    values = [node.value for node in ast.walk(tree)
              if isinstance(node, ast.Assign)
              and any(isinstance(target, ast.Name) and target.id == name
                      for target in node.targets)]
    assert len(values) == 1, ('unique maintained oracle', path, name)
    return ast.literal_eval(values[0])


def cmake_output(stem):
    """Native runtime registration is the owner where no .expected exists."""
    text = (NATIVE / 'CMakeLists.txt').read_text(encoding='utf-8')
    pattern = (r'add_levelc_runtime_slice_test\(\s*' + re.escape(stem)
               + r'\s+rexx_src/' + re.escape(stem) + r'\.rexx\s+"((?:\\.|[^"\\])*)"\s*\)')
    found = re.findall(pattern, text)
    assert len(found) == 1, ('unique native runtime output', stem, found)
    # CMake registrations use these intentional escapes, never Python's broad
    # unicode_escape decoder (which would corrupt literal Unicode source).
    value = re.sub(r'\\([nrt"\\])',
                   lambda match: {'n': '\n', 'r': '\r', 't': '\t',
                                  '"': '"', '\\': '\\'}[match[1]], found[0])
    return value.encode('utf-8')


def ordinary_output(stem):
    expected = SOURCES / (stem + '.expected')
    return expected.read_bytes().replace(b'\r\n', b'\n') if expected.exists() else cmake_output(stem)


def native_comparison(path, variable):
    tree = ast.parse(path.read_text(encoding='utf-8'), filename=str(path))
    found = [ast.literal_eval(node.comparators[0]) for node in ast.walk(tree)
             if isinstance(node, ast.Compare) and len(node.comparators) == 1
             and isinstance(node.left, ast.Name) and node.left.id == variable
             and isinstance(node.comparators[0], ast.Constant)]
    assert len(found) == 1, ('unique checked native comparison', path, variable)
    return found[0]


def native_negative_cases():
    """Isolate existing native diagnostic rows for a fail-first C parser.

    Earlier invalid rows become blank lines; the selected authored clause,
    original row number and maintained code/column oracle remain unchanged.
    One dead variant supplies only the still-missing unreachable-form check.
    """
    cmake = (NATIVE / 'CMakeLists.txt').read_text().replace('\\', '')
    cases = []
    for stem in ('levelc_call_diagnostic_matrix', 'levelc_call_surplus_policy_tails'):
        source = SOURCES / (stem + '.rexx'); rows = source.read_text().splitlines()
        pattern = re.escape(stem) + r'\.rexx @ (\d+):(\d+) - #RXC-LC-(\d+\.\d+)'
        expected = re.findall(pattern, cmake)
        assert len(expected) == (10 if stem.endswith('matrix') else 4), ('complete maintained diagnostic row crosswalk', stem, expected)
        for number, column, code in expected:
            number = int(number); column = int(column)
            text = rows[0] + '\n' + '\n' * (number - 2) + rows[number - 1] + '\n'
            offset = len(text.splitlines(keepends=True)[0].encode()) + number - 2
            offset += len(rows[number - 1][:column - 1].encode())
            cases.append((f'{stem}-{number}', source.name, text.encode(), code, offset, number, column))
    stem = 'levelc_call_diagnostic_matrix'; rows = (SOURCES / (stem + '.rexx')).read_text().splitlines()
    original = [case for case in cases if case[0] == stem + '-5']
    assert len(original) == 1 and rows[4] == 'call on syntax'
    prefix = 'if 0 then '
    text = rows[0] + '\n' + prefix + rows[4] + '\n'
    column = original[0][6] + len(prefix)
    offset = len((rows[0] + '\n').encode()) + column - 1
    cases.append(('dead-native-call-on-syntax', stem + '.rexx', text.encode(), original[0][3], offset, 2, column))
    return cases


def negative_excerpt(source, line, column):
    """Exact two-row source/caret contract for these maintained ASCII clauses."""
    text = source.decode('utf-8').splitlines()[line - 1]
    assert all(32 <= ord(character) < 127 for character in text), 'selected diagnostic clauses contain printable ASCII only'
    start = max(0, column - 21)
    prefix = '...' if start else ''
    excerpt = prefix + text[start:start + 160]
    if len(text) > start + 160:
        excerpt += '...'
    marker = len(prefix) + min(column - 1, len(text)) - start
    return [('  ' + excerpt).encode(), ('  ' + ' ' * marker + '^').encode()]


def function_extent(text):
    header = re.search(r'^llf1\(\) \.locals=(\d+)[^\n]*$', text, re.M)
    assert header, 'one canonical shared C source function'
    following = re.search(r'^\S+\([^\n]*\) \.locals=\d+', text[header.end():], re.M)
    end = header.end() + following.start() if following else len(text)
    assert 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int' in text[header.start():end]
    return header, end


def called(text, name):
    return list(re.finditer(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s+[^,\n]+,§?'
                           + re.escape(name) + r'\(\)(?:,[^\n]*)?$', text, re.M))


def rxas_string(value):
    return '"' + value.replace('\\', '\\\\').replace('"', '\\"').replace('\0', '\\x00').replace('\n', '\\n').replace('\r', '\\r') + '"'


def queue_snippet(base, line, description, condition=2):
    """Existing queue API, borrowing the canonical context a1; no VM event API."""
    return (f'   linkref r{base + 1},a1\n'
            f'   load r{base},1\n'
            f'   settp r{base + 1},256\n'
            f'   call r{base + 6},rexxclassiccontext.rexxclassiccontext.activation(),r{base}\n'
            f'   unlink r{base + 1}\n'
            f'   linkref r{base + 1},r{base + 6}\n'
            f'   load r{base},4\n'
            f'   load r{base + 2},{condition}\n'
            f'   load r{base + 3},{rxas_string(description)}\n'
            f'   load r{base + 4},{line}\n'
            f'   settp r{base + 1},256\n'
            f'   settp r{base + 2},768\n'
            f'   settp r{base + 3},768\n'
            f'   settp r{base + 4},768\n'
            f'   call r{base + 5},rexxactivation.rexxactivationarguments.queuecallcondition(),r{base}\n'
            f'   unlink r{base + 1}\n'
            f'   null r{base + 6}\n')


def queue_imports(text):
    signatures = (
        ('rexxclassiccontext.rexxclassiccontext.activation',
         'reference .rexxactivation..rexxactivationarguments', ''),
        ('rexxactivation.rexxactivationarguments.queuecallcondition',
         '.boolean', 'condition=.int,description=.string,line=.int'),
    )
    for name, result, arguments in signatures:
        declarations = re.findall(r'^(\S+)\(\) \.expose=' + re.escape(name) + r'\s*$', text, re.M)
        assert len(declarations) <= 1, ('one imported queue/accessor declaration', name)
        if declarations:
            assert declarations[0] == name, ('canonical unprefixed imported callable', name, declarations)
        if not declarations:
            text += (f'\n{name}() .expose={name}\n'
                     f'   .meta "{name}"="b" "{result}" {name}() "{arguments}"\n'
                     f'   .meta "{name}"=".autoload" "rxfnsc"\n')
    return text


def inject(text, sites):
    """Insert controlled queue calls at checked offsets in the real C body."""
    header, end = function_extent(text)
    body = text[header.start():end]
    highest = max(map(int, re.findall(r'\br(\d+)\b', body)), default=-1)
    first = max(int(header[1]), highest + 1)
    assert sites and len(set(offset for offset, *_ in sites)) == len(sites)
    assert all(header.end() <= offset <= end for offset, *_ in sites)
    assert first + 7 * len(sites) <= 65536, 'injection stays inside the admitted register budget'
    for index, (offset, line, description, condition) in reversed(list(enumerate(sorted(sites)))):
        text = text[:offset] + queue_snippet(first + index * 7, line, description, condition) + text[offset:]
    text, count = re.subn(r'^(llf1\(\) \.locals=)\d+',
                          lambda match: match[1] + str(first + 7 * len(sites)),
                          text, count=1, flags=re.M)
    assert count == 1
    return queue_imports(text)


def policy_injection(text):
    header, end = function_extent(text)
    policies = [match for match in called(text, 'rexxactivation.rexxactivationarguments.setcallpolicy')
                if header.end() <= match.start() < end]
    assert len(policies) == 2, ('the unchanged native policy fixture has two ON sites', len(policies))
    return inject(text, [(match.end() + 1, line, 'Élan 😀', 2)
                         for match, line in zip(policies, (3, 5))])


def source_anchors(text, filename, line):
    header, end = function_extent(text)
    found = []
    for match in re.finditer(r'^\s*\.srcstep[^\n]*$', text[header.start():end], re.M):
        fields = shlex.split(match[0])
        if len(fields) >= 8 and fields[4] == filename and int(fields[5]) == line:
            found.append(header.start() + match.end() + 1)
    assert found, ('authored source anchor is retained', filename, line)
    return found


def dispatcher(text):
    headers = list(re.finditer(r'^(\S+)\(\) \.locals=\d+[^\n]*$', text, re.M))
    found = []
    for index, header in enumerate(headers):
        end = headers[index + 1].start() if index + 1 < len(headers) else len(text)
        if called(text[header.end():end], 'rexxactivation.rexxactivationarguments.takecallcondition'):
            found.append(header[1])
    assert len(found) == 1 and found[0] != 'llf1', ('one module-private delayed dispatcher', found)
    return found[0]


def call_at_line(text, filename, line, target):
    header, end = function_extent(text); source = None; result = []
    pattern = re.compile(r'^\s*\.srcstep[^\n]*$|^\s*(?:settp)?(?:swap)?call[1-4]?\s+[^\n]*$', re.M)
    for match in pattern.finditer(text, header.end(), end):
        if match[0].lstrip().startswith('.srcstep '):
            fields = shlex.split(match[0]); source = (fields[4], int(fields[5]))
        elif source == (filename, line) and re.search(r',§?' + re.escape(target) + r'\(\)(?:,|\s*$)', match[0]):
            result.append(match)
    assert len(result) == 1, ('one target call at the maintained authored site', filename, line, target, len(result))
    return result[0]


def complete_at_line(text, filename, line):
    return call_at_line(text, filename, line, dispatcher(text)).start()


def controlled_injection(text, stem, scenario):
    filename = stem + '.rexx'
    if scenario == 'policy':
        return policy_injection(text)
    if scenario == 'conditions':
        sites = [(source_anchors(text, filename, line)[0], line, description, condition)
                 for line, description, condition in ((4, 'error', 2), (7, 'failure', 3),
                                                       (10, 'halt', 4), (13, 'notready', 5))]
    elif scenario == 'boundaries':
        sites = [(source_anchors(text, filename, line)[0], line, name, 2)
                 for line, name in ((4, 'if'), (9, 'when'), (15, 'do'))]
    elif scenario == 'transfers':
        sites = [(source_anchors(text, filename, line)[0], line, name, 2)
                 for line, name in ((4, 'leave'), (11, 'exit'), (17, 'return'))]
    elif scenario == 'missing':
        sites = [(source_anchors(text, filename, 3)[0], 3, 'missing', 2)]
    elif scenario == 'buffered_halt':
        # Native inject_buffered_halt queues only after the guarded 'inside'
        # SAY, so the buffered handler invocation cannot enqueue itself again.
        sites = [(source_anchors(text, filename, 3)[0], 3, 'initial', 4),
                 (call_at_line(text, filename, 8, 'llctarget.sayvalue').end() + 1, 8, 'buffered', 4)]
    elif scenario == 'lifecycle':
        header, end = function_extent(text)
        on = [match for match in called(text, 'rexxactivation.rexxactivationarguments.setcallpolicy')
              if header.end() <= match.start() < end]
        assert len(on) == 2, 'two maintained CALL ON sites'
        # OFF uses the same shared policy slot. Its zero setter may be emitted
        # through the native OFF owner or the additive CALL policy owner.
        off = call_at_line(text, filename, 6, 'rexxactivation.rexxactivationarguments.setsignalpolicy')
        sites = [(on[0].end() + 1, 3, 'first', 2),
                 (call_at_line(text, filename, 4, 'llctarget.sayvalue').end() + 1, 4, 'again', 2),
                 (off.end() + 1, 6, 'off', 2),
                 (complete_at_line(text, filename, 9), 9, 'parent', 2)]
    else:
        raise AssertionError(scenario)
    return inject(text, sites)


def export_body(text, name):
    text, header = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$',
                           lambda match: match[1] + ' .expose=' + name + match[2],
                           text, count=1, flags=re.M)
    text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)',
                             lambda match: match[1] + '"' + name + '"',
                             text, count=1, flags=re.M)
    assert header == metadata == 1, 'one test-only borrowed-context entry, without a product ABI change'
    return text


def c_literal(value):
    return json.loads('"' + value + '"')


def native_editor():
    """Translate the maintained native host's responses; do not recreate it."""
    text = (NATIVE / 'src/test_levelc_address_host_callback.c').read_text(encoding='utf-8')
    pattern = re.compile(r'if \(state->calls == (\d+) && strcmp\(request->command, "((?:\\.|[^"\\])*)"\) == 0\) \{([\s\S]*?)\n    \}')
    rows = []
    for match in pattern.finditer(text):
        number, command, body = match.groups()
        assert int(number) == len(rows) + 1, 'native producer response order'
        output = re.findall(r'rxvml_address_emit_output\(ctx, request, "((?:\\.|[^"\\])*)"\)', body)
        error = re.findall(r'rxvml_address_emit_error\(ctx, request, "((?:\\.|[^"\\])*)"\)', body)
        code = re.findall(r'response->rc = (-?\d+);', body)
        condition = re.findall(r'response->condition_name = "([A-Z]+)";', body)
        assert all(len(items) <= 1 for items in (output, error, code, condition))
        rows.append('|'.join((c_literal(command), code[0] if code else '0', condition[0] if condition else '',
                              c_literal(output[0]).encode().hex() if output else '',
                              c_literal(error[0]).encode().hex() if error else '')))
    count = re.findall(r'state\.calls != (\d+)', text)
    assert len(count) == 1 and len(rows) == int(count[0]), 'complete maintained native host response inventory'
    files = re.findall(r'failed \|= check_file\("([^"]+)", "((?:\\.|[^"\\])*)"\);', text)
    assert len(files) == 2, 'both maintained file-output oracles'
    return rows, {name: c_literal(value).encode() for name, value in files}


def initial_shapes(text, source):
    header, end = function_extent(text); body = text[header.start():end]
    assert len(called(body, 'rexxactivation.rexxactivationarguments.setcallpolicy')) == 2
    assert called(text, 'rexxactivation.rexxactivationarguments.takecallcondition'), 'one existing pending-event owner'
    assert called(text, 'rexxactivation.rexxactivationarguments.begincalltrap'), 'existing handler DELAY owner'
    assert called(text, 'rexxpool.rexxvariablepool.setcalltrapsigl'), 'CALL updates ordinary SIGL through its existing owner'
    assert not called(text, 'rexxpool.rexxvariablepool.setsignalline'), 'delayed CALL does not acquire SIGNAL .SIGL updates'
    assert not re.search(r'^\s*sigcalla\b', text, re.M), 'no new arbitrary-event resumption route'
    assert not called(text, 'rexxactivation.rexxactivationarguments.beginclausetime'), 'clock-free native fixture stays clock-free'
    assert not re.search(r'^\s*\.meta\s+"__rxcalltrap_[^"\n]*"=', text, re.M | re.I), 'private delayed helper has no conflicting global callable descriptor'
    assert source.read_bytes() == (SOURCES / source.name).read_bytes(), 'native fixture is read unchanged'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


class Batch:
    def __init__(self, args, directory):
        self.args = args; self.directory = directory; self.commands = 0
        self.build = args.build.resolve()
        self.backend = self.build / 'research/level-l/compilers/backend/opt'
        self.runtime = directory / 'runtime'; self.runtime.mkdir(parents=True, exist_ok=True)
        self.host = directory / 'compiler'; self.host.mkdir(exist_ok=True)
        self.application = directory / 'application'; self.application.mkdir(exist_ok=True)
        suffix = '.exe' if os.name == 'nt' else ''
        self.tools = {name: self.build / 'bin' / (name + suffix)
                      for name in ('rxc', 'rxas', 'rxlink', 'rxbvm')}
        inputs = [*self.tools.values(), self.backend / 'conditions-fixtures-linked.rxbin']
        for name in LIBRARIES:
            root = self.backend if name == 'llctarget' else self.build / 'bin'
            inputs.append(root / (name + '.rxbin'))
        inputs.append(FIXTURES / 'compile-fixtures.crexx')
        inputs.extend(SOURCES / (stem + '.rexx') for stem in (*DELAYED.values(), *ORDINARY,
                      *EXTERNAL, *HOSTED, *FATAL, 'levelc_call_external_handler',
                      'levelc_exit_handler', 'levelc_trace_values'))
        inputs.extend(FIXTURES / name for name in ('parse-fields.crexx', 'bif-stream-condition.crexx', 'unicode-precedence.crexx', 'call-clock.crexx'))
        identity = {str(path): digest(path) for path in inputs}
        record = directory / 'inputs.json'
        if record.exists():
            assert json.loads(record.read_text()) == identity, 'prepared products require byte-identical compiler/runtime/tools/source inputs'
        else:
            record.write_text(json.dumps(identity, sort_keys=True), encoding='utf-8')
        for name in LIBRARIES:
            root = self.backend if name == 'llctarget' else self.build / 'bin'
            shutil.copy2(root / (name + '.rxbin'), self.runtime / (name + '.rxbin'))
        for destination in (self.host, self.application):
            shutil.copy2(self.tools['rxbvm'], destination / ('rxbvm' + suffix))
            for dll in (self.build / 'bin').glob('*.dll'):
                shutil.copy2(dll, destination / dll.name)
            providers = destination / 'providers'; providers.mkdir(exist_ok=True)
            for name in ('rxfs', 'rxcstream'):
                shutil.copy2(self.build / 'bin' / (name + '.rxplugin'), providers / (name + '.rxplugin'))
        self.host_vm = self.host / ('rxbvm' + suffix)
        self.target_vm = self.application / ('rxbvm' + suffix)
        self.service = self.host / 'conditions-fixtures-linked.rxbin'
        shutil.copy2(self.backend / self.service.name, self.service)
        empty = directory / 'empty PATH'; empty.mkdir(exist_ok=True)
        self.env = dict(os.environ, PATH=str(empty), CREXX_TRACE_TO='stdout')
        self.env.pop('CREXX_PROVIDER_PATH', None); self.env.pop('CREXX_TRACE', None)
        assert not any(shutil.which(name, path=self.env['PATH']) for name in ('rxc', 'rxc-c'))

    def run(self, label, command, *, status=0, data=b'', cwd=None, timeout=1800, env=None):
        completed = subprocess.run([str(item) for item in command], cwd=cwd or self.application,
                                   env=self.env if env is None else env, input=data, capture_output=True, timeout=timeout)
        self.commands += 1
        diagnostic = completed.stdout + completed.stderr
        assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-6000:])
        if status is None:
            assert completed.returncode != 0, (label, completed.returncode, diagnostic[-6000:])
        else:
            assert completed.returncode == status, (label, completed.returncode, diagnostic[-6000:])
        return completed

    def prepare(self, stems, *, supplementary=False, external=False, negative=False):
        rows = []
        if external:
            source = SOURCES / 'levelc_call_external_handler.rexx'
            output = self.runtime / 'nativehandler.rxas'
            if not output.exists() or not output.with_suffix('.rxbin').exists():
                rows.append('|'.join((str(source), str(output), source.name, '', '',
                                      'nativehandler.levelc_call_external_handler', '')))
        for stem in stems:
            source = SOURCES / (stem + '.rexx'); output = self.application / (stem + '.rxas')
            if not output.exists() or not output.with_suffix('.rxbin').exists():
                binding = ('nativehandler', 'nativehandler.levelc_call_external_handler') if stem == 'levelc_call_external_trap' else ('', '')
                rows.append('|'.join((str(source), str(output), source.name, *binding, '', '')))
        if supplementary:
            for name in ('parse-fields', 'bif-stream-condition', 'unicode-precedence', 'call-clock'):
                source = FIXTURES / (name + '.crexx'); output = self.application / (name + '.rxas')
                if not output.exists() or not output.with_suffix('.rxbin').exists():
                    rows.append('|'.join((str(source), str(output), source.name, '', '', '', '')))
        negatives = []; negative_receipt = self.directory / 'negative.passed'
        if negative:
            cases = native_negative_cases()
            payload = json.dumps([(name, logical, source.hex(), code, offset, line, column)
                                  for name, logical, source, code, offset, line, column in cases])
            key = hashlib.sha256((payload + digest(self.directory / 'inputs.json')
                                  + inspect.getsource(native_negative_cases)
                                  + inspect.getsource(negative_excerpt)).encode()).hexdigest()
            if not negative_receipt.exists() or negative_receipt.read_text() != key:
                for name, logical, source, code, offset, line, column in cases:
                    path = self.directory / (name + '.rexx'); path.write_bytes(source)
                    output = self.application / (name + '.rxas'); output.write_bytes(b'previous owned output\n')
                    rows.append('|'.join((str(path), str(output), logical, '', '', '', code)))
                    negatives.append((name, logical, code, offset, line, column, output, source))
        if rows:
            manifest = self.directory / 'compile.manifest'
            manifest.write_text('\n'.join(rows) + '\n', encoding='utf-8')
            completed = self.run('one warm B/L fixture batch',
                                 [self.host_vm, '--no-autoload', self.service, '-a', self.runtime, manifest, self.tools['rxas']], cwd=self.host)
            output_rows = completed.stdout.replace(b'\r\n', b'\n').splitlines()
            assert output_rows[-1:] == [f'CONDITIONS_FIXTURES|requests={len(rows)}|PASS'.encode()] and not completed.stderr, (completed.stdout, completed.stderr)
            assert len(output_rows) == 3 * len(negatives) + 1, ('each refusal has exactly one source row and caret row before the batch summary', output_rows)
            for index, expected in enumerate(negatives):
                name, logical, code, offset, line, column, output, source = expected
                actual, excerpt, caret = output_rows[index * 3:index * 3 + 3]
                fields = actual.decode().split('|', 3)
                assert len(fields) == 4 and fields[0] == 'REFUSAL' and fields[1] == logical and int(fields[2]) == offset, (name, actual, expected[:6])
                assert 'RXC-LC-' + code in fields[3] and f'at line {line}, column {column} (byte {offset})' in fields[3], (name, fields[3], expected[:6])
                assert [excerpt, caret] == negative_excerpt(source, line, column), ('exact bounded refusal source/caret rows', name, excerpt, caret)
                assert output.read_bytes() == b'previous owned output\n' and not Path(str(output) + '.llrxc-stage').exists(), ('failed publication is atomic', name)
            if negatives:
                negative_receipt.write_text(key, encoding='utf-8')

    def compile_b(self, source, stem, *, extra=()):
        output = self.application / stem
        completed = self.run('prepare B test adapter ' + source.name,
                             [self.tools['rxc'], '-x', '--no-exe-import', '-i', self.runtime,
                              '-o', output, source])
        assert not completed.stdout and not completed.stderr
        return self.link(output.with_suffix('.rxas'), extra=extra)

    def checked_run(self, stem, image, expected, *, data=b'', status=0, arguments=(), env=None):
        completed = self.run(stem, [self.target_vm, '--no-autoload', image,
                                   *(['-a', *arguments] if arguments else [])],
                             status=status, data=data, env=env)
        assert completed.stdout.replace(b'\r\n', b'\n') == expected and not completed.stderr, (stem, completed.stdout, completed.stderr, expected)
        return completed

    def link(self, assembly, *, extra=()):
        binary = assembly.with_suffix('.rxbin')
        # The warm service already assembled original sources once. Only an
        # injected/export-adapted source needs another assembler request.
        if not binary.exists() or binary.stat().st_mtime_ns < assembly.stat().st_mtime_ns:
            completed = self.run('assemble controlled fixture', [self.tools['rxas'], '-o', binary, assembly])
            assert not completed.stdout and not completed.stderr
        linked = assembly.with_name(assembly.stem + '-linked.rxbin')
        completed = self.run('link static C fixture', [self.tools['rxlink'], '-o', linked, binary, *extra,
                                                     *(self.runtime / (name + '.rxbin') for name in LIBRARIES)])
        assert not completed.stdout and not completed.stderr
        return linked

    def inspect(self, path):
        if self.args.inspect:
            self.args.inspect.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, self.args.inspect / path.name)

    def representative(self):
        stem = DELAYED['policy']; self.prepare((stem,))
        original = self.application / (stem + '.rxas'); text = original.read_text()
        initial_shapes(text, SOURCES / (stem + '.rexx')); self.inspect(original)
        injected = original.with_name(stem + '-queued.rxas')
        injected.write_text(policy_injection(text), encoding='utf-8'); self.inspect(injected)
        image = self.link(injected)
        expected = native_assignment(NATIVE / 'levelc_call_delayed_injection.py', 'expected_by_scenario')['policy'].encode('utf-8')
        self.checked_run('unchanged native local/BIF delayed fixture', image, expected)
        (self.directory / 'representative.passed').write_text(self.representative_identity(), encoding='utf-8')

    def representative_identity(self):
        original = self.application / (DELAYED['policy'] + '.rxas')
        functions = (function_extent, called, rxas_string, queue_snippet,
                     queue_imports, inject, policy_injection, initial_shapes,
                     native_assignment)
        payload = digest(original) + digest(NATIVE / 'levelc_call_delayed_injection.py')
        payload += ''.join(inspect.getsource(function) for function in functions)
        return hashlib.sha256(payload.encode()).hexdigest()

    def delayed(self):
        expected = native_assignment(NATIVE / 'levelc_call_delayed_injection.py', 'expected_by_scenario')
        for scenario, stem in DELAYED.items():
            if scenario == 'policy':
                receipt = self.directory / 'representative.passed'
                if not receipt.exists() or receipt.read_text() != self.representative_identity():
                    self.representative()
                continue
            original = self.application / (stem + '.rxas')
            queued = original.with_name(stem + '-queued.rxas')
            queued.write_text(controlled_injection(original.read_text(), stem, scenario), encoding='utf-8')
            self.inspect(queued); image = self.link(queued)
            if scenario == 'missing':
                completed = self.run('native missing delayed handler', [self.target_vm, '--no-autoload', image], status=None)
                assert not completed.stdout and b'CLASSIC_SYNTAX' in completed.stderr and b'RXC-LC-16.1' in completed.stderr, (completed.stdout, completed.stderr)
                assert b'levelc_call_missing_delayed_handler.rexx:3:' in completed.stderr, completed.stderr
            else:
                self.checked_run('native delayed ' + scenario, image, expected[scenario].encode())
        stem = 'levelc_exit_handler'; original = self.application / (stem + '.rxas')
        text = original.read_text(); queued = original.with_name(stem + '-queued.rxas')
        queued.write_text(inject(text, [(source_anchors(text, stem + '.rexx', 3)[0], 3, 'exit', 2)]), encoding='utf-8')
        tree = ast.parse((NATIVE / 'levelc_exit_handler.py').read_text())
        oracle = [(ast.literal_eval(node.args[-2]), ast.literal_eval(node.args[-1]))
                  for node in ast.walk(tree) if isinstance(node, ast.Call)
                  and isinstance(node.func, ast.Name) and node.func.id == 'run'
                  and len(node.args) == 5 and isinstance(node.args[-2], ast.Constant)
                  and isinstance(node.args[-2].value, int)]
        assert len(oracle) == 1, 'one maintained native handler EXIT result'
        self.checked_run('native handler EXIT', self.link(queued), oracle[0][1], status=oracle[0][0])

    def ordinary(self):
        for stem in ORDINARY:
            assembly = self.application / (stem + '.rxas'); self.inspect(assembly)
            data = SOURCES / (stem + '.input')
            self.checked_run('unchanged native ' + stem, self.link(assembly), ordinary_output(stem),
                             data=data.read_bytes() if data.exists() else b'')
        for stem, code in FATAL.items():
            image = self.link(self.application / (stem + '.rxas'))
            completed = self.run('native fatal ' + stem, [self.target_vm, '--no-autoload', image], status=None)
            assert completed.stdout.replace(b'\r\n', b'\n') == b'before\n', (stem, completed.stdout)
            assert b'CLASSIC_SYNTAX' in completed.stderr and ('RXC-LC-' + code).encode() in completed.stderr, (stem, completed.stderr)
            assert (stem + '.rexx:3:').encode() in completed.stderr, (stem, completed.stderr)

    def external(self):
        for stem, line, description, variable in (
                ('levelc_call_external_trap', 5, 'external', 'trap_output'),
                ('levelc_call_signal_override', 4, 'replaced', 'override_output')):
            original = self.application / (stem + '.rxas'); text = original.read_text()
            queued = original.with_name(stem + '-queued.rxas')
            queued.write_text(inject(text, [(source_anchors(text, stem + '.rexx', line)[0], line, description, 2)]), encoding='utf-8')
            image = self.link(queued, extra=(self.runtime / 'nativehandler.rxbin',))
            expected = native_comparison(NATIVE / 'levelc_call_external.py', variable)
            self.checked_run('unchanged native ' + stem, image, expected)

    def address(self):
        stem = 'levelc_address_host_callback'
        original = self.application / (stem + '.rxas')
        exported = self.runtime / 'nativeaddressfixture.rxas'
        exported.write_text(export_body(original.read_text(), 'nativeaddressfixture.run'), encoding='utf-8')
        completed = self.run('assemble test-only native ADDRESS entry', [self.tools['rxas'], exported])
        assert not completed.stdout and not completed.stderr
        rows, files = native_editor(); manifest = self.directory / 'native-editor.manifest'
        manifest.write_text('\n'.join(rows) + '\n', encoding='utf-8')
        image = self.compile_b(FIXTURES / 'address-host.crexx', 'native-address-host', extra=(exported.with_suffix('.rxbin'),))
        self.checked_run('unchanged native ADDRESS source with existing B environment', image,
                         f'NATIVE_ADDRESS_HOST|calls={len(rows)}|PASS\n'.encode(), arguments=(manifest,))
        for name, expected in files.items():
            path = self.application / name
            assert path.read_bytes().replace(b'\r\n', b'\n') == expected, ('native host file oracle', name, path.read_bytes(), expected)
            path.unlink()
        stem = 'levelc_address_notready'
        # This maintained fixture executes cat after its missing-input SIGNAL
        # control. Give only that runtime invocation the required host utility.
        cat = shutil.which('cat')
        assert cat, 'the maintained native stream fixture requires host cat'
        cat = Path(cat).resolve(strict=True)
        cat_directory = self.directory / 'stream PATH'; cat_directory.mkdir(exist_ok=True)
        cat_copy = cat_directory / ('cat.exe' if os.name == 'nt' else 'cat')
        if not cat_copy.exists():
            try:
                cat_copy.symlink_to(cat)
            except OSError:
                shutil.copy2(cat, cat_copy)
        assert set(path.name for path in cat_directory.iterdir()) == {cat_copy.name}, 'runtime PATH contains only the required cat utility'
        assert digest(cat_copy) == digest(cat), 'runtime utility is the discovered owning host cat'
        runtime_env = dict(self.env, PATH=str(cat_directory))
        assert shutil.which('cat', path=runtime_env['PATH'])
        assert not any(shutil.which(name, path=runtime_env['PATH']) for name in ('rxc', 'rxc-c'))
        assert not list(Path(self.env['PATH']).iterdir()), 'compiler and default target PATH remain empty'
        expected = ordinary_output(stem)
        rows = (SOURCES / (stem + '.rexx')).read_text().splitlines()
        declared = [row for row in rows if row.startswith('address crexx with input stem ')]
        assert declared == ['address crexx with input stem missing.'], 'one unchanged native raw stem resource declaration'
        resource = declared[0].removeprefix('address crexx with input stem ') + '0'
        native_slot = ('Invalid ADDRESS input stem count ' + resource.upper()).encode()
        source_slot = ('Invalid ADDRESS input stem count ' + resource).encode()
        assert expected.count(native_slot) == 1, 'one maintained diagnostic stem slot admits raw source spelling'
        expected = expected.replace(native_slot, source_slot, 1)
        self.checked_run('unchanged native real stream NOTREADY',
                         self.link(self.application / (stem + '.rxas')), expected, env=runtime_env)

    def trace(self):
        checker = runpy.run_path(str(FIXTURES.parent / 'trace/check-trace.py'))
        # Native optimisation can omit reads. Both maintained transcripts own
        # retained kind/value pairs; the finite additions below are exact BIF
        # results, a static option and C22 PARSE/ARG stores those transcripts omit.
        stem = 'levelc_trace_values'
        completed = self.run('unchanged native practical TRACE source',
                             [self.target_vm, '--no-autoload', self.link(self.application / (stem + '.rxas'))])
        plain, events = checker['records'](completed)
        rows = (SOURCES / (stem + '.rexx')).read_text().splitlines()
        expected_values = {}
        for mode in ('opt', 'noopt'):
            reference = subprocess.CompletedProcess([], 0, (SOURCES / (stem + '_' + mode + '.expected')).read_bytes(), b'')
            expected_plain, expected_events = checker['records'](reference)
            assert plain == expected_plain, ('native ordinary TRACE behavior', mode, plain, expected_plain)
            for kind, line, value in expected_events:
                if kind.startswith('>'):
                    expected_values.setdefault(line, set()).add((kind, value))
        additions = {
            "nul = x2c('00')": {('>F>', '\0')},
            'say c2x(nul)': {('>F>', '00')},
            "parse value 'a b' with a b": {('>>>', 'a'), ('>>>', 'b')},
            "say 'mode' trace()": {('>F>', 'I')},
            '  arg p': {('>>>', 'É')},
            '  trace o': {('>L>', 'o')},
        }
        for authored, values in additions.items():
            lines = [line for line, row in enumerate(rows, 1) if row == authored]
            assert len(lines) == 1, ('one unchanged authored source row for each additional exact observation', authored, lines)
            expected_values.setdefault(lines[0], set()).update(values)
        retained = 0
        for kind, line, value in events:
            if kind == 'source':
                assert 1 <= line <= len(rows), ('native authored source line', line)
                assert value == rows[line - 1], ('truthful native source clause', line, value)
            elif kind.startswith('>'):
                assert (kind, value) in expected_values.get(line, set()), ('native checked kind/value at its authored clause', kind, line, repr(value), expected_values.get(line))
                retained += 1
            else:
                assert kind == 'file' and value == stem + '.rexx', ('only the stable logical source file may accompany scalar records', kind, line, value)
        assert retained > 0, 'representative native checked scalar observations remain live'

    def observations(self):
        checker = runpy.run_path(str(FIXTURES.parent / 'trace/check-trace.py'))
        source = FIXTURES / 'parse-fields.crexx'
        image = self.link(self.application / 'parse-fields.rxas')
        normal = self.run('new PARSE/ARG/PULL field/drop records',
                          [self.target_vm, '--no-autoload', image, '-a', 'normal', 'raw skip kept'],
                          data=b'one skip final\n')
        plain, events = checker['records'](normal)
        assert plain == ('FIELDS|é雪\0|207461696C20|RAW|KEPT|ONE|FINAL\n').encode(), plain
        expected = {
            "parse value PAYLOAD with HEAD": [('>>>', 'é雪\0'), ('>.>', 'drop'), ('>>>', ' tail ')],
            'arg ., FIRSTARG': [('>.>', 'NORMAL'), ('>>>', 'RAW'), ('>.>', 'SKIP'), ('>>>', 'KEPT')],
            'pull PULLFIRST': [('>>>', 'ONE'), ('>.>', 'SKIP'), ('>>>', 'FINAL')],
        }
        rows = source.read_text().splitlines()
        for needle, values in expected.items():
            lines = [number for number, row in enumerate(rows, 1) if row.startswith(needle)]
            assert len(lines) == 1
            actual = [(kind, value) for kind, line, value in events if line == lines[0] and kind in ('>>>', '>.>')]
            assert actual == values, ('ordered actual fields/dots', needle, actual, values)
        partial = self.run('new PARSE partial failure observation order',
                           [self.target_vm, '--no-autoload', image, '-a', 'partial'])
        plain, events = checker['records'](partial)
        assert plain == b'PARTIAL|left|old mid|old last|NOVALUE|MISSING_PATTERN\n', plain
        line = next(number for number, row in enumerate(rows, 1) if "parse value 'left:drop=tail'" in row)
        actual = [(kind, value) for kind, number, value in events if number == line and kind in ('>>>', '>.>')]
        assert actual == [('>>>', 'left'), ('>.>', 'drop')], ('earlier records survive; later fields do not exist', actual)

    def bif_stream(self):
        original = self.application / 'bif-stream-condition.rxas'
        exported = self.runtime / 'bifstreamfixture.rxas'
        exported.write_text(export_body(original.read_text(), 'bifstreamfixture.run'), encoding='utf-8')
        completed = self.run('assemble BIF condition test-only entry', [self.tools['rxas'], exported])
        assert not completed.stdout and not completed.stderr
        image = self.compile_b(FIXTURES / 'bif-stream-host.crexx', 'bif-stream-host', extra=(exported.with_suffix('.rxbin'),))
        source = (FIXTURES / 'bif-stream-condition.crexx').read_text().splitlines()
        line = next(number for number, row in enumerate(source, 1) if '/* CAUSING_ADDRESS */' in row)
        error_line = next(number for number, row in enumerate(source, 1) if '/* CAUSING_BIF_ERROR */' in row)
        completed = self.run('BIF handler stream condition returns to its source parent',
                             [self.target_vm, '--no-autoload', image, '-a', str(line), str(error_line)])
        assert re.fullmatch(rb'BIF_STREAM_CONDITIONS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)

    def precedence(self):
        source = FIXTURES / 'unicode-precedence.crexx'
        line = next(number for number, row in enumerate(source.read_text().splitlines(), 1) if row == 'nop')
        original = self.application / 'unicode-precedence.rxas'; text = original.read_text()
        queued = self.application / 'unicode-precedence-queued.rxas'
        queued.write_text(inject(text, [(source_anchors(text, source.name, line)[0], line, 'precedence', 2)]), encoding='utf-8')
        image = self.link(queued)
        completed = self.run('quoted Unicode first trace-only handler refuses without later fallback',
                             [self.target_vm, '--no-autoload', image], status=None)
        assert not completed.stdout and b'CLASSIC_SYNTAX' in completed.stderr and b'RXC-LC-16.2' in completed.stderr, (completed.stdout, completed.stderr)
        assert f'{source.name}:{line}:'.encode() in completed.stderr, completed.stderr

    def clock(self):
        source = FIXTURES / 'call-clock.crexx'
        line = next(number for number, row in enumerate(source.read_text().splitlines(), 1) if '/* QUEUED_CLOCK_ERROR */' in row)
        original = self.application / 'call-clock.rxas'; text = original.read_text()
        queued = self.application / 'call-clock-queued.rxas'
        queued.write_text(inject(text, [(source_anchors(text, source.name, line)[0], line, 'clock', 2)]), encoding='utf-8')
        self.inspect(queued); image = self.link(queued)
        completed = self.run('parent/handler clause clocks across delayed completion and empty WHILE',
                             [self.target_vm, '--no-autoload', image], timeout=60)
        assert completed.stdout.replace(b'\r\n', b'\n') == b'CALL_CLOCK|1|1|1|1|triggered|prior result|prior result\n' and not completed.stderr, (completed.stdout, completed.stderr)

    def contracts(self):
        image = self.host / 'conditions-observations-contracts-linked.rxbin'
        shutil.copy2(self.backend / image.name, image)
        completed = self.run('tiny independent C22 admission/ownership contracts',
                             [self.host_vm, '--no-autoload', image, '-a', self.runtime], cwd=self.host)
        assert re.fullmatch(rb'CONDITIONS_OBSERVATIONS_CONTRACTS\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--prepared', type=Path)
    parser.add_argument('--inspect', type=Path)
    parser.add_argument('--phase', choices=('representative', 'all', 'contracts', 'observations'), default='all')
    args = parser.parse_args(); args.work.mkdir(parents=True, exist_ok=True)
    if args.prepared:
        args.prepared.mkdir(parents=True, exist_ok=True)
    manager = nullcontext(str(args.prepared.resolve())) if args.prepared else tempfile.TemporaryDirectory(prefix='conditions-observations-', dir=args.work)
    with manager as temporary:
        batch = Batch(args, Path(temporary))
        if args.phase == 'all':
            batch.prepare((*DELAYED.values(), 'levelc_exit_handler', *ORDINARY, *EXTERNAL,
                           *HOSTED, *FATAL, 'levelc_trace_values'), supplementary=True, external=True, negative=True)
            batch.delayed(); batch.ordinary(); batch.external(); batch.address()
            batch.trace(); batch.observations(); batch.bif_stream(); batch.precedence(); batch.clock(); batch.contracts()
        elif args.phase == 'representative':
            batch.representative()
        elif args.phase == 'contracts':
            batch.contracts()
        else:
            batch.prepare((), supplementary=True); batch.observations()
        print(f'CONDITIONS_OBSERVATIONS|phase={args.phase}|commands={batch.commands}|PASS')


if __name__ == '__main__':
    main()
