#!/usr/bin/env python3
"""Check prepared-reader references in built SDK RXAS, without running tools.

The functional fixture supplies recognition/lifetime oracles. This check keeps
the large owner fields out of borrowed factories and follows each wrapper's
reference back to its receiver attribute. Registers and attribute IDs come from
the actual assembly/metadata rather than snapshots.
"""
from pathlib import Path
import re
import sys


def require(condition, message):
    if not condition:
        raise ValueError(message)


def routines(text):
    result = {}
    current = None
    for line in text.splitlines():
        match = re.match(r"^\S+\(.*\s\.expose=(\S+)\s*$", line)
        if match:
            current = match[1] if ".locals=" in line else None
            if current:
                result[current] = []
        elif current:
            result[current].append(line)
    return result


def instructions(body):
    result = []
    for line in body:
        match = re.match(r"\s+([a-z][a-z0-9]*)\s+(.+)$", line)
        if match:
            result.append((match[1], tuple(x.strip() for x in match[2].split(","))))
    return result


def callnames(body):
    return {arg.lstrip("§")[:-2] for op, args in instructions(body)
            if re.fullmatch(r"(?:settp)?(?:swap)?call[1-4]?", op)
            for arg in args if arg.endswith("()")}


def check(directory):
    texts = {name: (directory / f"{name}.rxas").read_text(encoding="utf-8")
             for name in ("lllanguage", "llmunch", "lllr", "llcfg", "llcheck")}
    code = {}
    for text in texts.values():
        code.update(routines(text))

    def body(name):
        require(name in code, f"missing built routine {name}")
        return code[name]

    def attribute(module, owner, field):
        match = re.search(rf'\.meta "{re.escape(owner)}\.{field}"="b" "[^"]+" \.attr (\d+)\b', texts[module])
        require(match, f"missing owner field metadata {owner}.{field}")
        return match[1]

    def inline_production_query(name, field):
        """Prove the scalar array read is linked through retained tables/CFG."""
        match = re.search(rf'\.meta "{re.escape(name)}\.tables"="b" "[^"]+" ([ra]\d+)\b',
                          "\n".join(body(name)))
        if not match:
            return False
        table_reg = match[1]
        cfg_id = attribute("lllr", "lllr.tables", "_cfg")
        array_id = attribute("llcfg", "llcfg.productions", field)
        ops = instructions(body(name))
        for i, (op, args) in enumerate(ops):
            if op != "linkattr1" or len(args) != 3 or args[1:] != (table_reg, cfg_id):
                continue
            cfg_reg = args[0]
            for j in range(i + 1, len(ops)):
                next_op, next_args = ops[j]
                if next_op in {"unlink", "unlinkn"} and cfg_reg in next_args:
                    break
                if next_op != "linkattr1" or len(next_args) != 3 or next_args[1:] != (cfg_reg, array_id):
                    continue
                array_reg = next_args[0]
                for read_op, read_args in ops[j + 1:]:
                    if read_op in {"unlink", "unlinkn"} and array_reg in read_args:
                        break
                    if read_op == "minlinkattr1" and len(read_args) == 4 and read_args[1] == array_reg:
                        return True
        return False

    # Packed LR load needs the checked production representation; nullable/FIRST
    # consumers retain one explicit, cached demand for complete graph facts.
    require("llcheck.analysis.structure" in callnames(body("llcfg.productions.build"))
            and "llcheck.analysis.run" not in callnames(body("llcfg.productions.build")),
            "production layout must reuse structural admission without full analysis")
    require("llcheck.analysis.structure" in callnames(body("llcheck.analysis.run")),
            "full analysis must share the owning structural validator")
    require("llcheck.analysis.run" in callnames(body("llcfg.productions.preparefacts")),
            "CFG fact demand must retain the complete analysis")
    ready_id = attribute("llcfg", "llcfg.productions", "_factsready")
    for query in ("nullable", "first"):
        name = "llcfg.productions." + query
        lines = body(name); ops = instructions(lines)
        demand = next((i for i, (_, args) in enumerate(ops)
                       if any(arg.lstrip("§") == "llcfg.productions.preparefacts()" for arg in args)), None)
        require(demand is not None, f"{query} must demand facts when cold")
        require(any(op == "linkattr1" and len(args) == 3 and args[1:] == ("a1", ready_id)
                    for op, args in ops[:demand])
                and any(op == "brf" for op, _ in ops[:demand]),
                f"{query} must guard fact preparation with its retained readiness field")
        require("llcheck.analysis.run" not in callnames(lines),
                f"{query} must not dispatch full analysis on every query")

    for module, cls, field, wrapper, factory in (
        ("lllanguage", "definition", "_dfa", "borrowedscanner", "llmunch.scanner.§factory.borrowed"),
        ("lllanguage", "definition", "_parser", "borrowedparser", "lllr.parser.§factory.borrowed"),
    ):
        owner = f"{module}.{cls}"
        match = re.search(rf'\.meta "{re.escape(owner)}\.{field}"="b" "[^"]+" \.attr (\d+)\b', texts[module])
        require(match, f"missing owner field metadata {owner}.{field}")
        ops = instructions(body(f"{owner}.{wrapper}"))
        targets = {args[0] for op, args in ops if op == "linkattr1" and len(args) == 3
                   and args[1] == "a1" and args[2] == match[1]}
        require(targets, f"{wrapper} must address its anchored receiver field")
        refs = {args[0] for op, args in ops if op == "mkref" and len(args) == 2 and args[1] in targets}
        require(refs, f"{wrapper} must reference the field directly")
        require(not any(op in {"copy", "bcopy"} and len(args) >= 2 and args[1] in targets
                        for op, args in ops), f"{wrapper} snapshots its table field")
        require(factory in callnames(body(f"{owner}.{wrapper}")), f"{wrapper} must call borrowed factory")

    for name in ("lllanguage.request.§factory.borrowed", "llmunch.scanner.§factory.borrowed",
                 "lllr.parser.§factory.borrowed"):
        text = "\n".join(body(name))
        require(re.search(r'"reference \.[^"]+" a1\b', text), f"{name} needs an explicit reference argument")
        require("LL_BORROW_OWNER:" in text, f"{name} lacks checked-owner refusal")
        forbidden = {"lllanguage.definition.automaton", "lllanguage.definition.tables",
                     "lllr.tables.grammar", "llcfg.productions.source"}
        require(not forbidden.intersection(callnames(body(name)))
                and not any(target.endswith(".§factory.loaded") for target in callnames(body(name))),
                f"{name} reloads or snapshots prepared data")

    for name in ("llmunch.scanner.run", "llmunch.scanner.prepare", "lllr.parser.run", "lllr.parser.accepts",
                 "lllanguage.request.lex", "lllanguage.request.classify_tokens", "lllanguage.request.parse_tokens",
                 "lllanguage.request.ast", "lllanguage.request.projection"):
        ops = instructions(body(name))
        require(any(op == "linkref" for op, _ in ops), f"{name} needs a scoped live alias")
        require("LL_BORROW_OWNER:" in "\n".join(body(name)), f"{name} lacks owner-expiry handling")

    for name in ("lllr.parser.run", "lllr.parser.accepts"):
        calls = callnames(body(name))
        require("lllr.tables.grammar" not in calls and "llcfg.productions.source" not in calls,
                f"{name} copies production/grammar metadata")
        # The seed may inline scalar queries. Require the actual retained
        # tables -> CFG -> scalar array link chain when the call disappears.
        require(all(f"lllr.tables.{query}" in calls or inline_production_query(name, field)
                    for query, field in (("productionlength", "_length"), ("productionlhs", "_lhs"))),
                f"{name} must query retained production metadata")
    for name in ("lllr.tables.productionlength", "lllr.tables.productionlhs", "lllr.tables.symbolkind",
                 "lllr.tables.symbollabel", "lllr.tables.terminalname", "llcfg.productions.symbolkind",
                 "llcfg.productions.terminalname"):
        require(not any(target.endswith((".grammar", ".source")) for target in callnames(body(name))),
                f"{name} copies its metadata owner")

    # Owning readers choose scoped aliases at operation time, never persistent
    # self-references which would expire after returning/copying the wrapper.
    for name in ("llmunch.scanner.§factory", "lllr.parser.§factory", "lllanguage.request.§factory"):
        require(not any(op == "mkref" for op, _ in instructions(body(name))),
                f"{name} stores a self-reference into an owning wrapper")


if __name__ == "__main__":
    try:
        require(len(sys.argv) == 2, "usage: check-borrowed-rxas.py <built lib/rxfnsl/rexx directory>")
        check(Path(sys.argv[1]))
    except (OSError, ValueError) as error:
        print(f"BORROWED_RXAS|FAIL|{error}")
        sys.exit(1)
    print("BORROWED_RXAS|PASS")
