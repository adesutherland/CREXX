#!/usr/bin/env python3
"""Keep C requests on the private prepared-language borrowing boundary.

The independent L checker owns the downstream field/reader/CFG checks. This
consumer check distinguishes the scalar module initializer and first-use
preparation from parser/name hot paths without depending on register numbering or source/debug record counts.
"""
from pathlib import Path
import re
import sys


def require(condition, message):
    if not condition:
        raise ValueError(message)


def routines(text):
    result = {}
    body = None
    for line in text.splitlines():
        if re.match(r'^\S+\(.*\s\.(?:locals=|expose=)', line):
            body = [] if '.locals=' in line else None
        elif body is not None:
            body.append(line)
            match = re.match(r'\s*\.meta "([^"]+)"="b" "[^"]+" \S+\(\) "', line)
            if match:
                result[match[1]] = body
    return result


def instructions(body):
    for line in body:
        match = re.match(r'\s+([a-z][a-z0-9]*)\s+(.+)$', line)
        if match:
            yield match[1], tuple(x.strip() for x in match[2].split(','))


def calls(body):
    return [arg.lstrip('§')[:-2] for op, args in instructions(body)
            if re.fullmatch(r'(?:settp)?(?:swap)?call[1-4]?', op)
            for arg in args if arg.endswith('()')]


def delegates_symbol(body):
    wrapper = 'llcsymbols.checked.symbol'
    recognize = 'llcsymbols.checked.recognize'
    targets = [target for target in calls(body) if target in (wrapper, recognize)]
    if targets == [wrapper]:
        return True
    if targets != [recognize]:
        return False
    for index, line in enumerate(body):
        row = next(instructions([line]), None)
        if row is None:
            continue
        op, args = row
        if len(args) < 2 or args[1].lstrip('§') != recognize + '()':
            continue
        if op == 'call' and len(args) == 3 and re.fullmatch(r'r\d+', args[2]):
            # The counted window is receiver, name, label mode.
            mode = 'r' + str(int(args[2][1:]) + 3)
            expected = {args[2]: '3', mode: '0'}
        elif op == 'call3' and len(args) == 5:
            expected = {args[4]: '0'}
        else:
            return False
        # Require an uninterrupted local setup: no label can enter after a
        # required load, and no intervening instruction can replace its value.
        for prior in reversed(body[:index]):
            if re.match(r'^\S+:', prior.strip()):
                return False
            setup = next(instructions([prior]), None)
            if setup is None:
                continue
            setup_op, setup_args = setup
            if setup_op == 'settp' and len(setup_args) == 2 and setup_args[0] not in expected:
                continue
            if setup_op != 'load' or len(setup_args) != 2 or setup_args[0] not in expected:
                return False
            if setup_args[1] != expected.pop(setup_args[0]):
                return False
            if not expected:
                return True
        return False
    return False


def token_roles_consistent(pairs):
    by_role, by_name = {}, {}
    for role, name in pairs:
        if not 1 <= role <= 58:
            return False
        if role in by_role and by_role[role] != name:
            return False
        if name in by_name and by_name[name] != role:
            return False
        by_role[role] = name; by_name[name] = role
    return bool(pairs)


def token_role_controls():
    good = [(1, 'identifier'), (2, 'separator'), (1, 'identifier')]
    require(token_roles_consistent(good), 'fixed token-role control')
    for bad in ([*good, (1, 'string')], [*good, (3, 'identifier')],
                [*good, (0, 'invalid')], [*good, (59, 'invalid')]):
        require(not token_roles_consistent(bad), 'conflicting/out-of-range token-role negative')


def check_token_roles():
    token_role_controls()
    source = (Path(__file__).resolve().parents[1] / 'levelc' / 'llcparse.crexx').read_text()
    factory = source.split('parsed: class', 1)[1].split('  status: method', 1)[0]
    pairs = [(int(role), name) for role, name in re.findall(
        r"tokenrole\(\s*language_ref\s*,\s*roleids_ref\s*,\s*(\d+)\s*,\s*'([^']+)'\s*\)", factory)]
    require(token_roles_consistent(pairs) and {role for role, _ in pairs} == set(range(1, 59)),
            'each bounded C token role must map to one exact literal and vice versa')
    require(not re.search(r"language\.tokenid\(\s*'", factory),
            'hot literal token lookups must use their request-local roles')
    require(factory.count('roleids = .int[]; roleids_ref = reference roleids') == 1,
            'fixed token roles must have one request-local owner')


def check(directory):
    check_token_roles()
    code = {}
    for module in ('llclanguage', 'llcsymbols', 'llcparse', 'llcdoprotocol', 'llcprocedureprotocol'):
        code.update(routines((directory / f'{module}.rxas').read_text()))
    require('llcparse.tokenrole' in code, 'missing private literal token-role helper')
    role = code['llcparse.tokenrole']
    require(calls(role).count('lllanguage.definition.tokenid') == 1
            and not any(target.startswith('llcgrammar.') for target in calls(role)),
            'token-role misses must retain one original token lookup authority')
    require(any('"reference .lllanguage..definition" a1' in line for line in role)
            and any('"reference .int[*]" a2' in line for line in role),
            'token roles must borrow their request language and local cache')
    require('llclanguage.prepare' in code, 'missing private owner initializer')
    prepare = code['llclanguage.prepare']
    require(any('".initializer"' in line for line in prepare), 'owner preparation is not a module initializer')
    owners = [(name, target) for name, body in code.items() for target in calls(body)
              if target == 'llcgrammar.language']
    require(not calls(prepare), 'module initialization must not prepare or analyze a grammar')
    require(owners == [('llclanguage.prepared', 'llcgrammar.language')],
            'one first-use path owns generated language construction')
    accessor = code['llclanguage.prepared']
    rows = list(instructions(accessor))
    joined = '\n'.join(accessor)
    def global_cell(name):
        match = re.search(r'\.meta "llclanguage\.' + name + r'"="b" "[^"]+" (g\d+)\b', joined)
        require(match, f'missing anchored {name} cell')
        return match[1]
    definition = global_cell('_definition'); state = global_cell('_state')
    factory = next(i for i, (_, args) in enumerate(rows) if 'llcgrammar.language()' in args)
    parser = next(i for i, (_, args) in enumerate(rows) if any(arg.lstrip('§') == 'lllr.parser.run()' for arg in args))
    publications = [i for i, (op, args) in enumerate(rows)
                    if op == 'copy' and len(args) == 2 and args[0] == definition]
    ready = [i for i, (op, args) in enumerate(rows) if op == 'load' and args == (state, '2')]
    require(len(publications) == len(ready) == 1 and factory < parser < publications[0] < ready[0],
            'publish the complete local owner only after scanner/parser checks, then mark ready')
    require(any(op == 'load' and args == (state, '1') for op, args in rows[:factory]),
            'first use must mark construction in progress before entering generated code')
    require(any(op == 'mkref' and len(args) == 2 and args[1] == definition
                for op, args in rows[:factory]), 'ready requests return the anchored owner before construction')
    require(any(op == 'load' and args == (state, '3') for op, args in rows)
            and 'rxfnsb.signalrelay' in calls(accessor)
            and 'LL_BORROW_OWNER: recursive C language preparation' in joined,
            'failed and reentrant preparation must not publish or retry a partial owner')
    for name in ('llcparse.parsed.§factory', 'llcparse.headerpolicy', 'llcsymbols.checked.recognize'):
        require(name in code, f'missing consumer {name}')
        targets = calls(code[name])
        require('lllanguage.request.§factory.borrowed' in targets, f'{name} must use a borrowed request')
        require('lllanguage.request.§factory' not in targets and 'llcgrammar.language' not in targets,
                f'{name} constructs an owning language/request')
        require('lllanguage.definition.usecomments' not in targets, f'{name} mutates shared comment policy')
    recognize = calls(code['llcsymbols.checked.recognize'])
    require(recognize.count('llclanguage.prepared') == 1
            and recognize.count('lllanguage.request.§factory.borrowed') == 1,
            'symbol admission must borrow the prepared language exactly once')
    for name in ('llcsymbols.checked.symbol', 'llcsymbols.checked.label'):
        require(name in code, f'missing consumer {name}')
        require(calls(code[name]) == ['llcsymbols.checked.recognize'],
                f'{name} must delegate only to shared borrowed recognition')
    for name in ('llcparse.headerpolicy', 'llcparse.parsemessage'):
        require(any('"reference .lllanguage..definition" a1' in line for line in code[name]),
                f'{name} must take a definition reference')
    require(delegates_symbol(code['llcdoprotocol.checked.symbol']),
            'DO name admission must delegate the shared lexical authority')
    require(not any(target.startswith('llcdoprotocol.') for target in calls(code['llcprocedureprotocol.checked.run'])),
            'PROCEDURE symbol/error queries must not execute another DO traversal')


if __name__ == '__main__':
    try:
        require(len(sys.argv) == 2, 'usage: check-prepared-rxas.py <built C module directory>')
        check(Path(sys.argv[1]))
    except (OSError, ValueError) as error:
        print(f'C_PREPARED_RXAS|FAIL|{error}')
        sys.exit(1)
    print('C_PREPARED_RXAS|PASS')
