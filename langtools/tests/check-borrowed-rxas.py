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


def copies_receiver(ops):
    # Follow the operation-local MKREF/LINKREF receiver window as well as a1.
    # References themselves may be copied; copying the dereferenced owner may not.
    receivers = {"a1"}
    references = set()
    for op, args in ops:
        if op == "copy" and len(args) == 2 and args[1] in receivers:
            return True
        if op in {"unlink", "unlinkbr"} and args:
            receivers.discard(args[0]); references.discard(args[0])
        elif op == "mkref" and len(args) == 2:
            references.discard(args[0])
            if args[1] in receivers:
                references.add(args[0])
        elif op == "linkref" and len(args) == 2:
            receivers.discard(args[0])
            if args[1] in references:
                receivers.add(args[0])
    return False


def receiver_copy_controls():
    # A packed scalar bound reads an anchored field, not a value-copy of the
    # complete definition. This is the former inlined self-count regression.
    window = [("linkattr1", ("r2", "a1", "7")),
              ("load", ("r3", "8")), ("bgetu32", ("r4", "r2", "r3")),
              ("unlink", ("r2",))]
    require(not copies_receiver(window), "packed scalar bound control")
    bad = [("copy", ("r5", "a1")), *window]
    require(copies_receiver(bad), "packed scalar bound receiver-copy negative")
    alias = [("mkref", ("r8", "a1")), ("linkref", ("r9", "r8"))]
    require(not copies_receiver([*alias, ("copy", ("r10", "r8"))]),
            "receiver reference may be copied without copying its owner")
    require(copies_receiver([*alias, ("copy", ("r10", "r9"))]),
            "scoped receiver alias-copy negative")
    require(not copies_receiver([("mkref", ("r8", "a2")),
                                 ("linkref", ("r9", "r8")),
                                 ("copy", ("r10", "r9"))])
            and not copies_receiver([*alias, ("unlink", ("r9",)),
                                     ("copy", ("r10", "r9"))]),
            "foreign and released alias controls")


def copies_anchored_store(ops, field):
    """Follow only the module's actual anchored store/reference alias window."""
    stores, references = set(), set()
    for op, args in ops:
        if op == "copy" and len(args) == 2:
            if args[1] in stores:
                return True
            is_reference = args[1] in references
            stores.discard(args[0]); references.discard(args[0])
            if is_reference:
                references.add(args[0])
        elif op in {"unlink", "unlinkbr"} and args:
            stores.discard(args[0]); references.discard(args[0])
        elif op == "linkattr1" and len(args) == 3:
            stores.discard(args[0]); references.discard(args[0])
            if args[1:] == ("a1", field):
                stores.add(args[0])
        elif op == "mkref" and len(args) == 2:
            is_store = args[1] in stores
            stores.discard(args[0]); references.discard(args[0])
            if is_store:
                references.add(args[0])
        elif op == "linkref" and len(args) == 2:
            is_reference = args[1] in references
            stores.discard(args[0]); references.discard(args[0])
            if is_reference:
                stores.add(args[0])
    return False


def anchored_store_controls():
    direct = [("linkattr1", ("r2", "a1", "7"))]
    alias = [*direct, ("mkref", ("r3", "r2")), ("unlink", ("r2",)),
             ("linkref", ("r4", "r3"))]
    require(not copies_anchored_store(alias, "7"), "anchored store borrow control")
    require(copies_anchored_store([*direct, ("copy", ("r8", "r2"))], "7")
            and copies_anchored_store([*alias, ("copy", ("r8", "r4"))], "7"),
            "direct and dereferenced store-copy negatives")
    copied_reference = [*direct, ("mkref", ("r3", "r2")),
                        ("copy", ("r6", "r3")), ("linkref", ("r4", "r6"))]
    require(not copies_anchored_store(copied_reference, "7")
            and copies_anchored_store([*copied_reference, ("copy", ("r8", "r4"))], "7"),
            "reference copies are permitted but preserve store ownership tracking")
    require(not copies_anchored_store([*alias, ("unlink", ("r4",)),
                                       ("copy", ("r8", "r4"))], "7")
            and not copies_anchored_store([*direct, ("copy", ("r8", "r2"))], "8")
            and not copies_anchored_store([("linkattr1", ("r2", "a2", "7")),
                                          ("copy", ("r8", "r2"))], "7"),
            "released aliases and unrelated owner/field controls")


def flag_regions(body, field):
    """Fall-through bodies guarded by a scalar receiver field."""
    result = []
    for i, line in enumerate(body):
        match = re.fullmatch(r"\s+linkattr1\s+(r[0-9]+),a1," + re.escape(field), line)
        if not match:
            continue
        flag = match[1]
        for j in range(i + 1, len(body)):
            branch = re.fullmatch(r"\s+brf\s+(\S+)," + re.escape(flag), body[j])
            if branch:
                end = next((k for k in range(j + 1, len(body))
                            if body[k] == branch[1] + ":"), None)
                if end is not None:
                    result.append(body[j + 1:end])
                break
            if instructions([body[j]]):
                break
    return result


def sealed_read_safe(body):
    scans = {"llir.schema_data.arity", "llir.schema_data.fieldid",
             "llir.schema_data.fieldname", "llir.schema_data.fieldtype",
             "llir.schema_data.manifest"}
    return not copies_receiver(instructions(body)) and not scans.intersection(callnames(body))


def sealed_read_controls():
    body = ["   linkattr1 r2,a1,7", "   brf mutable,r2",
            "   bgetu32 r4,r3,r5", "   ret r4", "mutable:",
            "   copy r8,a1", "   call2 r9,§llir.schema_data.fieldid(),r8,r6"]
    regions = flag_regions(body, "7")
    require(len(regions) == 1 and sealed_read_safe(regions[0]),
            "sealed scalar control permits mutable fallback")
    for inserted in ("   copy r6,a1", "   call2 r9,§llir.schema_data.fieldid(),a1,r6"):
        bad = [*regions[0][:-1], inserted, regions[0][-1]]
        require(not sealed_read_safe(bad), "sealed read snapshot/scan negative")


def bucket_field_safe(body, index):
    ops = instructions(body)
    heads = {args[0] for op, args in ops if op == "linkattr1" and len(args) == 3
             and args[1:] == ("a1", index)}
    return (not copies_receiver(ops) and "llir.table.size" not in callnames(body)
            and any(op == "bgetu32" and len(args) == 3 and args[1] in heads
                    for op, args in ops)
            and any(op == "bsetu32" and len(args) == 3 and args[0] in heads
                    for op, args in ops))


def bucket_field_controls():
    body = ["   linkattr1 r2,a1,7", "   bgetu32 r3,r2,r4",
            "   call3 r5,§llir.table.get(),r6,r3,r8", "   bsetu32 r2,r4,r9"]
    require(bucket_field_safe(body, "7"), "anchored mutable bucket control")
    for inserted in ("   copy r6,a1", "   call1 r5,§llir.table.size(),r6"):
        require(not bucket_field_safe([inserted, *body], "7"),
                "mutable bucket receiver-copy/full-table-count negative")
    require(not bucket_field_safe(body, "8")
            and not bucket_field_safe(body[:-1], "7"),
            "mutable bucket foreign-field/missing-publication negative")


def linked_cfg_query(window, cfg, query):
    """Exact scalar call ABI after linking the retained tables._cfg field."""
    if len(window) != 7:
        return False
    (load, flags, bind, call, restore, unlink, argument) = window
    if load[0] != "load" or len(load[1]) != 2 or load[1][1] != "2":
        return False
    count = load[1][0]
    if not re.fullmatch(r"r[0-9]+", count):
        return False
    receiver = f"r{int(count[1:]) + 1}"
    if flags != ("settp", (cfg, "256")) or bind != ("swap", (receiver, cfg)):
        return False
    if call[0] != "swapcall" or len(call[1]) != 5:
        return False
    _, target, actual_count, production, saved = call[1]
    return (target.lstrip("§") == f"llcfg.productions.{query}()"
            and actual_count == count
            and saved == f"r{int(count[1:]) + 2}"
            and restore == ("swap", (cfg, receiver))
            and unlink == ("unlink", (cfg,))
            and argument == ("swap", (production, saved)))


def query_window_controls():
    window = [("load", ("r4", "2")), ("settp", ("r2", "256")),
              ("swap", ("r5", "r2")),
              ("swapcall", ("r0", "§llcfg.productions.length()", "r4", "r7", "r6")),
              ("swap", ("r2", "r5")), ("unlink", ("r2",)),
              ("swap", ("r7", "r6"))]
    require(linked_cfg_query(window, "r2", "length"), "linked CFG scalar control")
    for position, replacement in (
        (0, ("load", ("r4", "1"))),
        (1, ("settp", ("r2", "0"))),
        (2, ("swap", ("r5", "r3"))),
        (3, ("swapcall", ("r0", "§foreign.length()", "r4", "r7", "r6"))),
        (4, ("copy", ("r2", "r5"))),
        (5, ("unlink", ("r3",))),
        (6, ("swap", ("r7", "r9"))),
    ):
        bad = window.copy(); bad[position] = replacement
        require(not linked_cfg_query(bad, "r2", "length"),
                f"linked CFG scalar negative {position}")
    bad = window.copy()
    bad[3] = ("swapcall", ("r0", "§llcfg.productions.length()", "r4", "r7", "r9"))
    bad[6] = ("swap", ("r7", "r9"))
    require(not linked_cfg_query(bad, "r2", "length"),
            "linked CFG scalar negative argument window")


def check(directory):
    query_window_controls()
    receiver_copy_controls()
    anchored_store_controls()
    sealed_read_controls()
    bucket_field_controls()
    texts = {name: (directory / f"{name}.rxas").read_text(encoding="utf-8")
             for name in ("lllanguage", "llmunch", "lllr", "llcfg", "llcheck",
                          "llcore", "llcomments", "llstage", "llproject", "llast", "llir")}
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

    require(bucket_field_safe(body("llir.schema_data.field"),
                              attribute("llir", "llir.schema_data", "_fieldindex")),
            "mutable field admission must use anchored bucket heads without owner copies or a full-table bound")
    for method in ("addfamily", "addtype", "addop", "effect", "service"):
        name = "llir.schema_data." + method
        require(not copies_receiver(instructions(body(name))),
                f"{name} copies its schema receiver through a scoped alias")
    store = attribute("llir", "llir.module", "_store")
    for method in ("error", "identity", "operations", "operation"):
        name = "llir.module." + method
        ops = instructions(body(name))
        require(any(op == "linkattr1" and len(args) == 3
                    and args[1:] == ("a1", store) for op, args in ops),
                f"{name} must borrow its anchored store")
        require(not copies_receiver(ops) and not copies_anchored_store(ops, store),
                f"{name} copies its module or anchored arena")
    indexed = attribute("llir", "llir.schema_data", "_indexready")
    for query in ("arity", "fieldid"):
        name = "llir.schema_data." + query
        regions = flag_regions(body(name), indexed)
        require(len(regions) == 1 and sealed_read_safe(regions[0])
                and not callnames(regions[0])
                and any(op == "bgetu32" for op, _ in instructions(regions[0])),
                f"{name} must read its sealed index without snapshots or scans")
    for query in ("fieldname", "fieldtype"):
        name = "llir.schema_data." + query
        require(not copies_receiver(instructions(body(name))),
                f"{name} snapshots its schema for a field read")
    name = "llir.schema_data.manifest"
    cached = flag_regions(body(name), attribute("llir", "llir.schema_data", "_manifestready"))
    require(len(cached) == 1 and sealed_read_safe(cached[0])
            and not callnames(cached[0])
            and any(op == "ret" for op, _ in instructions(cached[0])),
            "sealed manifest must return its complete cache without rebuilding")
    rendered = flag_regions(body(name), indexed)
    require(len(rendered) >= 2 and all(sealed_read_safe(region) for region in rendered),
            "sealed manifest rendering must read global indexed fields without receiver snapshots")

    for method in ("tokenname", "tokenid", "stageat", "messagetext", "messagemask", "explain"):
        name = "lllanguage.definition." + method
        ops = instructions(body(name))
        require(not copies_receiver(ops), f"{name} copies its complete metadata owner")
        require(any(op == "bgetu32" for op, _ in ops),
                f"{name} must read its packed scalar bound directly")
        require(not {"lllanguage.definition.tokencount", "lllanguage.definition.stagecount",
                     "lllanguage.definition.messagecount"}.intersection(callnames(body(name))),
                f"{name} dispatches a self-count query")

    def inline_production_query(name, field):
        """Prove a scalar call/read is linked through retained tables/CFG."""
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
            query = {"_length": "length", "_lhs": "lhs"}[field]
            if linked_cfg_query(ops[i + 1:i + 8], cfg_reg, query):
                return True
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

    # Trusted generated factories may copy fixed sections/header scalars only.
    # Checked external loaders keep the semantic validators below.
    forbidden = ("llcheck.analysis.", "llprepared.reader.", "llprepared.writer.",
                 "llprepared.budget.", "lllr.imagecheck.")
    for name in ("lllanguage.compiled", "lllanguage.definition.§factory.compiled",
                 "llcore.grammar.§factory.compiled", "llcfg.productions.§factory.compiled",
                 "lllr.tables.§factory.compiled", "llmunch.automaton.§factory.compiled",
                 "llcomments.rules.§factory.compiled", "llstage.rules.§factory.compiled",
                 "llproject.schema.§factory.compiled", "llast.schema.§factory.compiled"):
        calls = callnames(body(name))
        require(not any(target.startswith(forbidden)
                        or target.endswith((".image", ".frozenimage", ".prepared", ".loaded", ".build", ".preparefacts"))
                        for target in calls), f"{name} hydrates or validates compiled data")
        require(not any(op == "minlinkattr1" for op, _ in instructions(body(name))),
                f"{name} constructs row arrays")

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
        # The seed may inline the outer query or the entire scalar accessor.
        # Both alternatives must follow the retained tables -> CFG link.
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
