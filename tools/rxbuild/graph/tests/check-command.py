"""CTest smoke test for the direct COMMAND graph action."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
import uuid
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--rxbuild", required=True, type=Path)
    parser.add_argument("--source", required=True, type=Path)
    args = parser.parse_args()

    root = args.source.resolve()
    graph = root / "tools" / "rxbuild" / "graph" / "command" / "command.graph.build"
    source = root / "tools" / "rxbuild" / "graph" / "command" / "source" / "input.dat"
    output = root / ".crexx-build" / "graph-command" / "output.dat"
    command = [str(args.rxbuild.resolve()), str(graph), "--verbose", "--explain"]
    completed = subprocess.run(
        command,
        cwd=root,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    if completed.returncode != 0:
        print("RXBUILD_COMMAND_TEST|FAIL|command action failed")
        print(completed.stdout)
        return 1
    if not output.is_file() or output.read_bytes() != source.read_bytes():
        print("RXBUILD_COMMAND_TEST|FAIL|declared output was not copied")
        print(completed.stdout)
        return 1
    if "RXBUILD: built .crexx-build/graph-command/output.dat" not in completed.stdout and "RXBUILD: up to date .crexx-build/graph-command/output.dat" not in completed.stdout:
        print("RXBUILD_COMMAND_TEST|FAIL|command action output was not reported")
        print(completed.stdout)
        return 1

    failure_root = root / ".crexx-build" / ("graph-command-failure-" + uuid.uuid4().hex)
    failure_root.mkdir(parents=True, exist_ok=False)
    input_root = failure_root / "inputs"
    input_root.mkdir()
    for name in ("first.dat", "second.dat"):
        (input_root / name).write_text(name + "\n", encoding="utf-8", newline="\n")
    failure_graph = failure_root / "failure.graph.build"
    failure_graph.write_text(
        f'''SOURCES = FILES IN "{input_root.as_posix()}"
          RECURSIVE
          EXTENSION ".dat";

RULE run_tool
  ACTION = "COMMAND"
END RULE;

BUILD_GROUP "failures"
  RULE = run_tool
  SOURCES = SOURCES
  OUTPUT_DIR = "{(failure_root / 'outputs').as_posix()}"
END BUILD_GROUP;

DEFAULT_BUILD = failures;
''',
        encoding="utf-8",
        newline="\n",
    )
    try:
        failure_command = [
            str(args.rxbuild.resolve()),
            str(failure_graph),
            "--toolchain-dir",
            str(args.rxbuild.resolve().parent),
            "--verbose",
            "--jobs",
            "2",
        ]
        failure = subprocess.run(
            failure_command,
            cwd=root,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        failure_output_text = failure.stdout
        if (
            failure.returncode != 4
            or "RXBUILD SCHEDULER: parallel wave" not in failure_output_text
            or "RXBUILD SCHEDULER: failure task=" not in failure_output_text
            or "target=" not in failure_output_text
            or "first.rxbin" not in failure_output_text
            or " rc=4" not in failure_output_text
        ):
            print("RXBUILD_COMMAND_TEST|FAIL|parallel child failure diagnostics were incomplete")
            print(failure_output_text)
            return 1
    finally:
        if failure_root.exists():
            shutil.rmtree(failure_root)

    print("RXBUILD_COMMAND_TEST|PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
