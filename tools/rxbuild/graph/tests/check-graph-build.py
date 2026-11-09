"""Focused tests for the canonical rxbuild graph implementation."""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
import uuid
from pathlib import Path


def run(exe: Path, root: Path, plan: Path, *target: str) -> tuple[int, str]:
    command = [str(exe), str(plan), *target]
    if "--toolchain-dir" not in target:
        command.extend(["--toolchain-dir", str(exe.parent)])
    completed = subprocess.run(
        command,
        cwd=root,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    return completed.returncode, completed.stdout


def require(condition: bool, message: str, output: str = "") -> None:
    if not condition:
        raise AssertionError(message + ("\n" + output if output else ""))


def write_plan(path: Path, body: str) -> None:
    path.write_text(body, encoding="utf-8", newline="\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--rxbuild", required=True, type=Path)
    parser.add_argument("--source", required=True, type=Path)
    args = parser.parse_args()
    root = args.source.resolve()
    exe = args.rxbuild.resolve()
    toolchain_dir = exe.parent
    test_root = root / ".crexx-build" / ("graph-test-" + uuid.uuid4().hex)
    test_root.mkdir(parents=True, exist_ok=False)
    plan = test_root / "graph.build"
    try:
        source = "tools/rxbuild/build_demo.crexx"
        output_dir = ".crexx-build/" + test_root.name
        write_plan(
            plan,
            f'''BUILD_DIR = "{output_dir}";
SOURCE = "{source}";
RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "${{BUILD_DIR}}/prepare.rxbin"
  RULE = compile_program
  INPUT = "${{SOURCE}}"
END BUILD;
BUILD "${{BUILD_DIR}}/final.rxbin"
  RULE = compile_program
  INPUT = "${{SOURCE}}"
  DEPENDS_ON = "${{BUILD_DIR}}/prepare.rxbin"
END BUILD;
DEFAULT "${{BUILD_DIR}}/final.rxbin";
''',
        )

        rc, output = run(exe, root, plan, "--toolchain-dir", str(toolchain_dir), "--verbose")
        require(rc == 0, "default graph build failed", output)
        require(
            output.count("built") == 2
            and "RXBUILD TOOLCHAIN: root=" in output,
            "default graph did not use the explicit toolchain",
            output,
        )

        rc, output = run(exe, root, plan, "--verbose", output_dir + "/final.rxbin")
        require(
            rc == 0
            and "RXBUILD VERBOSE: action=COMPILE PROGRAM" in output
            and "input=" in output
            and "output=" in output
            and "depends=" in output
            and "up to date" in output,
            "verbose graph diagnostics were incomplete",
            output,
        )

        rc, output = run(exe, root, plan, "-keep", "-verbose3", output_dir + "/final.rxbin")
        require(
            rc == 0 and "RXBUILD VERBOSE: action=COMPILE PROGRAM" in output,
            "crexx compatibility options were not normalized",
            output,
        )

        rc, output = run(exe, root, plan, "--explain")
        require(rc == 0, "incremental graph build failed", output)
        require(
            output.count("RXBUILD: up to date") == 2
            and output.count("reason=up to date") == 2,
            "incremental graph was not a no-op or explanation was missing",
            output,
        )

        content_source = test_root / "content-source.crexx"
        content_source.write_text("options levelb\nmain: procedure = .int\n  say 'content-one'\n  return 0\n", encoding="utf-8", newline="\n")
        content_plan = test_root / "content-state.graph.build"
        content_output = output_dir + "/content-state.rxbin"
        write_plan(
            content_plan,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{content_output}"
  RULE = compile_program
  INPUT = "{content_source.as_posix()}"
END BUILD;
DEFAULT "{content_output}";
''',
        )
        rc, output = run(exe, root, content_plan)
        require(rc == 0 and "built" in output, "content-state initial build failed", output)
        rc, output = run(exe, root, content_plan, "--explain")
        require(rc == 0 and "reason=up to date" in output, "content-state no-op failed", output)
        marker = root / (content_output + ".rxbuild-state.in-progress")
        marker.parent.mkdir(parents=True, exist_ok=True)
        marker.write_text("RXBUILD IN PROGRESS", encoding="utf-8")
        rc, output = run(exe, root, content_plan, "--explain")
        require(
            rc == 0
            and "reason=interrupted previous action" in output
            and "built" in output
            and not marker.exists(),
            "interrupted action marker was not recovered",
            output,
        )
        original_mtime = content_source.stat().st_mtime_ns
        content_source.write_text("options levelb\nmain: procedure = .int\n  say 'content-two'\n  return 0\n", encoding="utf-8", newline="\n")
        os.utime(content_source, ns=(original_mtime, original_mtime))
        rc, output = run(exe, root, content_plan, "--explain")
        require(
            rc == 0
            and "reason=content or action changed" in output
            and "built" in output,
            "content change with preserved mtime was not detected",
            output,
        )
        rc, output = run(exe, root, content_plan, "--env", "RXBUILD_PHASE93=one", "--explain")
        require(
            rc == 0 and "reason=content or action changed" in output and "built" in output,
            "environment change was not included in target state",
            output,
        )
        rc, output = run(exe, root, content_plan, "--env", "RXBUILD_PHASE93=two", "--explain")
        require(
            rc == 0 and "reason=content or action changed" in output and "built" in output,
            "environment value change was not included in target state",
            output,
        )

        final_output = root / output_dir / "final.rxbin"
        final_output.unlink()
        rc, output = run(exe, root, plan, "--explain", output_dir + "/final.rxbin")
        require(rc == 0, "explicit target rebuild failed", output)
        require(
            "up to date " + output_dir + "/prepare.rxbin" in output
            and "reason=output missing" in output
            and "built " + output_dir + "/final.rxbin" in output,
            "explicit target was not rebuilt or explanation was missing",
            output,
        )

        logical = test_root / "logical.graph.build"
        logical_output_dir = output_dir + "/logical"
        write_plan(
            logical,
            f'''BUILD_DIR = "{logical_output_dir}";
SOURCE = "{source}";
RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "${{BUILD_DIR}}/prepare.rxbin"
  BUILD_NAME = prepare
  RULE = compile_program
  INPUT = "${{SOURCE}}"
END BUILD;
BUILD "${{BUILD_DIR}}/final.rxbin"
  BUILD_NAME = final_program
  RULE = compile_program
  INPUT = "${{SOURCE}}"
  DEPENDS_ON_BUILD = prepare
END BUILD;
DEFAULT_BUILD = final_program;
''',
        )
        rc, output = run(exe, root, logical)
        require(rc == 0 and output.count("built") == 2, "logical graph build failed", output)
        rc, output = run(exe, root, logical, "prepare")
        require(rc == 0 and "up to date" in output, "logical target selection failed", output)

        include_dir = test_root / "nested"
        include_dir.mkdir()
        included = include_dir / "included.graph.build"
        included_output = output_dir + "/included"
        write_plan(
            included,
            f'''BUILD_DIR = "{included_output}";
SOURCE = "{source}";
RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "${{BUILD_DIR}}/child.rxbin"
  BUILD_NAME = child
  RULE = compile_program
  INPUT = "${{SOURCE}}"
END BUILD;
DEFAULT_BUILD = child;
''',
        )
        composed = test_root / "composed.graph.build"
        write_plan(
            composed,
            f'''INCLUDE_GRAPH "nested/included.graph.build"
RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/parent.rxbin"
  BUILD_NAME = parent
  RULE = compile_program
  INPUT = "{source}"
  DEPENDS_ON_BUILD = child
END BUILD;
DEFAULT_BUILD = parent;
''',
        )
        rc, output = run(exe, root, composed)
        require(rc == 0 and output.count("built") == 2, "composed graph build failed", output)
        rc, output = run(exe, root, composed, "child")
        require(rc == 0 and "up to date" in output, "included logical target selection failed", output)
        rc, output = run(exe, root, composed, "--dump-graph")
        require(
            rc == 0
            and "RXBUILD GRAPH BEGIN" in output
            and "source_graph=" in output
            and "name=child" in output
            and "name=parent" in output
            and "depends_build=child -> output=" in output
            and "RXBUILD GRAPH END" in output
            and "RXBUILD: built" not in output
            and "RXBUILD: ran" not in output,
            "resolved graph dump was incomplete or executed an action",
            output,
        )
        rc, json_output = run(exe, root, composed, "--dump-graph=json")
        try:
            graph_json = json.loads(json_output)
        except json.JSONDecodeError as error:
            raise AssertionError("JSON graph dump was invalid: " + str(error) + "\n" + json_output) from error
        require(
            rc == 0
            and isinstance(graph_json.get("targets"), list)
            and any(target.get("name") == "child" for target in graph_json["targets"])
            and any(target.get("name") == "parent" for target in graph_json["targets"]),
            "JSON graph dump was incomplete",
            json_output,
        )
        rc, json_output = run(
            exe,
            root,
            composed,
            "--dump-graph=json",
            "--env",
            "RXBUILD_DIAGNOSTIC=hidden",
            "--unset-env",
            "RXBUILD_DIAGNOSTIC_UNUSED",
        )
        diagnostic_json = json.loads(json_output)
        require(
            rc == 0
            and "toolchain_dir" in diagnostic_json
            and diagnostic_json["environment"]["overrides"] == ["RXBUILD_DIAGNOSTIC"]
            and diagnostic_json["environment"]["unset"] == ["RXBUILD_DIAGNOSTIC_UNUSED"]
            and "hidden" not in json_output,
            "graph diagnostics exposed incorrect toolchain or environment data",
            json_output,
        )
        rc, output = run(exe, root, composed, "--list-targets")
        require(
            rc == 0
            and "TARGET output=" in output
            and "name=child" in output
            and "name=parent" in output
            and "RXBUILD: built" not in output,
            "target query was incomplete or executed an action",
            output,
        )
        rc, output = run(exe, root, composed, "--show-dependencies", "parent")
        require(
            rc == 0
            and "TARGET " in output
            and "depends_build=child -> target=" in output
            and "RXBUILD: built" not in output,
            "dependency query was incomplete or executed an action",
            output,
        )
        rc, output = run(exe, root, composed, "--reverse-dependencies", "child")
        require(
            rc == 0
            and "TARGET " in output
            and "dependent=" in output
            and "parent.rxbin" in output
            and "RXBUILD: built" not in output,
            "reverse dependency query was incomplete or executed an action",
            output,
        )
        rc, output = run(exe, root, composed, "--show-graph", "parent")
        require(
            rc == 0
            and "TARGET " in output
            and "child" in output
            and "input " in output
            and "RXBUILD: built" not in output,
            "ASCII graph query was incomplete or executed an action",
            output,
        )

        namespaced = test_root / "namespaced.graph.build"
        php_graph = test_root / "php.graph.build"
        fortran_graph = test_root / "fortran_iv.graph.build"
        write_plan(
            php_graph,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/php-reader.rxbin"
  BUILD_NAME = reader
  RULE = compile_program
  INPUT = "{source}"
END BUILD;
''',
        )
        write_plan(
            fortran_graph,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/fortran-reader.rxbin"
  BUILD_NAME = reader
  RULE = compile_program
  INPUT = "{source}"
END BUILD;
''',
        )
        write_plan(
            namespaced,
            f'''INCLUDE_GRAPH "php.graph.build" AS php
INCLUDE_GRAPH "fortran_iv.graph.build" AS fortran_iv
RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/namespaced.rxbin"
  BUILD_NAME = all_readers
  RULE = compile_program
  INPUT = "{source}"
  DEPENDS_ON_BUILD = php.reader
  DEPENDS_ON_BUILD = fortran_iv.reader
END BUILD;
DEFAULT_BUILD = all_readers;
''',
        )
        rc, output = run(exe, root, namespaced)
        require(rc == 0 and output.count("built") == 3, "namespaced graph build failed", output)
        rc, output = run(exe, root, namespaced, "php.reader")
        require(rc == 0 and "up to date" in output, "qualified logical target selection failed", output)
        rc, output = run(exe, root, namespaced, "--dump-graph")
        require(
            rc == 0
            and "source_graph=php" in output
            and "source_graph=fortran_iv" in output
            and "depends_build=php.reader -> output=" in output
            and "depends_build=fortran_iv.reader -> output=" in output,
            "namespaced graph dump was incomplete",
            output,
        )

        multi_output = test_root / "multi-output.graph.build"
        write_plan(
            multi_output,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/multi-output.rxbin"
  RULE = compile_program
  INPUT = "{source}"
  OUTPUT = "tools/rxbuild/build_demo.crexx"
END BUILD;
DEFAULT "{output_dir}/multi-output.rxbin";
''',
        )
        rc, output = run(exe, root, multi_output, "--dump-graph=json")
        try:
            multi_output_json = json.loads(output)
        except json.JSONDecodeError as error:
            raise AssertionError("multi-output JSON dump was invalid: " + str(error) + "\n" + output) from error
        require(
            rc == 0
            and [value.replace("\\", "/") for value in multi_output_json["targets"][0]["outputs"]]
            == [str((root / "tools/rxbuild/build_demo.crexx").resolve()).replace("\\", "/")],
            "secondary output declaration was not preserved",
            output,
        )
        duplicate_output = test_root / "duplicate-output.graph.build"
        write_plan(
            duplicate_output,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/duplicate-output.rxbin"
  RULE = compile_program
  INPUT = "{source}"
  OUTPUT = "{output_dir}/duplicate.map"
  OUTPUT = "{output_dir}/duplicate.map"
END BUILD;
DEFAULT "{output_dir}/duplicate-output.rxbin";
''',
        )
        rc, output = run(exe, root, duplicate_output)
        require(
            rc != 0
            and "duplicate output" in output
            and str(duplicate_output).replace("\\", "/") in output.replace("\\", "/"),
            "source-qualified duplicate output diagnostic was missing",
            output,
        )
        missing_output = test_root / "missing-output.graph.build"
        write_plan(
            missing_output,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/missing-output.rxbin"
  RULE = compile_program
  INPUT = "{source}"
  OUTPUT = "{output_dir}/not-produced.rxbin"
END BUILD;
DEFAULT "{output_dir}/missing-output.rxbin";
''',
        )
        rc, output = run(exe, root, missing_output)
        require(rc != 0 and "action did not produce" in output and "not-produced.rxbin" in output, "missing secondary output was not rejected", output)

        include_cycle_a = test_root / "include-cycle-a.graph.build"
        include_cycle_b = test_root / "include-cycle-b.graph.build"
        write_plan(include_cycle_a, 'INCLUDE_GRAPH "include-cycle-b.graph.build"\n')
        write_plan(include_cycle_b, 'INCLUDE_GRAPH "include-cycle-a.graph.build"\n')
        rc, output = run(exe, root, include_cycle_a)
        require(rc != 0 and "graph include cycle detected" in output, "include cycle was not diagnosed", output)

        unknown_build_name = test_root / "unknown-build-name.graph.build"
        write_plan(
            unknown_build_name,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/unknown-name.rxbin"
  RULE = compile_program
  INPUT = "{source}"
  DEPENDS_ON_BUILD = not_declared
END BUILD;
DEFAULT "{output_dir}/unknown-name.rxbin";
''',
        )
        rc, output = run(exe, root, unknown_build_name)
        require(
            rc != 0 and "Unknown build target name: not_declared" in output,
            "unknown logical dependency was not diagnosed",
            output,
        )

        ambiguous_default = test_root / "ambiguous-default.graph.build"
        write_plan(
            ambiguous_default,
            f'''BUILD "{output_dir}/ambiguous.rxbin"
  BUILD_NAME = ambiguous
  RULE = compile_program
  INPUT = "{source}"
END BUILD;
DEFAULT "{output_dir}/ambiguous.rxbin";
DEFAULT_BUILD = ambiguous;
''',
        )
        rc, output = run(exe, root, ambiguous_default)
        require(
            rc != 0 and "both DEFAULT and DEFAULT_BUILD are defined" in output,
            "ambiguous defaults were not diagnosed",
            output,
        )

        missing = test_root / "missing.graph.build"
        write_plan(
            missing,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/missing.rxbin"
  RULE = compile_program
  INPUT = "does/not/exist.crexx"
END BUILD;
DEFAULT "{output_dir}/missing.rxbin";
''',
        )
        rc, output = run(exe, root, missing)
        require(rc != 0 and "missing input" in output, "missing input was not diagnosed", output)

        unknown = test_root / "unknown.graph.build"
        write_plan(
            unknown,
            f'''BUILD "{output_dir}/unknown.rxbin"
  RULE = no_such_rule
  INPUT = "{source}"
END BUILD;
DEFAULT "{output_dir}/unknown.rxbin";
''',
        )
        rc, output = run(exe, root, unknown)
        require(rc != 0 and "unknown rule" in output, "unknown rule was not diagnosed", output)

        cycle = test_root / "cycle.graph.build"
        write_plan(
            cycle,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/a.rxbin"
  RULE = compile_program
  INPUT = "{source}"
  DEPENDS_ON = "{output_dir}/b.rxbin"
END BUILD;
BUILD "{output_dir}/b.rxbin"
  RULE = compile_program
  INPUT = "{source}"
  DEPENDS_ON = "{output_dir}/a.rxbin"
END BUILD;
DEFAULT "{output_dir}/a.rxbin";
''',
        )
        rc, output = run(exe, root, cycle)
        require(rc != 0 and "dependency cycle" in output, "dependency cycle was not diagnosed", output)

        recursive_logical = test_root / "recursive-logical.graph.build"
        write_plan(
            recursive_logical,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/logical-a.rxbin"
  BUILD_NAME = logical_a
  RULE = compile_program
  INPUT = "{source}"
  DEPENDS_ON_BUILD = logical_b
END BUILD;
BUILD "{output_dir}/logical-b.rxbin"
  BUILD_NAME = logical_b
  RULE = compile_program
  INPUT = "{source}"
  DEPENDS_ON_BUILD = logical_a
END BUILD;
DEFAULT_BUILD = logical_a;
''',
        )
        rc, output = run(exe, root, recursive_logical)
        require(rc != 0 and "dependency cycle" in output, "recursive logical dependency was not diagnosed", output)

        duplicate_physical = test_root / "duplicate-physical.graph.build"
        write_plan(
            duplicate_physical,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/duplicate-physical.rxbin"
  RULE = compile_program
  INPUT = "{source}"
  DEPENDS_ON = "{source}"
  DEPENDS_ON = "{source}"
END BUILD;
DEFAULT "{output_dir}/duplicate-physical.rxbin";
''',
        )
        rc, output = run(exe, root, duplicate_physical)
        require(rc != 0 and "duplicate dependency" in output, "duplicate physical dependency was not diagnosed", output)

        duplicate_logical = test_root / "duplicate-logical.graph.build"
        write_plan(
            duplicate_logical,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/duplicate-logical-dependency.rxbin"
  BUILD_NAME = duplicate_source
  RULE = compile_program
  INPUT = "{source}"
END BUILD;
BUILD "{output_dir}/duplicate-logical.rxbin"
  RULE = compile_program
  INPUT = "{source}"
  DEPENDS_ON_BUILD = duplicate_source
  DEPENDS_ON_BUILD = duplicate_source
END BUILD;
DEFAULT "{output_dir}/duplicate-logical.rxbin";
''',
        )
        rc, output = run(exe, root, duplicate_logical)
        require(rc != 0 and "duplicate dependency" in output, "duplicate logical dependency was not diagnosed", output)

        duplicate = test_root / "duplicate.graph.build"
        write_plan(
            duplicate,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{output_dir}/same.rxbin"
  RULE = compile_program
  INPUT = "{source}"
END BUILD;
BUILD "{output_dir}/same.rxbin"
  RULE = compile_program
  INPUT = "{source}"
END BUILD;
DEFAULT "{output_dir}/same.rxbin";
''',
        )
        rc, output = run(exe, root, duplicate)
        require(rc != 0 and "duplicate target" in output, "duplicate target was not diagnosed", output)

        exe_plan = test_root / "create-exe.graph.build"
        write_plan(
            exe_plan,
            f'''RULE create_exe
  ACTION = "CREATE EXE"
END RULE;
BUILD "{output_dir}/demo"
  RULE = create_exe
  INPUT = "{source}"
END BUILD;
DEFAULT "{output_dir}/demo";
''',
        )
        rc, output = run(exe, root, exe_plan)
        require(rc == 0 and "CREATE EXE:" in output, "CREATE EXE graph action failed", output)
        executable = root / output_dir / "demo"
        if sys.platform == "win32":
            executable = executable.with_suffix(".exe")
        require(executable.is_file(), "CREATE EXE did not create its target", output)

        run_plan = test_root / "run.graph.build"
        write_plan(
            run_plan,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
RULE run_program
  ACTION = "RUN"
END RULE;
BUILD "{output_dir}/run-program.rxbin"
  RULE = compile_program
  INPUT = "{source}"
END BUILD;
BUILD "run-demo"
  RULE = run_program
  INPUT = "{output_dir}/run-program.rxbin"
  DEPENDS_ON = "{output_dir}/run-program.rxbin"
  WORK_DIR = "."
  ARGUMENT = "-a" "tools/rxbuild/graph/example.graph.build"
  RUNTIME_ARGUMENT = "tools/rxbuild/graph/example.graph.build"
  RUNTIME_ARGUMENT = "tools/rxbuild/graph/README.md"
  STDIN = ""
END BUILD;
DEFAULT "run-demo";
''',
        )
        rc, output = run(
            exe,
            root,
            run_plan,
            "--verbose",
            "--env",
            "RXBUILD_PHASE92=enabled",
            "--unset-env",
            "RXBUILD_PHASE92_UNUSED",
        )
        require(
            rc == 0
            and "RXBUILD: ran" in output
            and "RXBUILD DEMO: generated program executed" in output
            and "RXBUILD ENVIRONMENT: overrides=1 unset=1" in output,
            "RUN graph action failed or hid program output",
            output,
        )
        require(output.count("runtime_argument=") == 2, "RUN runtime arguments were not forwarded", output)

        dry_run_plan = test_root / "dry-run.graph.build"
        dry_run_output = output_dir + "/dry-run.rxbin"
        write_plan(
            dry_run_plan,
            f'''RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;
BUILD "{dry_run_output}"
  RULE = compile_program
  INPUT = "{source}"
END BUILD;
DEFAULT "{dry_run_output}";
''',
        )
        rc, output = run(exe, root, dry_run_plan, "--dry-run", "--verbose")
        require(
            rc == 0
            and "state=dry-run" in output
            and "RXBUILD: would build" in output
            and "RXBUILD: built" not in output
            and not (root / dry_run_output).is_file(),
            "dry-run executed or failed to announce the build",
            output,
        )

        print("RXBUILD_TEST|PASS")
        return 0
    finally:
        shutil.rmtree(test_root, ignore_errors=True)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print("RXBUILD_TEST|FAIL|" + str(error))
        raise SystemExit(1)
