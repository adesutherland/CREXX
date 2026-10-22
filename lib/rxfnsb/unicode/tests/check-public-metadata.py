#!/usr/bin/env python3
"""Inspect built Unicode provider metadata using the owning RXBIN disassembler.

The runtime fixture separately exercises imported defaults/presence. This check
pins the binary callable inventory, types, exposed/optional formals and exports;
it does not parse RXBIN or compare executable/default/inline payload bytes.
"""
from pathlib import Path
import re
import subprocess
import sys


EXPECTED = {"version": (".string", "")}
for name in ("tonfd", "tonfc", "tonfkd", "tonfkc"):
    EXPECTED[name] = (".string", "expose text=.string")
for name in ("isnfd", "isnfc", "isnfkd", "isnfkc"):
    EXPECTED[name] = (".boolean", "expose text=.string")
for name in ("touppercase", "tolowercase", "tocasefold", "tosimplecasefold",
             "toturkiccasefold", "toturkicsimplecasefold", "graphemereverse"):
    EXPECTED[name] = (".string", "text=.string")
EXPECTED.update({
    "encode": (".binary", "text=.string,?encoding=.string,?replacement=.binary"),
    "decode": (".string", "data=.binary,?encoding=.string,?replacement=.string"),
    "isdecodable": (".boolean", "data=.binary,?encoding=.string"),
    "isencodingsupported": (".boolean", "encoding=.string"),
    "graphemecount": (".int", "text=.string"),
    "graphemesubstr": (".string", "text=.string,start=.int,?length=.int,?pad=.string"),
    "graphemepos": (".int", "needle=.string,haystack=.string,?start=.int"),
})
META = re.compile(r'^\s*\.meta "rxunicode\.([^." ]+)"="([^"]+)" "([^"]+)" (.*)$')
CALLABLE = re.compile(r'\S+\(\) "((?:\\.|[^"\\])*)"\s*$')
EXPORT = re.compile(r'\.expose=rxunicode\.([^.\s]+)\s*$')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def inventory(disassembler, image, expected_classes):
    functions = {}; classes = set(); exports = set(); last = ""; problems = []
    # Drain the stream without retaining executable code or large table dumps.
    with subprocess.Popen([str(disassembler), str(image)], stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, text=True, encoding="utf-8",
                          errors="replace") as process:
        for line in process.stdout:
            last = line[-240:].rstrip()
            match = EXPORT.search(line)
            if match:
                name = match[1].lower()
                if name in exports:
                    problems.append(f"duplicate export {name}")
                exports.add(name)
            match = META.match(line)
            if not match:
                continue
            name, flags, result, tail = match.groups(); name = name.lower()
            if tail.strip() == ".class":
                if name in classes:
                    problems.append(f"duplicate class {name}")
                classes.add(name)
                continue
            call = CALLABLE.fullmatch(tail)
            if not call:
                continue
            if name in functions:
                problems.append(f"duplicate callable {name}")
            if flags.lower() != "b":
                problems.append(f"public procedure {name} is not B-owned")
            functions[name] = (result.lower(), call[1].replace(r"\?", "?"))
        require(process.wait() == 0, f"{image.name}: disassembly failed: {last}")
    require(not problems, f"{image.name}: {'; '.join(problems)}")
    require(functions == EXPECTED,
            f"{image.name}: callable inventory/signatures differ: {functions}")
    require(exports == set(EXPECTED), f"{image.name}: procedural export inventory differs")
    require(classes == expected_classes, f"{image.name}: G class inventory differs: {classes}")
    return functions


def main():
    require(len(sys.argv) == 4,
            "usage: check-public-metadata.py RXDAS CORE_RXBIN G_RXBIN")
    disassembler, core, g = map(Path, sys.argv[1:])
    require(len(EXPECTED) == 23, "test oracle must contain 23 public procedures")
    require(inventory(disassembler, core, set()) ==
            inventory(disassembler, g, {"graphemes", "graphemeiterator"}),
            "B and G callable contracts differ")
    print("UNICODE_PUBLIC_METADATA|PASS: B23 and G23+2")


if __name__ == "__main__":
    try:
        main()
    except (OSError, UnicodeError, ValueError) as error:
        print(f"UNICODE_PUBLIC_METADATA|FAIL: {error}", file=sys.stderr)
        sys.exit(1)
