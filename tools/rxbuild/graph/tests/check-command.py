"""CTest smoke test for the direct COMMAND graph action."""

from __future__ import annotations

import argparse
import subprocess
import sys
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
    print("RXBUILD_COMMAND_TEST|PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
