#!/usr/bin/env python3
"""Synthetic relay qualification against an instrumented temporary exact source.

This does not cause real allocation, division or Unicode failures. It verifies
all nine unchanged production handlers with original bound provenance, an actual
constructed/closed cursor and caller skip. Source/images disappear on exit.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import subprocess
import tempfile
import time

NAMES = (
    "CLASSIC_SYNTAX", "DIVISION_BY_ZERO", "CONVERSION_ERROR",
    "INVALID_ARGUMENTS", "OBJECT_NOT_INITIALIZED", "REFERENCE_INVALID",
    "UNICODE_ERROR", "OUT_OF_RANGE", "FAILURE",
)


def replace_once(source: str, old: str, new: str) -> str:
    if source.count(old) != 1:
        raise ValueError(f"instrumentation seam changed: {old!r}")
    return source.replace(old, new, 1)


def instrument(source: str) -> str:
    handlers = source[source.index("  on signal classic_syntax as problem"):
                      source.index("  end\n  call worker.close()")]
    source = replace_once(source, "namespace rexxclassicparse expose execute",
                          "namespace rexxclassicparseinjected expose execute")
    source = replace_once(source, "  _packet = .binary",
                          "  _test_pool = reference .rexxpool..RexxVariablePool\n  _packet = .binary")
    source = replace_once(source, "    bundle = dereference context\n    config_ref = bundle.config()",
                          "    bundle = dereference context\n    _test_pool = bundle.pool()\n    config_ref = bundle.config()")
    source = replace_once(source, "      call _cursor.close()\n      _live = 0",
                          "      call _cursor.close()\n      test_pool = dereference _test_pool\n"
                          "      test_count = test_pool.symbolValue('_TEST_CLOSED').asString(1) as .int\n"
                          "      test_count = test_count + 1\n"
                          "      call test_pool.setString('_TEST_CLOSED',test_count as .string)\n      _live = 0")
    source = replace_once(source, "      field_start = scan; field_length = text_length - scan",
                          "      test_pool = dereference _test_pool\n"
                          "      if target = first + 1 & test_pool.symbolValue('_TEST_PHASE').asString(1) = 'SCAN' then call testRaise()\n"
                          "      field_start = scan; field_length = text_length - scan")
    source = replace_once(source, "        call current_pool.setSymbolValue(_target_name[target],value)",
                          "        call current_pool.setSymbolValue(_target_name[target],value)\n"
                          "        if target = first & current_pool.symbolValue('_TEST_PHASE').asString(1) = 'STORE' then call testRaise()")
    source += "\n  -- TEST ONLY: real bound events injected at asserted ownership checkpoints.\n"
    source += "  testRaise: method = .void\n    pool = dereference _test_pool\n"
    source += "    name = pool.symbolValue('_TEST_NAME').asString(1)\n"
    source += "    message = 'RXC-LC-26.4: injected ' || name\n    select\n"
    for name in NAMES:
        source += f"      when name = '{name}' then do\n        do\n"
        source += f'          assembler signal "{name}",message\n'
        source += f"        on signal {name.lower()} as original\n"
        source += "          call pool.setString('_TEST_RAW_MODULE',original.module() as .string)\n"
        source += "          call pool.setString('_TEST_RAW_ADDRESS',original.address() as .string)\n"
        source += f'          assembler signalorigin "{name}",message,original\n'
        source += "          return\n        end\n      end\n"
    source += "      otherwise nop\n    end\n    return\n"
    if handlers not in source:
        raise ValueError("production relay handlers were changed by instrumentation")
    return source


def consumer(builder: str) -> str:
    installs = "\n".join(f'  assembler sigcalla caught(),"{name}"' for name in NAMES)
    return f'''/* TEST ONLY: synthetic errors, unchanged production relay handlers. */
options levelb comments_dash numeric_classic
import rxfnsb
import rexxclassicparseinjected
import rexxclassiccontext
import rexxclassicconfig
import rexxclassicbifs
import rexxactivation
import rexxpool
import rexxvalue

main: procedure = .int expose checks failures handled expected phase active_context
  checks = 0; failures = 0; handled = 0
  activation = .rexxactivation..RexxActivationArguments()
  pool = .rexxpool..RexxVariablePool(1)
  config = .rexxclassicconfig..RexxClassicConfig('UTF8')
  context = .rexxclassiccontext..RexxClassicContext(reference activation,reference pool,reference config)
  active_context = reference context
{installs}
  index = .int; stage = .int; expected = .string; phase = .string; p = .Program
  do stage = 1 to 2
    phase = word('STORE SCAN',stage)
    do index = 1 to 9
      expected = word('{' '.join(NAMES)}',index)
      call pool.setString('_TEST_NAME',expected); call pool.setString('_TEST_PHASE',phase)
      call pool.setString('_TEST_CLOSED','0')
      call pool.setString('EARLY','untouched'); call pool.setString('LATER','untouched')
      p = .Program(); call p.section('EARLY LATER')
      before = handled
      call rexxclassicparseinjected..execute(reference context,.RexxValue('one two'),p.make(0))
      call check(handled = before + 1,'one skipped relay then immediate caller return')
      call check(pool.symbolValue('_TEST_CLOSED').asString(1) == '1','constructed cursor closed before return')
      call check(pool.symbolValue('EARLY').asString(1) == 'one','earlier acknowledged write survives')
      call check(pool.symbolValue('LATER').asString(1) == 'untouched','skip does not resume later write')
      call check(pool.symbolValue('RC').asString(1) == '26' & pool.symbolValue('.MN').asString(1) == '26.4','existing recorder preserves RC and message number')
    end
  end
  call pool.setString('_TEST_PHASE',''); call pool.setString('_TEST_CLOSED','0')
  p = .Program(); call p.section('RECOVERED')
  call rexxclassicparseinjected..execute(reference context,.RexxValue('recovered'),p.make(0))
  call check(pool.symbolValue('RECOVERED').asString(1) == 'recovered' & pool.symbolValue('_TEST_CLOSED').asString(1) == '1','normal execution recovers and closes')
  say 'CLASSIC_PARSE_RELAYS|checks=' || checks || '|failures=' || failures
  return failures

caught: procedure = .string expose checks failures handled expected phase active_context
  arg raw = .runtime_signal_raw
  condition = .runtime_signal(''); call condition.set_raw(raw)
  bundle = dereference active_context; pool = dereference bundle.pool()
  handled = handled + 1
  call check(condition.name() == expected,'named identity')
  target = 1
  if phase = 'SCAN' then target = 2
  suffix = ' [PARSE direct-parse.rexx:17:' || target || ' template=1 item=1 action=target#' || target || ']'
  call check(condition.message() == 'RXC-LC-26.4: injected ' || expected || suffix,'exact original prefix and current target suffix')
  call check(condition.module() as .string == pool.symbolValue('_TEST_RAW_MODULE').asString(1) &,
    condition.address() as .string == pool.symbolValue('_TEST_RAW_ADDRESS').asString(1),'original module and address')
  call check(pos('rexxclassicparseinjected.crexx',condition.file()) > 0,'raw injection source retained')
  call rexxclassic_signal_record(bundle.pool(),bundle.activation(),condition)
  return '__rxsignal_skip'

check: procedure = .void expose checks failures
  arg passed = .int, label = .string
  checks = checks + 1
  if passed then return
  failures = failures + 1; say 'FAIL|' || label
  return

{builder}'''


def run(command: list[str], cwd: Path) -> None:
    completed = subprocess.run(command, cwd=cwd, text=True, capture_output=True, timeout=600)
    if completed.stdout:
        print(completed.stdout, end="", flush=True)
    if completed.stderr:
        print(completed.stderr, end="", flush=True)
    if completed.returncode:
        raise RuntimeError(f"exit {completed.returncode}: {' '.join(command)}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--bin-dir", type=Path, required=True)
    parser.add_argument("--mode", choices=("opt", "noopt"), required=True)
    parser.add_argument("--vm", choices=("rxbvm", "rxtvm"), action="append")
    args = parser.parse_args()
    root = args.source_root.resolve(); binaries = args.bin_dir.resolve()
    test_dir = root / "lib/rxfnsc/tests_functional"
    source = (root / "lib/rxfnsc/RexxClassicParse.crexx").read_text(encoding="utf-8")
    fixture = (test_dir / "testRexxClassicParse.crexx").read_text(encoding="utf-8")
    if fixture.count("\nProgram: class\n") != 1:
        raise ValueError("independent packet fixture builder changed")
    builder = "Program: class\n" + fixture.split("\nProgram: class\n", 1)[1]
    started = time.monotonic()
    with tempfile.TemporaryDirectory(prefix="crexx-parse-relays-") as temporary:
        scratch = Path(temporary)
        module = scratch / "rexxclassicparseinjected"
        consumer_path = scratch / "relayconsumer"
        module.with_suffix(".crexx").write_text(instrument(source), encoding="utf-8")
        consumer_path.with_suffix(".crexx").write_text(consumer(builder), encoding="utf-8")
        options = ["-n"] if args.mode == "noopt" else []
        for path in (module, consumer_path):
            run([str(binaries / "rxc"), "--no-exe-import", *options,
                 "-i", str(scratch), "-i", str(binaries), "-o", str(path),
                 str(path.with_suffix(".crexx"))], root)
            run([str(binaries / "rxas"), *options, "-o", str(path), str(path)], root)
        image = scratch / "relay-linked"
        run([str(binaries / "rxlink"), "-o", str(image), str(consumer_path), str(module),
             *(str(binaries / name) for name in ("library", "classlib", "rxfnsc", "rxunicode-core"))], root)
        for vm in args.vm or ["rxbvm", "rxtvm"]:
            print(f"CLASSIC_PARSE_RELAYS|mode={args.mode}|vm={vm}", flush=True)
            run([str(binaries / vm), "--no-autoload", str(image)], root)
    print(f"CLASSIC_PARSE_RELAYS|mode={args.mode}|seconds={time.monotonic() - started:.2f}", flush=True)


if __name__ == "__main__":
    main()
