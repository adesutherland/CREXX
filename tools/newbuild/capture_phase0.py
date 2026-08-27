#!/usr/bin/env python3
"""Capture a clean current-build Phase 0 evidence bundle.

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
        "schema_version": "crexx.phase0-capture/v1",
        "source": source_state,
        "configuration": args.configuration,
        "jobs": args.jobs,
        "timing_policy": {
            "classification": "indicative-non-comparative",
            "reason": "Other host and cluster activities were allowed during Phase 0 capture.",
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
