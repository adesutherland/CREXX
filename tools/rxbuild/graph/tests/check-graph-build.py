"""Focused tests for the canonical rxbuild graph implementation."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
import uuid
from pathlib import Path


def run(exe: Path, root: Path, plan: Path, *target: str) -> tuple[int, str]:
    completed = subprocess.run(
        [str(exe), str(plan), *target],
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

        rc, output = run(exe, root, plan)
        require(rc == 0, "default graph build failed", output)
        require(output.count("built") == 2, "default graph did not build both targets", output)

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

        rc, output = run(exe, root, plan)
        require(rc == 0, "incremental graph build failed", output)
        require(output.count("up to date") == 2, "incremental graph was not a no-op", output)

        final_output = root / output_dir / "final.rxbin"
        final_output.unlink()
        rc, output = run(exe, root, plan, output_dir + "/final.rxbin")
        require(rc == 0, "explicit target rebuild failed", output)
        require("up to date " + output_dir + "/prepare.rxbin" in output, "dependency was rebuilt unexpectedly", output)
        require("built " + output_dir + "/final.rxbin" in output, "explicit target was not rebuilt", output)

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
  STDIN = ""
END BUILD;
DEFAULT "run-demo";
''',
        )
        rc, output = run(exe, root, run_plan)
        require(
            rc == 0
            and "RXBUILD: ran" in output
            and "RXBUILD DEMO: generated program executed" in output,
            "RUN graph action failed or hid program output",
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
