"""CTest smoke test for the maintained composed graph example."""

from __future__ import annotations

import argparse
import subprocess
import sys
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


def fail(message: str, output: str) -> int:
    print("RXBUILD_COMPLEX_TEST|FAIL|" + message)
    print(output)
    return 1


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--rxbuild", required=True, type=Path)
    parser.add_argument("--source", required=True, type=Path)
    args = parser.parse_args()

    root = args.source.resolve()
    rxbuild = args.rxbuild.resolve()
    graph = root / "tools" / "rxbuild" / "graph" / "complex" / "complex.graph.build"
    toolchain = rxbuild.parent

    command = [
        str(rxbuild),
        str(graph),
        "--toolchain-dir",
        str(toolchain),
        "--verbose",
        "--explain",
    ]
    rc, output = run(command, root)
    if rc != 0:
        return fail("complex graph build failed", output)
    for expected in (
        "RXBUILD: built .crexx-build/graph-complex-frontend.rxbin",
        "RXBUILD: built .crexx-build/graph-complex-backend.rxbin",
        "RXBUILD: built .crexx-build/graph-complex-package.rxbin",
        "RXBUILD DEMO: generated program executed",
        "RXBUILD: ran .crexx-build/graph-complex-package.rxbin",
    ):
        if expected not in output:
            return fail("complex graph output was incomplete: " + expected, output)

    query = [
        str(rxbuild),
        str(graph),
        "--toolchain-dir",
        str(toolchain),
        "--show-graph",
        "package",
    ]
    rc, output = run(query, root)
    if rc != 0:
        return fail("complex graph query failed", output)
    for expected in ("frontend.reader", "backend.runtime", "build_demo.crexx"):
        if expected not in output:
            return fail("complex graph query was incomplete: " + expected, output)

    print("RXBUILD_COMPLEX_TEST|PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
