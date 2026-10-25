#!/usr/bin/env python3
"""Capture a clean current-build graph evidence bundle.

This is build-shape evidence, not a performance benchmark. Timings are always
labelled indicative because unrelated host or cluster activity may be present.
"""

from __future__ import annotations

import argparse
import datetime as dt
import gzip
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any


def run_capture(command: list[str], *, cwd: Path | None = None, check: bool = True) -> str:
    result = subprocess.run(command, cwd=cwd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
    if check and result.returncode != 0:
        raise RuntimeError(f"command failed ({result.returncode}): {' '.join(command)}\n{result.stdout[-4000:]}")
    return result.stdout


def utc_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def host_snapshot() -> dict[str, Any]:
    snapshot: dict[str, Any] = {
        "captured_at": utc_now(),
        "system": platform.system(),
        "release": platform.release(),
        "machine": platform.machine(),
        "platform": platform.platform(),
        "cpu_count": os.cpu_count(),
        "load_average_1_5_15": list(os.getloadavg()) if hasattr(os, "getloadavg") else None,
    }
    if platform.system() == "Darwin":
        for key in ("hw.logicalcpu", "hw.memsize", "machdep.cpu.brand_string"):
            value = run_capture(["sysctl", "-n", key], check=False).strip()
            if value:
                snapshot[key] = value
        snapshot["power"] = run_capture(["pmset", "-g", "batt"], check=False).strip()
        snapshot["thermal"] = run_capture(["pmset", "-g", "therm"], check=False).strip()
    else:
        for path in (Path("/proc/meminfo"), Path("/proc/cpuinfo")):
            if path.exists():
                text = path.read_text(encoding="utf-8", errors="replace")
                if path.name == "meminfo":
                    snapshot["meminfo"] = "\n".join(text.splitlines()[:8])
                else:
                    model_lines = sorted({line for line in text.splitlines() if line.lower().startswith(("model name", "model\t"))})
                    snapshot["cpu_models"] = model_lines[:4]
        snapshot["power"] = "not available in generic Linux capture"
    return snapshot


def parse_time_output(text: str, system: str) -> dict[str, Any]:
    result: dict[str, Any] = {"raw": text.strip()}
    if system == "Darwin":
        first_line = text.splitlines()[0] if text.splitlines() else ""
        match = re.search(r"([0-9.]+) real\s+([0-9.]+) user\s+([0-9.]+) sys", first_line)
        if match:
            result.update(
                {
                    "elapsed_seconds": float(match.group(1)),
                    "user_seconds": float(match.group(2)),
                    "system_seconds": float(match.group(3)),
                }
            )
        rss_match = re.search(r"^\s*(\d+)\s+maximum resident set size\s*$", text, re.MULTILINE)
        if rss_match:
            result["maximum_resident_set_bytes"] = int(rss_match.group(1))
    else:
        fields: dict[str, str] = {}
        for line in text.splitlines():
            if ": " in line:
                key, value = line.strip().split(": ", 1)
                fields[key] = value
        elapsed = fields.get("Elapsed (wall clock) time (h:mm:ss or m:ss)")
        if elapsed:
            pieces = elapsed.split(":")
            try:
                if len(pieces) == 3:
                    result["elapsed_seconds"] = int(pieces[0]) * 3600 + int(pieces[1]) * 60 + float(pieces[2])
                elif len(pieces) == 2:
                    result["elapsed_seconds"] = int(pieces[0]) * 60 + float(pieces[1])
            except ValueError:
                pass
        for source_key, target_key in (
            ("User time (seconds)", "user_seconds"),
            ("System time (seconds)", "system_seconds"),
        ):
            if source_key in fields:
                try:
                    result[target_key] = float(fields[source_key])
                except ValueError:
                    pass
        rss = fields.get("Maximum resident set size (kbytes)")
        if rss and rss.isdigit():
            result["maximum_resident_set_bytes"] = int(rss) * 1024
    return result


def timed_command(label: str, command: list[str], output_root: Path) -> dict[str, Any]:
    log_path = output_root / f"{label}.log"
    time_path = output_root / f"{label}.time.txt"
    if platform.system() == "Darwin":
        timed = ["/usr/bin/time", "-l", "-o", str(time_path), *command]
    else:
        timed = ["/usr/bin/time", "-v", "-o", str(time_path), *command]
    started = utc_now()
    with log_path.open("w", encoding="utf-8", newline="\n") as log:
        result = subprocess.run(timed, text=True, stdout=log, stderr=subprocess.STDOUT, check=False)
    finished = utc_now()
    time_text = time_path.read_text(encoding="utf-8", errors="replace") if time_path.exists() else ""
    return {
        "label": label,
        "command": command,
        "started_at": started,
        "finished_at": finished,
        "exit_code": result.returncode,
        "log": f"{log_path.name}.gz",
        "time_file": time_path.name,
        "metrics": parse_time_output(time_text, platform.system()),
    }


def gzip_file(path: Path) -> Path:
    target = path.with_suffix(path.suffix + ".gz")
    with path.open("rb") as source:
        with target.open("wb") as raw_target:
            with gzip.GzipFile(filename="", fileobj=raw_target, mode="wb", compresslevel=9, mtime=0) as destination:
                shutil.copyfileobj(source, destination)
    path.unlink()
    return target


def write_json(path: Path, value: Any) -> None:
    with path.open("w", encoding="utf-8", newline="\n") as handle:
        json.dump(value, handle, indent=2, sort_keys=True)
        handle.write("\n")


def verify_source(source: Path, expected_commit: str, allow_source_without_git: bool) -> dict[str, Any]:
    probe = subprocess.run(
        ["git", "-C", str(source), "rev-parse", "HEAD"],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if probe.returncode != 0:
        if allow_source_without_git:
            return {"git_available": False, "commit": expected_commit, "clean": "archive-assumed"}
        raise RuntimeError(f"source is not a Git worktree: {source}")
    commit = probe.stdout.strip()
    if commit != expected_commit:
        raise RuntimeError(f"source commit {commit} does not match requested baseline {expected_commit}")
    status = run_capture(["git", "-C", str(source), "status", "--porcelain=v1"])
    if status.strip():
        raise RuntimeError(f"source worktree is not clean:\n{status}")
    return {"git_available": True, "commit": commit, "clean": True}


def capture(args: argparse.Namespace) -> int:
    source = args.source.resolve()
    build = args.build.resolve()
    evidence = args.evidence.resolve()
    if build.exists() and any(build.iterdir()):
        raise RuntimeError(f"build directory must be absent or empty: {build}")
    if evidence.exists() and any(evidence.iterdir()):
        raise RuntimeError(f"evidence directory must be absent or empty: {evidence}")
    build.mkdir(parents=True, exist_ok=True)
    evidence.mkdir(parents=True, exist_ok=True)
    source_state = verify_source(source, args.source_commit, args.allow_source_without_git)

    query = build / ".cmake" / "api" / "v1" / "query"
    query.mkdir(parents=True, exist_ok=True)
    for name in ("codemodel-v2", "cache-v2", "cmakeFiles-v1", "toolchains-v1"):
        (query / name).touch()

    trace = evidence / "cmake-trace.jsonl"
    ctest_json = evidence / "ctest.json"
    configure_command = [
        "cmake",
        "-S",
        str(source),
        "-B",
        str(build),
        "-G",
        "Ninja",
        f"-DCMAKE_BUILD_TYPE={args.configuration}",
        *args.cmake_arg,
        "--trace-expand",
        "--trace-format=json-v1",
        f"--trace-redirect={trace}",
    ]
    build_command = ["cmake", "--build", str(build), "--parallel", str(args.jobs)]

    capture_record: dict[str, Any] = {
        "schema_version": "crexx.build-graph-capture/v1",
        "source": source_state,
        "configuration": args.configuration,
        "jobs": args.jobs,
        "timing_policy": {
            "classification": "indicative-non-comparative",
            "reason": "Other host and cluster activities were allowed during capture.",
            "permitted_use": "gross build shape and resource-pressure observation only",
            "prohibited_use": "performance baseline, regression verdict or product comparison",
        },
        "host_before": host_snapshot(),
        "tools": {
            "cmake": run_capture(["cmake", "--version"]).splitlines()[0],
            "ninja": run_capture(["ninja", "--version"]).strip(),
            "python": sys.version.splitlines()[0],
        },
        "commands": [],
    }
    configure_result = timed_command("configure", configure_command, evidence)
    capture_record["commands"].append(configure_result)
    if configure_result["exit_code"] != 0:
        capture_record["host_after"] = host_snapshot()
        capture_record["status"] = "configure-failed"
        write_json(evidence / "capture.json", capture_record)
        return configure_result["exit_code"]

    build_result = timed_command("build", build_command, evidence)
    capture_record["commands"].append(build_result)
    if build_result["exit_code"] != 0:
        capture_record["host_after"] = host_snapshot()
        capture_record["status"] = "build-failed"
        write_json(evidence / "capture.json", capture_record)
        return build_result["exit_code"]

    with ctest_json.open("w", encoding="utf-8", newline="\n") as output:
        ctest_result = subprocess.run(
            ["ctest", "--test-dir", str(build), "--show-only=json-v1"],
            text=True,
            stdout=output,
            stderr=subprocess.PIPE,
            check=False,
        )
    (evidence / "ctest.stderr.log").write_text(ctest_result.stderr, encoding="utf-8")
    if ctest_result.returncode != 0:
        raise RuntimeError(f"CTest inventory failed: {ctest_result.stderr[-4000:]}")

    ninja_commands = {
        "ninja-targets.txt": ["ninja", "-C", str(build), "-t", "targets", "all"],
        "ninja-graph.dot": ["ninja", "-C", str(build), "-t", "graph", "all"],
        "ninja-commands.txt": ["ninja", "-C", str(build), "-t", "commands", "all"],
        "ninja-missingdeps.txt": ["ninja", "-C", str(build), "-t", "missingdeps", "all"],
    }
    for filename, command in ninja_commands.items():
        result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
        (evidence / filename).write_text(result.stdout, encoding="utf-8")
        capture_record["commands"].append({"label": filename, "command": command, "exit_code": result.returncode})
        if result.returncode != 0:
            raise RuntimeError(f"Ninja graph command failed: {' '.join(command)}\n{result.stdout[-4000:]}")

    exporter = Path(__file__).with_name("cmake_catalogue.py")
    export_command = [
        sys.executable,
        str(exporter),
        "export",
        "--source",
        str(source),
        "--build",
        str(build),
        "--trace",
        str(trace),
        "--ctest-json",
        str(ctest_json),
        "--source-commit",
        args.source_commit,
        "--output",
        str(evidence),
    ]
    export_result = subprocess.run(
        export_command,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    (evidence / "catalogue-export.log").write_text(export_result.stdout, encoding="utf-8")
    capture_record["commands"].append(
        {"label": "catalogue-export", "command": export_command, "exit_code": export_result.returncode}
    )
    if export_result.returncode != 0:
        raise RuntimeError(f"catalogue export failed:\n{export_result.stdout[-4000:]}")

    capture_record["host_after"] = host_snapshot()
    capture_record["status"] = "complete"
    write_json(evidence / "capture.json", capture_record)

    for path in (
        trace,
        ctest_json,
        evidence / "ninja-targets.txt",
        evidence / "ninja-graph.dot",
        evidence / "ninja-commands.txt",
        evidence / "catalogue.json",
        evidence / "manifest-projection.json",
        evidence / "configure.log",
        evidence / "build.log",
    ):
        if path.exists():
            gzip_file(path)
    print(evidence)
    return 0


def ninja_query_inputs(text: str) -> tuple[str, list[str]]:
    lines = text.splitlines()
    if not lines or lines[0].startswith(" ") or not lines[0].endswith(":"):
        raise RuntimeError("Unrecognized Ninja query header")
    rule = None
    inputs: list[str] = []
    section = None
    for line in lines[1:]:
        if line.startswith("  input: ") and section is None:
            rule, section = line[len("  input: "):], "input"
        elif line == "  outputs:" and section == "input":
            section = "outputs"
        elif line.startswith("    ") and section == "input" and line[4:]:
            inputs.append(line[4:].lstrip("| "))
        elif line.startswith("    ") and section == "outputs" and line[4:]:
            pass
        else:
            raise RuntimeError("Unrecognized Ninja input query: " + line)
    if not rule or section != "outputs":
        raise RuntimeError("Incomplete Ninja input query")
    return rule, inputs


def classify_missingdeps(source: Path, build: Path, report: str, ninja: str) -> tuple[int, int]:
    records = []
    declared_count = None
    clean = processed = advisory = False
    summary = None
    for line in report.splitlines():
        footer = re.sub(r"[ \t]+", " ", line)
        if line.startswith("Missing dep:") and (line.count(" uses ") != 1
                or line.count(" (generated by ") != 1):
            raise RuntimeError("Ambiguous Ninja missingdeps record: " + line)
        match = re.fullmatch(r"Missing dep: (.+) uses (.+) \(generated by ([^)]+)\)", line)
        if match:
            records.append((match[1], match[2], match[3]))
        elif match := re.fullmatch(r"Error: There are (\d+) missing dependency paths\.", footer):
            if declared_count is not None:
                raise RuntimeError("Duplicate Ninja missingdeps count")
            declared_count = int(match[1])
        elif footer == "No missing dependencies on generated files found.":
            if clean:
                raise RuntimeError("Duplicate Ninja missingdeps clean footer")
            clean = True
        elif footer == ("There might be build flakiness if any of the targets listed "
                      "above are built alone, or not late enough, in a clean output directory."):
            if advisory:
                raise RuntimeError("Duplicate Ninja missingdeps advisory")
            advisory = True
        elif re.fullmatch(r"Processed \d+ nodes\.", footer):
            if processed:
                raise RuntimeError("Duplicate Ninja missingdeps processed footer")
            processed = True
        elif match := re.fullmatch(
                r"(\d+) targets had depfile dependencies on (\d+) distinct generated inputs "
                r"\(from (\d+) rules\) without a non-depfile dep path to the generator\.", footer):
            if summary is not None:
                raise RuntimeError("Duplicate Ninja missingdeps summary")
            summary = tuple(map(int, match.groups()))
        else:
            raise RuntimeError("Unrecognized Ninja missingdeps output: " + line)
    if not processed:
        raise RuntimeError("Incomplete Ninja missingdeps report")
    # Ninja counts consumer/rule pairs, not individual included paths.
    path_count = len({(row[0], row[2]) for row in records})
    expected = tuple(len({row[index] for row in records}) for index in range(3))
    if records and (declared_count != path_count or clean or not advisory or summary != expected):
        raise RuntimeError("Incomplete or inconsistent Ninja missingdeps report")
    if not records:
        if not clean or advisory or summary is not None or declared_count is not None:
            raise RuntimeError("Incomplete Ninja missingdeps report")
        return 0, 0
    source, build = source.resolve(strict=True), build.resolve(strict=True)
    rule, regeneration = ninja_query_inputs(run_capture(
        [ninja, "-C", str(build), "-t", "query", "build.ninja"]))
    if rule != "RERUN_CMAKE":
        raise RuntimeError("build.ninja is not owned by CMake regeneration")
    def absolute(name: str) -> Path:
        path = Path(name)
        return path if path.is_absolute() else build / path
    regeneration_paths = {absolute(name) for name in regeneration}
    proved = set()
    for _, name, producer in records:
        if producer != "phony":
            raise RuntimeError("Real generated dependency lacks an ordering edge: " + name)
        if name in proved:
            continue
        path = absolute(name)
        if (not path.is_file() or path.is_symlink()
                or path.absolute() != path.resolve(strict=True)
                or not path.is_relative_to(source) or path not in regeneration_paths):
            raise RuntimeError("Unproved generated dependency: " + name)
        run_capture(["git", "--literal-pathspecs", "-C", str(source), "ls-files", "--error-unmatch", "--",
                     path.relative_to(source).as_posix()])
        input_rule, prerequisites = ninja_query_inputs(run_capture(
            [ninja, "-C", str(build), "-t", "query", name]))
        if input_rule != "phony" or prerequisites:
            raise RuntimeError("Dependency is not a zero-input CMake source fallback: " + name)
        proved.add(name)
    return path_count, len(proved)


def missingdeps_output(status: int, output: str) -> str:
    reported = any(line.startswith("Missing dep: ") for line in output.splitlines())
    if status not in (0, 3) or reported != (status == 3):
        raise RuntimeError(f"Unexpected Ninja missingdeps status/report ({status}):\n{output[-4000:]}")
    return output


def capture_missingdeps(ninja: str, build: Path, target: str) -> str:
    result = subprocess.run([ninja, "-C", str(build), "-t", "missingdeps", target],
                            text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
    return missingdeps_output(result.returncode, result.stdout)


def missingdeps_self_test(ninja: str) -> None:
    cmake = shutil.which("cmake")
    if not cmake:
        raise RuntimeError("The graph self-test requires the existing CMake tool")
    with tempfile.TemporaryDirectory(prefix="crexx-missingdeps-") as temporary:
        root = Path(temporary).resolve()
        source, build = root / "source with spaces", root / "build with spaces"
        source.mkdir(); build.mkdir()
        watched, untracked, indirect = [source / name for name in
                                        ("watched header.h", "watch[1].h", "indirect.h")]
        for path in (watched, untracked, indirect):
            path.write_text("/* maintained source */\n")
        second = source / "second header.h"; second.write_text("/* second tracked input */\n")
        alias = source / "alias.h"; alias.symlink_to(watched)
        sibling = source / "watch1.h"; sibling.write_text("/* tracked pathspec sibling */\n")
        run_capture(["git", "init", "-q", str(source)])
        run_capture(["git", "-C", str(source), "add", "--", watched.name, second.name,
                     indirect.name, alias.name, sibling.name])
        def escape(path: Path) -> str:
            return path.as_posix().replace("$", "$$").replace(" ", "$ ").replace(":", "$:")
        graph = ["rule RERUN_CMAKE", f'  command = "{Path(cmake).as_posix()}" -E touch $out',
                 "  generator = 1", "rule generate",
                 f'  command = "{Path(cmake).as_posix()}" -E touch $out',
                 "rule consume", f'  command = "{Path(cmake).as_posix()}" -E touch $out',
                 "  depfile = $out.d", "  deps = gcc",
                 f"build build.ninja: RERUN_CMAKE {escape(watched)} {escape(second)} {escape(untracked)} {escape(indirect)} {escape(alias)}",
                 f"build {escape(watched)}: phony", f"build {escape(untracked)}: phony",
                 f"build {escape(second)}: phony",
                 f"build {escape(alias)}: phony",
                 f"build {escape(indirect)}: phony generated.h", "build generated.h: generate"]
        for name, header in (("good", watched), ("untracked", untracked), ("alias", alias),
                             ("indirect", indirect), ("bad", Path("generated.h"))):
            graph.append(f"build {name}.o: consume")
            dep = header.as_posix().replace(" ", "\\ ")
            if name == "good": dep += " " + second.as_posix().replace(" ", "\\ ")
            (build / f"{name}.o.d").write_text(f"{name}.o: {dep}\n")
        graph.append("build all: phony good.o untracked.o alias.o indirect.o bad.o generated.h")
        (build / "build.ninja").write_text("\n".join(graph) + "\n")
        run_capture([ninja, "-C", str(build), "all"])
        source_report = ""
        for name in ("good", "untracked", "alias", "indirect", "bad"):
            report = capture_missingdeps(ninja, build, name + ".o")
            if name == "good": source_report = report
            if name == "bad" and " (generated by generate)" not in report:
                raise RuntimeError("Real generated-edge control produced no missing edge:\n" + report)
            try:
                result = classify_missingdeps(source, build, report, ninja)
            except RuntimeError:
                if name == "good":
                    raise
            else:
                if name != "good" or result != (1, 2):
                    raise RuntimeError("Graph control failed to reject " + name + ":\n" + report)
        spaced = "\n".join(line if line.startswith("Missing dep: ") else line.replace(" ", "\t  ")
                           for line in source_report.splitlines()) + "\n"
        if classify_missingdeps(source, build, spaced, ninja) != (1, 2):
            raise RuntimeError("Horizontal footer spacing changed the source proof")
        outside = root / "outside.h"; outside.write_text("/* outside checkout */\n")
        for invalid in (source / "missing.h", outside):
            try:
                classify_missingdeps(source, build, source_report.replace(
                    watched.as_posix(), invalid.as_posix()), ninja)
            except RuntimeError:
                pass
            else:
                raise RuntimeError("Unowned/missing source path was accepted")
        # Contradictory/ambiguous tool output must never become a clean result.
        malformed = ["Missing dep: unknown\n", source_report + "Processed 1 nodes.\n",
                     source_report.replace(" uses ", " uses ambiguous uses ", 1),
                     "Processed 1 nodes.\nNo missing dependencies on generated files found.\n"
                     "1 targets had depfile dependencies on 1 distinct generated inputs "
                     "(from 1 rules)  without a non-depfile dep path to the generator.\n"]
        for report in malformed:
            try:
                classify_missingdeps(source, build, report, ninja)
            except RuntimeError:
                pass
            else:
                raise RuntimeError("Malformed graph report was accepted")
        for status, report in ((1, source_report), (-9, source_report), (0, source_report),
                               (3, "Processed 1 nodes.\nNo missing dependencies on generated files found.\n"),
                               (3, "Missing dep: truncated\n")):
            try:
                classify_missingdeps(source, build, missingdeps_output(status, report), ninja)
            except RuntimeError:
                pass
            else:
                raise RuntimeError("Invalid Ninja status/report was accepted")
        try:
            ninja_query_inputs("node:\n  input: phony\n  outputs:\n  validations:\n    hidden\n")
        except RuntimeError:
            pass
        else:
            raise RuntimeError("Unknown Ninja query section was accepted")
    print("PASS: Ninja source-fallback and real missing-edge controls")


def missingdeps_main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Check real generated-file ordering without source-watch false positives")
    parser.add_argument("--source", type=Path)
    parser.add_argument("--build", type=Path)
    parser.add_argument("--target", default="stage-product")
    parser.add_argument("--ninja", default="ninja")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args(argv)
    if args.self_test:
        missingdeps_self_test(args.ninja)
    else:
        if args.source is None or args.build is None:
            parser.error("--source and --build are required")
        report = capture_missingdeps(args.ninja, args.build, args.target)
        print(report, end="" if report.endswith("\n") else "\n")
        paths, inputs = classify_missingdeps(args.source, args.build, report, args.ninja)
        print(f"Generated-file dependency check passed; {paths} paths use {inputs} proved CMake source fallbacks")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--evidence", type=Path, required=True)
    parser.add_argument("--source-commit", required=True)
    parser.add_argument("--configuration", choices=("Debug", "Release"), required=True)
    parser.add_argument("--jobs", type=int, required=True)
    parser.add_argument("--cmake-arg", action="append", default=[])
    parser.add_argument("--allow-source-without-git", action="store_true")
    return parser


def main() -> int:
    if sys.argv[1:2] == ["check-missingdeps"]:
        try:
            return missingdeps_main(sys.argv[2:])
        except (OSError, RuntimeError) as exc:
            print(f"error: {exc}", file=sys.stderr)
            return 1
    args = build_parser().parse_args()
    if args.jobs < 1:
        print("error: --jobs must be positive", file=sys.stderr)
        return 2
    try:
        return capture(args)
    except (OSError, RuntimeError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
