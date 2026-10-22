#!/usr/bin/env python3
"""Keep C requests on the private prepared-language borrowing boundary.

The independent L checker owns the downstream field/reader/CFG checks. This
consumer check distinguishes the cold module initializer from parser/name hot
paths without depending on register numbering or source/debug record counts.
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


def check(directory):
    code = {}
    for module in ('llclanguage', 'llcsymbols', 'llcparse', 'llcdoprotocol', 'llcprocedureprotocol'):
        code.update(routines((directory / f'{module}.rxas').read_text()))
    require('llclanguage.prepare' in code, 'missing private owner initializer')
    prepare = code['llclanguage.prepare']
    require(any('".initializer"' in line for line in prepare), 'owner preparation is not a module initializer')
    owners = [(name, target) for name, body in code.items() for target in calls(body)
              if target == 'llcgrammar.language']
    require(owners == [('llclanguage.prepare', 'llcgrammar.language')], 'generated language must be constructed only once in the cold owner initializer')
    accessor = code['llclanguage.prepared']
    require(any(op == 'mkref' and len(args) == 2 and re.fullmatch(r'g\d+', args[1])
                for op, args in instructions(accessor)), 'prepared accessor must reference anchored module storage')
    require(not calls(accessor), 'prepared accessor must not reconstruct or snapshot a definition')
    for name in ('llcparse.parsed.§factory', 'llcparse.headerpolicy', 'llcsymbols.checked.symbol'):
        require(name in code, f'missing consumer {name}')
        targets = calls(code[name])
        require('lllanguage.request.§factory.borrowed' in targets, f'{name} must use a borrowed request')
        require('lllanguage.request.§factory' not in targets and 'llcgrammar.language' not in targets,
                f'{name} constructs an owning language/request')
        require('lllanguage.definition.usecomments' not in targets, f'{name} mutates shared comment policy')
    for name in ('llcparse.headerpolicy', 'llcparse.parsemessage'):
        require(any('"reference .lllanguage..definition" a1' in line for line in code[name]),
                f'{name} must take a definition reference')
    require('llcsymbols.checked.symbol' in calls(code['llcdoprotocol.checked.symbol']),
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
