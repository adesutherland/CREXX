"""CTest coverage for root graph execution policies."""

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


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--rxbuild", required=True, type=Path)
    parser.add_argument("--source", required=True, type=Path)
    args = parser.parse_args()

    root = args.source.resolve()
    graph = root / "tools" / "rxbuild" / "graph" / "policy" / "policy.graph.build"
    output = root / "tools" / "rxbuild" / "graph" / "policy" / "output.dat"
    output.unlink(missing_ok=True)
    prefix = [str(args.rxbuild.resolve()), str(graph)]

    rc, text = run(prefix, root)
    if (rc != 0 or output.exists() or "state=dry-run" not in text
            or "would build" not in text or "RXBUILD EXPLAIN:" not in text
            or "RXBUILD TOOLCHAIN:" not in text):
        print("RXBUILD_POLICY_TEST|FAIL|root dry-run policy was not applied")
        print(text)
        return 1

    rc, text = run(prefix + ["--no-dry-run"], root)
    if rc != 0 or not output.is_file():
        print("RXBUILD_POLICY_TEST|FAIL|explicit execution override failed")
        print(text)
        return 1

    output.unlink(missing_ok=True)
    print("RXBUILD_POLICY_TEST|PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
