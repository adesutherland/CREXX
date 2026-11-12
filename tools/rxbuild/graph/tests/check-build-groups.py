"""CTest smoke test for source-discovered graph build groups."""

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
    print("RXBUILD_GROUP_TEST|FAIL|" + message)
    print(output)
    return 1


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--rxbuild", required=True, type=Path)
    parser.add_argument("--source", required=True, type=Path)
    args = parser.parse_args()

    root = args.source.resolve()
    rxbuild = args.rxbuild.resolve()
    graph = root / "tools" / "rxbuild" / "graph" / "groups" / "groups.graph.build"
    toolchain = rxbuild.parent
    prefix = [str(rxbuild), str(graph), "--toolchain-dir", str(toolchain)]

    rc, output = run(prefix + ["--list-targets"], root)
    if rc != 0:
        return fail("group target listing failed", output)
    for expected in ("graph-groups\\one.rxbin", "graph-groups\\sub\\two.rxbin", "@group/::programs"):
        if expected not in output:
            return fail("group target listing was incomplete: " + expected, output)

    rc, output = run(prefix + ["--show-graph", "programs"], root)
    if rc != 0:
        return fail("group graph query failed", output)
    for expected in ("sources\\one.crexx", "sources\\sub\\two.crexx"):
        if expected not in output:
            return fail("group graph query was incomplete: " + expected, output)

    rc, output = run(prefix + ["--dry-run", "programs"], root)
    if rc != 0:
        return fail("group dry-run failed", output)

    rc, output = run(prefix + ["--verbose", "programs"], root)
    if rc != 0 or "RXBUILD SCHEDULER: workers=2" not in output:
        return fail("root JOBS setting was not applied", output)

    rc, output = run(prefix + ["--verbose", "--jobs", "4", "programs"], root)
    if (rc != 0
            or "RXBUILD SCHEDULER: parallel wave" not in output
            or "RXBUILD SCHEDULER: launch task=[01]" not in output
            or "[01] RXBUILD SCHEDULER: finish task=1" not in output
            or "RXBUILD SCHEDULER: child task=1" not in output):
        return fail("parallel group scheduling was not exercised", output)

    rc, output = run(prefix + ["--show-dependencies", "programs"], root)
    if rc != 0 or "graph-groups-rxpp" not in output:
        return fail("group dependency ordering was missing", output)

    rc, output = run(prefix + ["--list-targets"], root)
    if rc != 0:
        return fail("RXPP group target listing failed", output)
    for expected in ("graph-groups-rxpp\\one.rxbin", "graph-groups-rxpp\\sub\\two.rxbin"):
        if expected not in output:
            return fail("RXPP group target listing was incomplete: " + expected, output)

    rc, output = run(prefix + ["--verbose", "preprocessed"], root)
    if rc != 0:
        return fail("RXPP group build failed", output)
    for target in (".crexx-build/graph-groups-rxpp/one.rxbin", ".crexx-build/graph-groups-rxpp/sub/two.rxbin"):
        if ("RXBUILD: built " + target not in output
                and "RXBUILD: up to date " + target not in output):
            return fail("RXPP group build output was incomplete: " + target, output)
    for generated in (".crexx-build/graph-groups-rxpp/one.crexx", ".crexx-build/graph-groups-rxpp/sub/two.crexx"):
        if not (root / generated).is_file():
            return fail("RXPP intermediate source was not retained: " + generated, output)

    print("RXBUILD_GROUP_TEST|PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
