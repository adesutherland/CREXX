"""Compare the maintained PHP rxBuild plan with its graph-build equivalent."""

from __future__ import annotations

import argparse
import subprocess
from pathlib import Path


def run(command: list[str], root: Path) -> tuple[int, str]:
    completed = subprocess.run(
        command,
        cwd=root,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    return completed.returncode, completed.stdout


def output_lines(output: str) -> list[str]:
    return sorted(
        line.strip()
        for line in output.splitlines()
        if "expected=" in line and "result=" in line
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--crexx", required=True, type=Path)
    parser.add_argument("--source", required=True, type=Path)
    args = parser.parse_args()
    root = args.source.resolve()

    legacy_rc, legacy_output = run(
        [str(args.crexx.resolve()), "examples/php/setup.build"], root
    )
    graph_rc, graph_output = run(
        [
            str(args.crexx.resolve()),
            "examples/php/setup.graph.build",
            "run_php_migration",
        ],
        root,
    )

    if legacy_rc != 0:
        print("PHP_PLAN_PARITY|FAIL|setup.build rc=%d" % legacy_rc)
        print(legacy_output)
        return 1
    if graph_rc != 0:
        print("PHP_PLAN_PARITY|FAIL|setup.graph.build rc=%d" % graph_rc)
        print(graph_output)
        return 1

    legacy_lines = output_lines(legacy_output)
    graph_lines = output_lines(graph_output)
    if not legacy_lines:
        print("PHP_PLAN_PARITY|FAIL|setup.build produced no application output")
        print(legacy_output)
        return 1
    if legacy_lines != graph_lines:
        print("PHP_PLAN_PARITY|FAIL|application output differs")
        print("--- setup.build ---")
        print("\n".join(legacy_lines))
        print("--- setup.graph.build ---")
        print("\n".join(graph_lines))
        return 1

    print("PHP_PLAN_PARITY|PASS|%d application output lines" % len(legacy_lines))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
