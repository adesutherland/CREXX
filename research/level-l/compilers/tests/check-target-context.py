#!/usr/bin/env python3
"""Inspect the approved borrowed-context runtime's generated RXAS.

This reads two files only: no build, compiler, linker or VM subprocess runs.
It checks executable instructions separately from source/TRACE metadata, and
derives reference registers and attribute slots from metadata rather than
freezing register allocation. Runtime tests own values, errors and lifetimes.

The clean llctarget surface is checked here independently of generated C.
Generated caller ownership and same-frame numeric installation are checked by
the C-only end-to-end harness. Argument/result transport, BIF configuration
getters and the checked host-context bundle snapshot retain their existing
value-copy costs; activation, pool and configuration owners remain borrowed.
"""

import argparse
from dataclasses import dataclass, field
from pathlib import Path
import re
import shlex
import sys
from typing import Optional


HEADER = re.compile(r'^(\S+)\(\)\s+(.*)$')
SIGNATURE = re.compile(r'^\.meta "([^"]+)"="b" "([^"]+)" \S+\(\) "([^"]*)"$')
VARIABLE = re.compile(r'^\.meta "([^"]+)"="b" "([^"]+)" ([ar]\d+)$')
ATTRIBUTE = re.compile(r'^\.meta "([^"]+)"="b" "([^"]+)" \.attr (\d+)$')
CONTEXT = 'rexxclassiccontext.rexxclassiccontext'
CONTEXT_TYPE = '.rexxclassiccontext..rexxclassiccontext'
BIF = '§rexxclassicbifs.rexxbifcallcontext'
CHECKED = 'rexxclassicbifs.rexxclassicbif_checked'
checks = 0


def require(condition, message):
    global checks
    checks += 1
    if not condition:
        raise ValueError(message)


@dataclass
class Procedure:
    name: str
    export: Optional[str]
    result: str = ''
    arguments: str = ''
    variables: dict = field(default_factory=dict)
    instructions: list = field(default_factory=list)
    sources: list = field(default_factory=list)

    def calls(self):
        return [(index, target) for index, (opcode, operands) in enumerate(self.instructions)
                if 'call' in opcode
                for target in re.findall(r'([^\s,()]+)\(\)', operands)]

    def call_index(self, target):
        matches = [index for index, name in self.calls() if name == target]
        require(len(matches) == 1, f'{self.name}: expected one direct call to {target}, got {len(matches)}')
        return matches[0]


def read_rxas(path):
    procedures, attributes = {}, {}
    current = None
    source = None
    for raw in path.read_text(encoding='utf-8').splitlines():
        row = raw.strip()
        header = HEADER.match(raw)
        if header:
            name, options = header.groups()
            current = None
            source = None
            if '.locals=' in options:
                exported = re.search(r'\.expose=(\S+)', options)
                require(name not in procedures, f'{path.name}: duplicate procedure {name}')
                current = Procedure(name, exported.group(1) if exported else None)
                procedures[name] = current
            continue  # Imported declarations have no executable body.
        attribute = ATTRIBUTE.match(row)
        if attribute and attribute[1].startswith(CONTEXT + '.'):
            attributes[attribute[1].removeprefix(CONTEXT + '.')] = (attribute[2], int(attribute[3]))
        if current is None:
            continue
        if row.startswith('.srcstep '):
            fields = shlex.split(row)
            source = (fields[4], int(fields[5]), fields[8])
        signature = SIGNATURE.match(row)
        if signature:
            current.result, current.arguments = signature[2], signature[3]
        variable = VARIABLE.match(row)
        if variable:
            current.variables[variable[1].rsplit('.', 1)[-1]] = (variable[2], variable[3])
        row = re.sub(r'^\S+:\s*', '', row)
        if not row or row.startswith(('.', '/*', '*', '*/')):
            continue
        opcode, _, operands = row.partition(' ')
        current.instructions.append((opcode, operands.strip()))
        current.sources.append(source)
    return procedures, attributes


def signal_returns(procedure, expected):
    signals = [index for index, (op, args) in enumerate(procedure.instructions)
               if op == 'signal' and args.startswith('"REFERENCE_INVALID",')]
    require(len(signals) == expected, f'{procedure.name}: wrong invalid-reference signal count')
    for index in signals:
        require(index + 1 < len(procedure.instructions)
                and procedure.instructions[index + 1][0] == 'ret',
                f'{procedure.name}: skipped invalid-reference signal must return before further work')
    return signals


def reference_flow(procedure, attributes):
    """Follow direct reference payloads and attribute links, not register IDs."""
    slots = {slot: name for name, (_, slot) in attributes.items()}
    values = {register: name for name, (kind, register) in procedure.variables.items()
              if kind.startswith('reference ') and register.startswith('a')}
    links, validated, writes, returns = {}, [], [], []
    for index, (op, operands) in enumerate(procedure.instructions):
        parts = operands.split(',')
        if op == 'linkattr1':
            links[parts[0]] = slots.get(int(parts[2]))
        elif op == 'unlink':
            links.pop(parts[0], None)
        elif op == 'refvalid':
            validated.append((index, links.get(parts[1]) or values.get(parts[1])))
        elif op == 'copy':
            destination, source = parts
            if destination in links:
                writes.append((index, links[destination], values.get(source)))
            else:
                values[destination] = values.get(source)
        elif op == 'ret' and parts[0] in links:
            returns.append(links[parts[0]])
    return validated, writes, returns


def attribute_borrows(procedure, attributes):
    slots = {slot: name for name, (_, slot) in attributes.items()}
    links, borrowed = {}, {}
    for op, operands in procedure.instructions:
        parts = operands.split(',')
        if op == 'linkattr1':
            links[parts[0]] = slots.get(int(parts[2]))
        elif op == 'linkref':
            borrowed[parts[0]] = links.get(parts[1])
        elif op == 'unlink':
            links.pop(parts[0], None)
    return borrowed


def check_context(procedures, attributes):
    expected_attributes = {
        'activation_ref': 'reference .rexxactivation..rexxactivationarguments',
        'pool_ref': 'reference .rexxpool..rexxvariablepool',
        'config_ref': 'reference .rexxclassicconfig..rexxclassicconfig',
        'zz_parse_observations': '.boolean',
    }
    require({name: kind for name, (kind, _) in attributes.items()} == expected_attributes,
            'context: expected three borrowed references and the observation flag')
    require({name: slot for name, (_, slot) in attributes.items()}
            == {'activation_ref': 1, 'config_ref': 2, 'pool_ref': 3, 'zz_parse_observations': 4},
            'context: observation flag must preserve existing reference-field ordinals')
    primitives = {'§factory', 'activation', 'pool', 'config', 'bindpool'}
    observers = {'enableparseobservations', 'parseobservations'}
    expected = primitives | observers | {'readsymbol'}
    public = {p.export.removeprefix(CONTEXT + '.'): p for p in procedures.values() if p.export}
    require(set(public) == expected and len(procedures) == len(expected),
            'context: expected exactly the eight approved callables and no extra implementation')
    for name in primitives | observers:
        p = public[name]
        require(not p.calls(), f'{p.name}: bundle must not call constructors or other services')
        require(not any(op in {'mkref', 'deref', 'setref'} for op, _ in p.instructions),
                f'{p.name}: bundle must copy existing reference payloads, not create owners or snapshots')

    factory = public['§factory']
    require(factory.result == CONTEXT_TYPE, 'context: wrong factory result type')
    require(factory.arguments == ','.join(f'{name}={expected_attributes[name + "_ref"]}'
                                        for name in ['activation', 'pool', 'config']),
            'context: factory parameters/defaults differ from approved contract')
    validated, writes, _ = reference_flow(factory, attributes)
    require({name for _, name in validated} == {'activation', 'pool', 'config'},
            'context: factory must validate each incoming reference')
    require({(name, source) for _, name, source in writes}
            == {(name + '_ref', name) for name in ['activation', 'pool', 'config']}
            and len(writes) == 3, 'context: factory must store each supplied reference payload exactly once')
    require(max(index for index, _ in validated) < min(index for index, _, _ in writes),
            'context: factory publishes bindings before all references are validated')
    flag_slot = attributes['zz_parse_observations'][1]
    constants, defaults = {}, []
    for index, (op, operands) in enumerate(factory.instructions):
        parts = operands.split(',')
        if op == 'load':
            constants[parts[0]] = parts[1]
        elif op == 'isetattr1' and int(parts[1]) == flag_slot:
            defaults.append((index, constants.get(parts[2], parts[2])))
    require(len(defaults) == 1 and defaults[0][1] == '0'
            and max(index for index, _ in validated) < defaults[0][0],
            'context: validated factory must initialize observations disabled')
    signal_returns(factory, 3)
    for name in ['activation', 'pool', 'config']:
        p = public[name]
        require(p.result == expected_attributes[name + '_ref'] and p.arguments == '',
                f'{p.name}: getter signature changed')
        validated, writes, returned = reference_flow(p, attributes)
        require([value for _, value in validated] == [name + '_ref'] and not writes,
                f'{p.name}: getter must validate only its selected reference without mutation')
        require(returned == [name + '_ref'], f'{p.name}: getter must return its reference payload')
        signal = signal_returns(p, 1)[0]
        empty = {args for op, args in p.instructions[:signal] if op == 'null'}
        require(p.instructions[signal + 1][1] in empty,
                f'{p.name}: skipped failure must return an empty reference, not an expired identity')
    p = public['bindpool']
    require(p.result == '.void' and p.arguments == 'pool=' + expected_attributes['pool_ref'],
            'context: bindPool signature changed')
    validated, writes, _ = reference_flow(p, attributes)
    require([name for _, name in validated] == ['pool']
            and [(name, source) for _, name, source in writes] == [('pool_ref', 'pool')],
            'context: bindPool must validate replacement and copy only its reference payload')
    require(validated[0][0] < writes[0][0], 'context: bindPool changes binding before validation')
    signal_returns(p, 1)

    for name, signature in {'enableparseobservations': ('.void', 'enabled=.boolean'),
                            'parseobservations': ('.boolean', '')}.items():
        p = public[name]
        require((p.result, p.arguments) == signature, f'{p.name}: observation signature changed')
        accesses = [(op, int(args.split(',')[2 if op == 'linkattr1' else 1]))
                    for op, args in p.instructions if op in {'linkattr1', 'isetattr1'}]
        expected_op = 'isetattr1' if name == 'enableparseobservations' else 'linkattr1'
        require(accesses == [(expected_op, flag_slot)], f'{p.name}: access only the observation flag')
        require(not any(op == 'linkref' for op, _ in p.instructions), f'{p.name}: must not borrow another owner')
    setter = public['enableparseobservations']
    receiver, enabled = (setter.variables[name][1] for name in ('§this', 'enabled'))
    require(('isetattr1', f'{receiver},{flag_slot},{enabled}') in setter.instructions,
            'context: observation setter stores the supplied Boolean')
    require(reference_flow(public['parseobservations'], attributes) == ([], [], ['zz_parse_observations']),
            'context: observation getter returns the stored flag without mutation')

    p = public['readsymbol']
    require((p.result, p.arguments) == ('.rexxvalue..rexxvalue', 'name=.string'),
            'context: readSymbol signature changed')
    borrowed = attribute_borrows(p, attributes)
    owner_registers = {p.variables[name][1] for name in ('current_pool', 'current_activation')}
    require(borrowed == {p.variables['current_pool'][1]: 'pool_ref',
                         p.variables['current_activation'][1]: 'activation_ref'},
            'readSymbol: borrow the live pool and activation bindings')
    require(not any(op in {'mkref', 'deref', 'setref'} or
                    (op == 'copy' and args.split(',')[0] in owner_registers)
                    for op, args in p.instructions), 'readSymbol: no owner construction or snapshots')
    targets = ['§rexxpool.rexxvariablepool.' + name
               for name in ('resolvesymbolname', 'resolvedhasvalue')]
    targets += ['§rexxclassicconditionevent.rexxclassicconditionevent.§factory',
                '§rexxpool.rexxvariablepool.resolvedvalue']
    policy_target = '§rexxactivation.rexxactivationarguments.signalpolicy'
    require({target for _, target in p.calls()} <= set(targets) | {policy_target},
            'readSymbol: delegate only to the shared pool, policy and condition event')
    positions = [p.call_index(target) for target in targets]
    require(positions == sorted(positions), 'readSymbol: resolve once before presence, event and value')
    policy = [index for index, target in p.calls() if target == policy_target]
    policy += [index for index, (op, args) in enumerate(p.instructions)
               if op == 'minlinkattr1' and args.split(',')[-1] == '6'
               and p.sources[index] is not None
               and Path(p.sources[index][0]).name == 'RexxActivationArguments.crexx'
               and 'signal_policy' in p.sources[index][2]]
    require(len(policy) == 1 and positions[0] < policy[0] < positions[1],
            'readSymbol: use the shared NOVALUE policy before testing presence')
    require(sum(op in {'brf', 'brt'} for op, _ in p.instructions[policy[0]:positions[2]]) >= 2,
            'readSymbol: condition event is gated by policy and absent final value')
    resolved = p.variables['resolved'][1]
    require(p.instructions[positions[0]][1].split(',')[0] == resolved
            and all(resolved in p.instructions[index][1].split(',') for index in positions[1:]),
            'readSymbol: presence, event and value share the once-resolved name')
    event = p.instructions[positions[2]][1].split(',')[0]
    require(('signal', f'"CLASSIC_CONDITION",{event}') in p.instructions,
            'readSymbol: relay the owned shared condition event')


def check_adapter(procedures):
    value, context = '.rexxvalue', 'reference ' + CONTEXT_TYPE
    state = 'reference .rexxdostate..rexxdostate'
    signatures = {
        'binaryop': ('.rexxvalue..rexxvalue', f'operation=.string,left={value},right={value},context={context}'),
        'unaryop': ('.rexxvalue..rexxvalue', f'operation=.string,value={value},context={context}'),
        'truth': ('.int', f'value={value},context={context}'),
        'whentruth': ('.int', f'value={value},context={context}'),
        'selectmissing': ('.void', 'line=.string'),
        'invalidtransfer': ('.void', 'code=.string,name=.string'),
        'donew': ('.rexxdostate..rexxdostate', f'context={context},name=.string'),
        'docapture': ('.void', f'state={state},part=.string,value=.rexxvalue..rexxvalue'),
        'dostart': ('.void', f'state={state}'),
        'doentry': ('.int', f'state={state}'),
        'doadvance': ('.void', f'state={state}'),
        'whiletruth': ('.int', f'value={value},context={context}'),
        'untiltruth': ('.int', f'value={value},context={context}'),
        'setting': ('.void', f'operation=.string,context={context},value={value}'),
        'digits': ('.rexxvalue..rexxvalue', f'context={context}'),
        'clausepending': ('.boolean', f'context={context}'),
        'parsesource': ('.rexxvalue..rexxvalue',
                        f'context={context},kind=.string,ordinal=.int,caller_source=.string,'
                        'source_state=reference .rexxclassicbifs..rexxbifcallcontext'),
        'countargs': ('.rexxvalue..rexxvalue', f'context={context}'),
        'existsarg': ('.rexxvalue..rexxvalue', f'context={context},ordinal={value},option={value}'),
        'lengthvalue': ('.rexxvalue..rexxvalue', f'value={value},context={context}'),
    }
    require({p.export for p in procedures.values() if p.export}
            == {'llctarget.' + name for name in signatures} | {
                'llctarget.' + name for name in ('literal', 'poolnew', 'poolread', 'poolwrite',
                    'sayvalue', 'sayconfigured', 'arguments', 'mainargs', 'append', 'present', 'absent', 'emptyreturn', 'bifarguments', 'missing', 'missingconfigured',
                    'requirevalue', 'setresult', 'status', 'failuremessage')},
            'adapter: expected exactly the clean context and packet/helper surface')
    helpers = {'literal', 'poolnew', 'poolread', 'poolwrite', 'sayvalue', 'sayconfigured',
               'arguments', 'mainargs', 'append', 'present', 'absent', 'emptyreturn', 'bifarguments', 'missing', 'missingconfigured',
               'requirevalue', 'setresult', 'status', 'failuremessage'}
    implementation = set(signatures) | helpers | {'bifcontext', 'lostdigitscheck'}
    require(set(procedures) - implementation in (set(), {'bad'})
            and implementation <= set(procedures),
            'adapter: unexpected implementation or executable main')
    lostdigits = procedures['lostdigitscheck']
    require(lostdigits.export is None and lostdigits.result == '.void'
            and lostdigits.arguments == f'value={value},context={context}',
            'lostdigitscheck: expected the private value/context condition helper')
    # Native B opt inlines this unchanged helper; noopt retains its body.
    if 'bad' in procedures:
        p = procedures['bad']
        require(p.export is None and p.result == '.void' and p.arguments == 'message=.string',
                'bad: expected only the private required-message error helper')
        require(any(op == 'signal' and args.startswith('"CLASSIC_SYNTAX",')
                    for op, args in p.instructions), 'bad: missing existing error transport')
        procedures['requirevalue'].call_index('bad')
    else:
        require(any(op == 'signal' and args.startswith('"CLASSIC_SYNTAX",')
                    for op, args in procedures['requirevalue'].instructions),
                'requirevalue: missing inlined existing error transport')
    for name, signature in signatures.items():
        p = procedures[name]
        require((p.result, p.arguments) == signature, f'{name}: approved signature/defaults changed')
    borrowed = {name: procedures[name] for name in [*signatures, 'bifcontext']
                if name not in {'selectmissing', 'invalidtransfer', 'parsesource'}}
    all_calls = [target for p in borrowed.values() for _, target in p.calls()]
    forbidden = {
        '§rexxactivation.rexxactivationarguments.§factory',
        '§rexxpool.rexxvariablepool.§factory',
        '§rexxclassicconfig.rexxclassicconfig.§factory', BIF + '.§factory',
        'rexxclassicbifs.rexxclassicbif_call',
    }
    require(not set(all_calls) & forbidden, 'adapter: owner/default-context construction or legacy BIF dispatch')
    require(all_calls.count(BIF + '.§factory.expression') == 1,
            'adapter: expected one shared expression-context construction site')
    helper = procedures['bifcontext']
    host_binding = '§rexxclassicconfig.rexxclassicconfig.bindhostcontext'
    binding = helper.call_index(host_binding)
    binding_operands = helper.instructions[binding][1].split(',')
    require(helper.instructions[binding][0] == 'settpcall' and len(binding_operands) == 5,
            'bifcontext: host binding must receive one explicit context value')
    bundle = helper.variables['bundle'][1]
    incoming = helper.variables['context'][1]
    borrows = [index for index, instruction in enumerate(helper.instructions)
               if instruction == ('linkref', f'{bundle},{incoming}')]
    actual = binding_operands[3]
    require(actual not in {register for _, register in helper.variables.values()},
            'bifcontext: host snapshot transport must not replace a named value or owner')
    transports = [index for index, instruction in enumerate(helper.instructions)
                  if instruction == ('copy', f'{actual},{bundle}')]
    require(len(borrows) == len(transports) == 1 and borrows[0] < transports[0] < binding,
            'bifcontext: copy only the incoming borrowed bundle into the host snapshot argument')
    transport = transports[0]
    require(not any(actual in operands.split(',')
                    for _, operands in helper.instructions[transport + 1:binding]),
            'bifcontext: host snapshot argument must reach its binding without other use or mutation')
    for p in borrowed.values():
        require(not any(op == 'deref' for op, _ in p.instructions), f'{p.name}: live-owner snapshot introduced')
        owner_registers = {register for kind, register in p.variables.values()
                           if kind in {CONTEXT_TYPE, '.rexxactivation..rexxactivationarguments',
                                       '.rexxpool..rexxvariablepool', '.rexxclassicconfig..rexxclassicconfig'}}
        require(not any(op == 'copy' and operands.split(',')[-1] in owner_registers
                        and not (p is helper and index == transport)
                        for index, (op, operands) in enumerate(p.instructions)),
                f'{p.name}: copied live owner value')
    for name in ['binaryop', 'unaryop', 'truth', 'whentruth', 'digits',
                 'donew', 'docapture', 'dostart', 'doentry', 'doadvance', 'whiletruth', 'untiltruth']:
        require(not any(op in {'numsci', 'numeng', 'setnumdgts', 'setnumfuz', 'setnumfrm'}
                        for op, _ in procedures[name].instructions), f'{name}: inherited numeric frame reset')
    require(helper.arguments == 'name=.string,context=' + context,
            'bifcontext: private helper must also borrow the authoritative bundle')
    ordered = [BIF + '.§factory.expression', '§' + CONTEXT + '.config', host_binding, BIF + '.setconfig',
               '§' + CONTEXT + '.pool', BIF + '.setcallerpool',
               '§' + CONTEXT + '.activation', BIF + '.setcalleractivation']
    positions = [helper.call_index(target) for target in ordered]
    require(positions == sorted(positions), 'bifcontext: bind config before pool, then activation')
    entries = {
        'binaryop': 'rexxclassicbifs.rexxclassic_expression_binary',
        'unaryop': 'rexxclassicbifs.rexxclassic_expression_unary',
        'truth': 'rexxclassicbifs.rexxclassic_expression_unary',
        'whentruth': 'rexxclassicbifs.rexxclassic_expression_unary',
        'whiletruth': 'rexxclassicbifs.rexxclassic_expression_unary',
        'untiltruth': 'rexxclassicbifs.rexxclassic_expression_unary',
        'digits': 'rexxclassicbifnumeric.rexxclassicbif_digits',
        'countargs': 'rexxclassicbifarg.rexxclassicbif_arg',
        'existsarg': 'rexxclassicbifarg.rexxclassicbif_arg',
        'lengthvalue': 'rexxclassicbiflength.rexxclassicbif_length',
    }
    for name, entry in entries.items():
        p = procedures[name]
        require(p.call_index('bifcontext') < p.call_index(entry) < p.call_index(CHECKED),
                f'{name}: expected direct existing implementation followed by checked delivery')
    for name in ('donew', 'docapture', 'dostart', 'doentry', 'doadvance'):
        p = procedures[name]
        captures = [(i, args.split(',')[1]) for i, (op, args) in enumerate(p.instructions)
                    if op == 'sigbrv' and args.endswith(',"ERROR"')]
        bridges = [(i, args.split(',')[-1]) for i, (op, args) in enumerate(p.instructions)
                   if op == 'signalorigin' and args.startswith('"CLASSIC_SYNTAX",')]
        require(len(captures) == len(bridges) == 1
                and captures[0][1] == bridges[0][1] and captures[0][0] < bridges[0][0],
                f'{name}: forward the captured shared ERROR event unchanged')
        require(any('rexxdostate.rexxdostate.' in target for _, target in p.calls()),
                f'{name}: delegate existing shared state implementation')
    for name, helper, filename in (
            ('selectmissing', 'rexxvalue.rexxvalue_select_missing', 'RexxValue.crexx'),
            ('invalidtransfer', 'rexxdostate.rexxdostate_invalid_transfer', 'RexxDoState.crexx')):
        missing = procedures[name]
        helper_sites = [index for index, target in missing.calls() if target == helper]
        inline_sites = [index for index, (op, operands) in enumerate(missing.instructions)
                        if op == 'signal' and operands.startswith('"ERROR",')
                        and missing.sources[index] is not None
                        and Path(missing.sources[index][0]).name == filename
                        and 'signal "ERROR"' in missing.sources[index][2]]
        require(len(helper_sites) + len(inline_sites) == 1,
                f'{name}: expected existing helper call or source-backed inline ERROR')
        captures = [(index, operands.split(',')[1])
                    for index, (op, operands) in enumerate(missing.instructions)
                    if op == 'sigbrv' and operands.endswith(',"ERROR"')]
        bridges = [(index, operands.split(',')[-1])
                   for index, (op, operands) in enumerate(missing.instructions)
                   if op == 'signalorigin' and operands.startswith('"CLASSIC_SYNTAX",')]
        require(len(captures) == len(bridges) == 1
                and captures[0][1] == bridges[0][1]
                and captures[0][0] < (helper_sites + inline_sites)[0] < bridges[0][0],
                f'{name}: bind one ERROR and forward that same event through signalorigin')
    binary = procedures['binaryop']
    transport = binary.call_index('bifcontext')
    for name in ['strictcompareequal', 'concat', 'spaceconcat']:
        direct = binary.call_index('§rexxvalue.rexxvalue.' + name)
        require(direct < transport and any(op == 'ret' for op, _ in binary.instructions[direct:transport]),
                f'binaryop: {name} must retain its direct return before BIF transport')
    for name in ['existsarg', 'lengthvalue']:
        require(procedures[name].call_index(BIF + '.setarguments') < procedures[name].call_index(entries[name]),
                f'{name}: operands/presence must reach the existing BIF context')
    source = procedures['parsesource']
    source_order = [BIF + '.§factory.expression', BIF + '.clearerror', BIF + '.setarguments',
                    BIF + '.setconfig', BIF + '.setcallerpool', BIF + '.setcalleractivation']
    positions = [source.call_index(target) for target in source_order]
    require(positions == sorted(positions), 'parsesource: one lean reset, clear, config before pool/activation')
    require(CHECKED not in {target for _, target in source.calls()},
            'parsesource: must retain caller-owned errors/NOTREADY for generated C checkpoint')
    require(not any('sourceinfo' in target for _, target in source.calls()),
            'parsesource: SOURCEINFO belongs to generated C caller')
    for target in ('§rexxactivation.rexxactivationarguments.argument',
                   '§rexxclassicconfig.rexxclassicconfig.pulltext',
                   '§rexxclassicconfig.rexxclassicconfig.sourcetext',
                   '§rexxclassicconfig.rexxclassicconfig.versiontext',
                   'rexxclassicbifstream.rexxclassicbif_linein'):
        source.call_index(target)
    state_register = source.variables['state'][1]
    fresh = [index for index, (op, operands) in enumerate(source.instructions)
             if op == 'mkref' and operands.split(',')[-1] == state_register]
    linein = source.call_index('rexxclassicbifstream.rexxclassicbif_linein')
    require(len(fresh) == 1 and positions[0] < fresh[0] < linein,
            'parsesource: renew reference from stable linked owner after destructive reset')
    p = procedures['setting']
    require(sum(op == 'linkref' for op, _ in p.instructions) >= 2,
            'setting: bundle and activation must be live links')
    for name in ['setnumericdigits', 'setnumericfuzz', 'setnumericform']:
        p.call_index('§rexxactivation.rexxactivationarguments.' + name)
    require(any(op == 'signal' and args.startswith('"CLASSIC_SYNTAX",') for op, args in p.instructions),
            'setting: existing validation error transport missing')
    require(not any(op in {'setnumdgts', 'setnumfuz', 'setnumfrm'} for op, _ in p.instructions),
            'setting: caller owns validated numeric-frame installation')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--context', type=Path, required=True, help='generated RexxClassicContext.rxas')
    parser.add_argument('--adapter', type=Path, required=True, help='generated clean llctarget.rxas')
    args = parser.parse_args()
    try:
        context, attributes = read_rxas(args.context)
        adapter, _ = read_rxas(args.adapter)
        check_context(context, attributes)
        check_adapter(adapter)
    except (OSError, ValueError, KeyError, IndexError) as error:
        print(f'TARGET_CONTEXT_SHAPE: {error}', file=sys.stderr)
        return 1
    print(f'TARGET_CONTEXT_SHAPE|checks={checks}|failures=0')
    return 0


if __name__ == '__main__':
    sys.exit(main())
