#!/usr/bin/env python3
"""Export and validate an observation-only catalogue of the current CMake graph.

The exporter combines CMake File API data, expanded CMake trace events and
CTest's JSON inventory.  It deliberately does not change the configured build
or infer that an observed relationship is already a correct future build DAG.
"""

from __future__ import annotations

import argparse
import collections
import gzip
import hashlib
import json
import os
import platform
import re
import shlex
import sys
from pathlib import Path
from typing import Any, Iterable


CATALOGUE_SCHEMA = "crexx.cmake-catalogue/v1"
MANIFEST_SCHEMA = "crexx.build-manifest/v1"
PRODUCT_LAYERS = {"C0", "C1", "B0", "X", "B1", "C", "G", "L", "Product", "Optional"}
QA_TIERS = {"none", "graph", "essential", "smoke", "comprehensive", "qualification", "stress", "measurement"}

CUSTOM_KEYWORDS = {
    "ALL",
    "APPEND",
    "ARGS",
    "BYPRODUCTS",
    "COMMAND",
    "COMMAND_EXPAND_LISTS",
    "COMMENT",
    "DEPENDS",
    "DEPFILE",
    "IMPLICIT_DEPENDS",
    "JOB_POOL",
    "JOB_SERVER_AWARE",
    "MAIN_DEPENDENCY",
    "OUTPUT",
    "POST_BUILD",
    "PRE_BUILD",
    "PRE_LINK",
    "SOURCES",
    "TARGET",
    "USES_TERMINAL",
    "VERBATIM",
    "WORKING_DIRECTORY",
}


class CatalogueError(RuntimeError):
    """A deterministic catalogue input is missing or malformed."""


class PathNormalizer:
    def __init__(self, source_root: Path, build_root: Path) -> None:
        self.source_root = source_root.absolute()
        self.build_root = build_root.absolute()
        self.source_parent = self.source_root.parent
        self.replacements: list[tuple[str, str]] = []
        for path, replacement in (
            (self.build_root, "<BUILD>"),
            (self.build_root.resolve(), "<BUILD>"),
            (self.source_root, "<SOURCE>"),
            (self.source_root.resolve(), "<SOURCE>"),
            (self.source_parent, "<SOURCE_PARENT>"),
            (self.source_parent.resolve(), "<SOURCE_PARENT>"),
        ):
            item = (str(path), replacement)
            if item not in self.replacements:
                self.replacements.append(item)
        self.replacements.sort(key=lambda item: len(item[0]), reverse=True)

    def text(self, value: Any) -> Any:
        if not isinstance(value, str):
            if isinstance(value, list):
                return [self.text(item) for item in value]
            if isinstance(value, dict):
                return {key: self.text(item) for key, item in value.items()}
            return value
        result = value
        for original, replacement in self.replacements:
            result = result.replace(original, replacement)
        return result

    def path(self, value: str | Path, anchor: Path | None = None) -> str:
        raw = str(value)
        if not raw or raw.startswith("$") or raw.startswith("<"):
            return self.text(raw)
        candidate = Path(raw)
        if not candidate.is_absolute() and anchor is not None:
            candidate = anchor / candidate
        return self.text(str(candidate))


def read_json(path: Path) -> dict[str, Any]:
    try:
        if path.suffix == ".gz":
            handle_context = gzip.open(path, mode="rt", encoding="utf-8")
        else:
            handle_context = path.open(encoding="utf-8")
        with handle_context as handle:
            value = json.load(handle)
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise CatalogueError(f"cannot read JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise CatalogueError(f"expected a JSON object in {path}")
    return value


def write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="\n") as handle:
        json.dump(value, handle, indent=2, sort_keys=True)
        handle.write("\n")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        while block := handle.read(1024 * 1024):
            digest.update(block)
    return digest.hexdigest()


def counter_dict(values: Iterable[Any]) -> dict[str, int]:
    return dict(sorted(collections.Counter(str(value) for value in values).items()))


def ordered_unique(values: Iterable[str]) -> list[str]:
    return list(dict.fromkeys(values))


def classify_surface(source: str, name: str, target_type: str = "") -> dict[str, Any]:
    path = source.lower().replace("\\", "/")
    target = name.lower()

    explicit_names = {
        "library": ("B0", 3, "none"),
        "library-rexx": ("B0", 3, "none"),
        "library-rxas": ("B0", 3, "none"),
        "rxcexits": ("X", 4, "none"),
        "classlibrary": ("B1", 5, "none"),
        "classlib": ("B1", 5, "none"),
        "classlib_native": ("B1", 5, "none"),
        "veclib": ("B1", 5, "none"),
        "rxfnsc": ("C", 6, "none"),
        "rxfnsg": ("G", 6, "none"),
        "rxfnsl": ("L", 6, "none"),
        "rexxscript": ("C", 6, "none"),
        "rxpp": ("C", 6, "none"),
        "rxdb": ("C", 6, "none"),
        "crexx": ("Product", 7, "none"),
        "rexxscript_cli": ("Product", 7, "none"),
        "packed_library": ("Product", 7, "none"),
        "rxvme": ("Product", 7, "none"),
        "rxbvme": ("Product", 7, "none"),
        "concurrency-qa": ("Optional", 7, "comprehensive"),
    }
    if target in explicit_names:
        layer, wave, qa_tier = explicit_names[target]
        return {"product_layer": layer, "wave": wave, "qa_tier": qa_tier, "basis": "target-name"}
    if target == "qa-prep" or target.startswith("qa-prep-"):
        return {"product_layer": "Product", "wave": 7, "qa_tier": "none", "basis": "qa-prep-target"}
    qa_runner_tiers = {
        "qa-essential": "essential",
        "qa-smoke": "smoke",
        "qa-comprehensive": "comprehensive",
        "qa-qualification": "qualification",
        "qa-stress": "stress",
        "qa-measurement": "measurement",
    }
    if target in qa_runner_tiers:
        return {
            "product_layer": "Optional",
            "wave": 7,
            "qa_tier": qa_runner_tiers[target],
            "basis": "qa-runner-target",
        }
    if (
        target in {"linked_opt_runtime_artifacts", "concurrency_test_artifacts"}
        or target == "concurrency_doc_examples"
        or target.startswith(
            (
                "test_",
                "performance_",
                "run_rxas",
                "runrxas",
                "module_initializers-",
                "persistent_worker_executor-",
                "program_generation_control-",
            )
        )
        or target.endswith(("_artifact", "_artifacts"))
        or (target_type == "UTILITY" and ("test" in target or "check" in target))
    ):
        return {
            "product_layer": "Optional",
            "wave": 7,
            "qa_tier": "comprehensive",
            "basis": "qa-artifact-target",
        }

    path_rules = (
        ("/tests/", "Optional", 7),
        ("/tests_functional/", "Optional", 7),
        ("/tests_performance/", "Optional", 7),
        ("/parsingtests/", "Optional", 7),
        ("/examples/", "Optional", 7),
        ("/demos/", "Optional", 7),
        ("/performance/", "Optional", 7),
        ("/experiments/", "Optional", 7),
        ("/compiler/exits/", "X", 4),
        ("/lib/rxfnsb/", "B0", 3),
        ("/lib/classlib_native/", "B1", 5),
        ("/lib/classlib/", "B1", 5),
        ("/lib/plugins/", "B1", 5),
        ("/lib/veclib/", "B1", 5),
        ("/lib/rxfnsc/", "C", 6),
        ("/rexxscript/", "C", 6),
        ("/preprocessor/", "C", 6),
        ("/debugger/", "C", 6),
        ("/lib/rxfnsg/", "G", 6),
        ("/lib/rxfnsl/", "L", 6),
        ("/bin/", "Product", 7),
        ("/assembler/", "C1", 2),
        ("/compiler/", "C1", 2),
        ("/linker/", "C1", 2),
        ("/disassembler/", "C1", 2),
        ("/interpreter/", "C1", 2),
        ("/contract/", "C1", 2),
        ("/platform/", "C0", 1),
        ("/avl_tree/", "C0", 1),
        ("/binutils/", "C0", 1),
        ("/cpacker/", "C0", 1),
        ("/rxpa/", "C0", 1),
        ("/re2c/", "C0", 1),
        ("/lemon/", "C0", 1),
    )
    padded = f"/{path.strip('/')}/"
    for marker, layer, wave in path_rules:
        if marker in padded:
            qa_tier = "comprehensive" if layer == "Optional" and ("test" in target or "/tests/" in padded) else "none"
            return {"product_layer": layer, "wave": wave, "qa_tier": qa_tier, "basis": f"source:{marker.strip('/')}"}

    dashboard_targets = {"experimental", "nightly", "continuous", "test", "package", "package_source"}
    if target in dashboard_targets or target.startswith(("nightly", "experimental", "continuous")):
        return {"product_layer": "Optional", "wave": 7, "qa_tier": "comprehensive", "basis": "cmake-dashboard"}
    if "<source_parent>" in path or "_deps" in path:
        return {"product_layer": "C0", "wave": 1, "qa_tier": "none", "basis": "external-foundation"}
    return {"product_layer": "Product", "wave": 7, "qa_tier": "none", "basis": "fallback-review"}


def classify_test(labels: Iterable[str], name: str) -> str:
    values = {value.lower() for value in labels}
    lower_name = name.lower()
    if "performance-measurement" in values:
        return "measurement"
    if "stress" in lower_name or any("stress" in value for value in values):
        return "stress"
    if "smoke" in values:
        return "smoke"
    if values.intersection({"qualification", "install", "package", "external-consumer", "reproducibility"}):
        return "qualification"
    if values.intersection({"essential", "unit", "contract"}):
        return "essential"
    return "comprehensive"


def backtrace_location(graph: dict[str, Any], index: int | None, normalizer: PathNormalizer) -> dict[str, Any]:
    if index is None:
        return {}
    nodes = graph.get("nodes", [])
    files = graph.get("files", [])
    commands = graph.get("commands", [])
    if not isinstance(index, int) or index < 0 or index >= len(nodes):
        return {}
    node = nodes[index]
    result: dict[str, Any] = {}
    file_index = node.get("file")
    if isinstance(file_index, int) and 0 <= file_index < len(files):
        result["file"] = normalizer.path(Path(files[file_index]), normalizer.source_root)
    command_index = node.get("command")
    if isinstance(command_index, int) and 0 <= command_index < len(commands):
        result["command"] = commands[command_index]
    if "line" in node:
        result["line"] = node["line"]
    return result


def load_file_api(source_root: Path, build_root: Path, normalizer: PathNormalizer) -> dict[str, Any]:
    reply_dir = build_root / ".cmake" / "api" / "v1" / "reply"
    indexes = sorted(reply_dir.glob("index-*.json"))
    if not indexes:
        raise CatalogueError(f"no CMake File API index found under {reply_dir}")
    index = read_json(indexes[-1])
    reply = index.get("reply", {})
    codemodel_ref = reply.get("codemodel-v2")
    if not isinstance(codemodel_ref, dict) or "jsonFile" not in codemodel_ref:
        raise CatalogueError("CMake File API codemodel-v2 reply is missing")
    codemodel = read_json(reply_dir / codemodel_ref["jsonFile"])
    configurations = codemodel.get("configurations", [])
    if len(configurations) != 1:
        raise CatalogueError(f"expected one configured codemodel, found {len(configurations)}")
    config = configurations[0]
    directories = config.get("directories", [])
    paths = codemodel.get("paths", {})
    api_source = Path(paths.get("source", source_root))
    api_build = Path(paths.get("build", build_root))

    directory_records: list[dict[str, Any]] = []
    source_to_build: dict[str, Path] = {}
    for index_value, directory in enumerate(directories):
        source_value = Path(directory.get("source", "."))
        build_value = Path(directory.get("build", "."))
        source_path = source_value if source_value.is_absolute() else api_source / source_value
        build_path = build_value if build_value.is_absolute() else api_build / build_value
        source_to_build[str(source_path.resolve())] = build_path.resolve()
        directory_records.append(
            {
                "index": index_value,
                "source": normalizer.path(source_path),
                "build": normalizer.path(build_path),
                "has_install_rule": bool(directory.get("hasInstallRule", False)),
            }
        )

    raw_targets: list[tuple[dict[str, Any], dict[str, Any]]] = []
    id_to_name: dict[str, str] = {}
    for reference in config.get("targets", []):
        target_data = read_json(reply_dir / reference["jsonFile"])
        raw_targets.append((reference, target_data))
        id_to_name[reference["id"]] = reference["name"]

    targets: list[dict[str, Any]] = []
    artifact_actual_paths: dict[str, list[Path]] = {}
    for reference, target_data in raw_targets:
        directory = directory_records[reference["directoryIndex"]]
        target_id = reference["id"]
        target_type = target_data.get("type", "UNKNOWN")
        location = backtrace_location(target_data.get("backtraceGraph", {}), target_data.get("backtrace"), normalizer)
        classification_source = directory["source"]
        definition_source = str(location.get("file", "")).lower().replace("\\", "/")
        if any(
            marker in f"/{definition_source.strip('/')}/"
            for marker in ("/tests/", "/tests_functional/", "/tests_performance/", "/parsingtests/")
        ):
            classification_source = str(location["file"])
        classification = classify_surface(classification_source, reference["name"], target_type)
        artifacts: list[str] = []
        actual_artifacts: list[Path] = []
        for artifact in target_data.get("artifacts", []):
            raw_path = Path(artifact["path"])
            actual = raw_path if raw_path.is_absolute() else api_build / raw_path
            artifacts.append(normalizer.path(actual))
            actual_artifacts.append(actual)
        artifact_actual_paths[target_id] = actual_artifacts
        sources = []
        for item in target_data.get("sources", []):
            raw_path = Path(item["path"])
            actual_source = raw_path if raw_path.is_absolute() else api_source / raw_path
            sources.append(
                {
                    "path": normalizer.path(actual_source),
                    "generated": bool(item.get("isGenerated", False)),
                }
            )
        dependencies = []
        for dependency in target_data.get("dependencies", []):
            dependency_id = dependency.get("id", "")
            dependencies.append({"id": dependency_id, "name": id_to_name.get(dependency_id, dependency_id)})
        targets.append(
            {
                "id": target_id,
                "name": reference["name"],
                "type": target_type,
                "source_dir": directory["source"],
                "build_dir": directory["build"],
                "definition": location,
                "classification": classification,
                "artifacts": sorted(artifacts),
                "dependencies": sorted(dependencies, key=lambda item: (item["name"], item["id"])),
                "sources": sorted(sources, key=lambda item: item["path"]),
                "source_count": len(sources),
                "generated_source_count": sum(1 for item in sources if item["generated"]),
            }
        )

    abstract_targets = []
    for item in config.get("abstractTargets", []):
        abstract_targets.append(
            {
                "id": item.get("id"),
                "name": item.get("name"),
                "source_dir": directory_records[item["directoryIndex"]]["source"],
            }
        )

    return {
        "index": normalizer.path(indexes[-1]),
        "cmake": index.get("cmake", {}),
        "configuration": config.get("name", ""),
        "directories": directory_records,
        "targets": sorted(targets, key=lambda item: (item["name"], item["id"])),
        "abstract_targets": sorted(abstract_targets, key=lambda item: (str(item["name"]), str(item["id"]))),
        "source_to_build": source_to_build,
        "artifact_actual_paths": artifact_actual_paths,
    }


def sections(args: list[str], keyword: str) -> list[list[str]]:
    found: list[list[str]] = []
    for position, token in enumerate(args):
        if token != keyword:
            continue
        values: list[str] = []
        for value in args[position + 1 :]:
            if value in CUSTOM_KEYWORDS:
                break
            values.append(value)
        found.append(values)
    return found


def expand_control_tokens(args: list[str]) -> list[str]:
    """Expand unquoted CMake list variables that begin with a control keyword.

    CMake's expanded trace can retain an unquoted variable such as
    ``COMMAND;cmake;-E;rm`` as one JSON argument.  A quoted semicolon-separated
    import root does not begin with a CMake control keyword and stays intact.
    """

    expanded: list[str] = []
    for token in args:
        pieces = token.split(";")
        if len(pieces) > 1 and pieces[0] in CUSTOM_KEYWORDS:
            expanded.extend(pieces)
        else:
            expanded.append(token)
    return expanded


def resolve_observed_path(
    value: str,
    working_directory: str | None,
    definition_file: Path,
    source_to_build: dict[str, Path],
    normalizer: PathNormalizer,
) -> tuple[str, Path | None]:
    if not value or "$<" in value or "${" in value:
        return normalizer.text(value), None
    path = Path(value)
    if path.is_absolute():
        return normalizer.path(path), path
    anchor: Path | None = None
    if working_directory and "$<" not in working_directory and "${" not in working_directory:
        working_path = Path(working_directory)
        anchor = working_path if working_path.is_absolute() else None
    if anchor is None:
        source_dir = str(definition_file.parent.resolve())
        anchor = source_to_build.get(source_dir)
    if anchor is None:
        return normalizer.text(value), None
    actual = anchor / path
    return normalizer.path(actual), actual


def split_commands(args: list[str]) -> list[list[str]]:
    return sections(args, "COMMAND")


def is_rxc_token(token: str) -> bool:
    lower = token.lower()
    return (
        lower.endswith("/rxc")
        or lower.endswith("\\rxc.exe")
        or lower in {"rxc", "$<target_file:rxc>", "$<target_file:crexx::rxc>"}
    )


def rxc_command_index(tokens: list[str]) -> int | None:
    if tokens and is_rxc_token(tokens[0]):
        return 0
    if len(tokens) < 4 or tokens[1:3] != ["-E", "env"]:
        return None
    index = 3
    while index < len(tokens) and "=" in tokens[index] and not is_rxc_token(tokens[index]):
        index += 1
    return index if index < len(tokens) and is_rxc_token(tokens[index]) else None


def import_roots(value: str, normalizer: PathNormalizer) -> list[str]:
    return [normalizer.text(item) for item in value.split(";") if item]


def extract_import_policy(tokens: list[str], normalizer: PathNormalizer, source: dict[str, Any]) -> list[dict[str, Any]]:
    command_index = rxc_command_index(tokens)
    if command_index is None:
        return []
    invocation = {
        "source": source,
        "tool": normalizer.text(tokens[command_index]),
        "source_roots": [],
        "binary_roots": [],
        "import_rxas": False,
        "exe_import": True,
    }
    index = command_index + 1
    while index < len(tokens):
        token = tokens[index]
        if token == "--import-rxas":
            invocation["import_rxas"] = True
        elif token == "--no-exe-import":
            invocation["exe_import"] = False
        elif token in {"-i", "-s", "--source"} and index + 1 < len(tokens):
            key = "binary_roots" if token == "-i" else "source_roots"
            invocation[key].extend(import_roots(tokens[index + 1], normalizer))
            index += 1
        index += 1
    return [invocation]


def extract_imports_from_content(content: str, normalizer: PathNormalizer, source: dict[str, Any]) -> list[dict[str, Any]]:
    invocations: list[dict[str, Any]] = []
    for match in re.finditer(r"execute_process\s*\(\s*COMMAND\s+([^\)]*?(?:rxc|TARGET_FILE:rxc)[^\)]*)\)", content, re.DOTALL):
        raw = match.group(1).replace("\\\"", '"')
        try:
            tokens = shlex.split(raw)
        except ValueError:
            tokens = raw.replace('"', " ").split()
        invocations.extend(extract_import_policy(tokens, normalizer, source))
    return invocations


def cleanup_from_tokens(tokens: list[str], normalizer: PathNormalizer, source: dict[str, Any]) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for index, token in enumerate(tokens):
        if token != "-E" or index + 1 >= len(tokens):
            continue
        operation = tokens[index + 1]
        if operation not in {"rm", "remove", "remove_directory"}:
            continue
        paths = [normalizer.text(value) for value in tokens[index + 2 :] if not value.startswith("-")]
        if paths:
            results.append({"source": source, "operation": f"cmake -E {operation}", "paths": paths})
    return results


def cleanup_from_content(content: str, normalizer: PathNormalizer, source: dict[str, Any]) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    pattern = re.compile(r"file\s*\(\s*(REMOVE|REMOVE_RECURSE)\s+([^\)]*)\)", re.DOTALL)
    for match in pattern.finditer(content):
        raw_paths = re.findall(r'"([^"\n]+)"|([^\s"\)]+)', match.group(2))
        paths = [normalizer.text(quoted or plain) for quoted, plain in raw_paths]
        if paths:
            results.append({"source": source, "operation": f"file({match.group(1)})", "paths": paths})
    return results


def load_trace(
    trace_path: Path,
    source_root: Path,
    normalizer: PathNormalizer,
    source_to_build: dict[str, Path],
    cmake_root: str | None,
) -> dict[str, Any]:
    custom_commands: list[dict[str, Any]] = []
    custom_targets: list[dict[str, Any]] = []
    cleanup_operations: list[dict[str, Any]] = []
    import_invocations: list[dict[str, Any]] = []
    excluded_system_custom = 0
    ordinals: collections.Counter[tuple[str, int, str]] = collections.Counter()

    trace_handle = (
        gzip.open(trace_path, mode="rt", encoding="utf-8")
        if trace_path.suffix == ".gz"
        else trace_path.open(encoding="utf-8")
    )
    with trace_handle as handle:
        for raw_line in handle:
            try:
                event = json.loads(raw_line)
            except json.JSONDecodeError:
                continue
            command = event.get("cmd")
            args = event.get("args", [])
            if not command or not isinstance(args, list):
                continue
            args = expand_control_tokens(args)
            file_value = event.get("file", "")
            definition_file = Path(file_value) if file_value else source_root / "CMakeLists.txt"
            location = {"file": normalizer.text(file_value), "line": event.get("line")}
            is_system = bool(cmake_root and file_value.startswith(cmake_root))

            if command in {"add_custom_command", "add_custom_target"}:
                if is_system:
                    excluded_system_custom += 1
                    continue
                key = (file_value, int(event.get("line", 0)), command)
                ordinals[key] += 1
                action_id = f"{command}:{normalizer.text(file_value)}:{event.get('line', 0)}:{ordinals[key]}"
                normalized_args = normalizer.text(args)
                command_sections = split_commands(args)
                working_sections = sections(args, "WORKING_DIRECTORY")
                working_directory = working_sections[0][0] if working_sections and working_sections[0] else None
                outputs: list[str] = []
                output_actual: list[Path] = []
                for group in sections(args, "OUTPUT"):
                    for value in group:
                        normalized, actual = resolve_observed_path(
                            value, working_directory, definition_file, source_to_build, normalizer
                        )
                        outputs.append(normalized)
                        if actual:
                            output_actual.append(actual)
                byproducts: list[str] = []
                byproduct_actual: list[Path] = []
                for group in sections(args, "BYPRODUCTS"):
                    for value in group:
                        normalized, actual = resolve_observed_path(
                            value, working_directory, definition_file, source_to_build, normalizer
                        )
                        byproducts.append(normalized)
                        if actual:
                            byproduct_actual.append(actual)
                source_for_policy = {"action": action_id, **location}
                for command_tokens in command_sections:
                    import_invocations.extend(extract_import_policy(command_tokens, normalizer, source_for_policy))
                    cleanup_operations.extend(cleanup_from_tokens(command_tokens, normalizer, source_for_policy))
                classification_name = args[0] if command == "add_custom_target" and args else action_id
                classification_source = file_value
                if "CrexxTestModes.cmake" in file_value or "CrexxSmokeTests.cmake" in file_value:
                    classification_source = str(source_root / "tests" / "CMakeLists.txt")
                classification = classify_surface(classification_source, classification_name, "UTILITY")
                entry = {
                    "id": action_id,
                    "kind": command,
                    "name": args[0] if command == "add_custom_target" and args else None,
                    "target_form": sections(args, "TARGET")[0][0] if sections(args, "TARGET") and sections(args, "TARGET")[0] else None,
                    "definition": location,
                    "classification": classification,
                    "all": "ALL" in args,
                    "outputs": sorted(set(outputs)),
                    "byproducts": sorted(set(byproducts)),
                    "depends": sorted(set(value for group in sections(args, "DEPENDS") for value in normalizer.text(group))),
                    "working_directory": normalizer.text(working_directory),
                    "commands": normalizer.text(command_sections),
                    "uses_terminal": "USES_TERMINAL" in args,
                    "job_pool": sections(args, "JOB_POOL")[0][0] if sections(args, "JOB_POOL") and sections(args, "JOB_POOL")[0] else None,
                    "raw_args": normalized_args,
                    "_actual_outputs": output_actual + byproduct_actual,
                }
                if command == "add_custom_command":
                    custom_commands.append(entry)
                else:
                    custom_targets.append(entry)
                continue

            if is_system:
                continue
            if command == "file" and args:
                if args[0] in {"REMOVE", "REMOVE_RECURSE"}:
                    cleanup_operations.append(
                        {"source": location, "operation": f"file({args[0]})", "paths": normalizer.text(args[1:])}
                    )
                if args[0] in {"GENERATE", "CONFIGURE"}:
                    content_sections = sections(args, "CONTENT")
                    for content_group in content_sections:
                        if not content_group:
                            continue
                        content = content_group[0]
                        cleanup_operations.extend(cleanup_from_content(content, normalizer, location))
                        import_invocations.extend(extract_imports_from_content(content, normalizer, location))

    return {
        "custom_commands": custom_commands,
        "custom_targets": custom_targets,
        "cleanup_operations": cleanup_operations,
        "import_invocations": import_invocations,
        "excluded_system_custom_definitions": excluded_system_custom,
    }


def load_ctest(path: Path, normalizer: PathNormalizer) -> list[dict[str, Any]]:
    data = read_json(path)
    graph = data.get("backtraceGraph", {})
    tests: list[dict[str, Any]] = []
    for test in data.get("tests", []):
        properties: dict[str, Any] = {}
        for item in test.get("properties", []):
            properties[item["name"]] = normalizer.text(item.get("value"))
        labels = properties.get("LABELS", [])
        if not isinstance(labels, list):
            labels = [str(labels)]
        definition = backtrace_location(graph, test.get("backtrace"), normalizer)
        source = definition.get("file", "")
        surface = classify_surface(source, test["name"])
        tier = classify_test(labels, test["name"])
        command = normalizer.text(test.get("command", []))
        tests.append(
            {
                "name": test["name"],
                "command": command,
                "definition": definition,
                "product_layer": surface["product_layer"],
                "wave": surface["wave"],
                "qa_tier": tier,
                "labels": sorted(labels),
                "fixtures_setup": properties.get("FIXTURES_SETUP", []),
                "fixtures_required": properties.get("FIXTURES_REQUIRED", []),
                "fixtures_cleanup": properties.get("FIXTURES_CLEANUP", []),
                "resource_locks": properties.get("RESOURCE_LOCK", []),
                "run_serial": bool(properties.get("RUN_SERIAL", False)),
                "timeout_seconds": properties.get("TIMEOUT"),
                "nested_build": "--build" in command and any("cmake" in part.lower() for part in command),
                "properties": properties,
            }
        )
    return sorted(tests, key=lambda item: item["name"])


def inventory_artifacts(
    file_api: dict[str, Any],
    trace: dict[str, Any],
    build_root: Path,
    normalizer: PathNormalizer,
) -> list[dict[str, Any]]:
    candidates: dict[str, tuple[Path, str]] = {}
    for target in file_api["targets"]:
        for actual in file_api["artifact_actual_paths"].get(target["id"], []):
            candidates[str(actual)] = (actual, f"target:{target['id']}")
    for action in trace["custom_commands"]:
        for actual in action.get("_actual_outputs", []):
            candidates.setdefault(str(actual), (actual, action["id"]))
    for relative_root in ("bin", "example-artifacts", "generated"):
        root = build_root / relative_root
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if path.is_file() and not path.is_symlink():
                candidates.setdefault(str(path), (path, f"published:{relative_root}"))

    inventory: list[dict[str, Any]] = []
    for path, owner in candidates.values():
        if not path.exists() or not path.is_file() or path.is_symlink():
            continue
        stat_result = path.stat()
        inventory.append(
            {
                "path": normalizer.path(path),
                "owner": owner,
                "size": stat_result.st_size,
                "sha256": sha256_file(path),
                "executable": bool(stat_result.st_mode & 0o111),
            }
        )
    return sorted(inventory, key=lambda item: item["path"])


def strip_internal_fields(value: Any) -> Any:
    if isinstance(value, list):
        return [strip_internal_fields(item) for item in value]
    if isinstance(value, dict):
        return {key: strip_internal_fields(item) for key, item in value.items() if not key.startswith("_")}
    return value


def build_findings(file_api: dict[str, Any], trace: dict[str, Any], tests: list[dict[str, Any]]) -> list[dict[str, Any]]:
    findings: list[dict[str, Any]] = []
    output_owners: dict[str, list[str]] = collections.defaultdict(list)
    for target in file_api["targets"]:
        for output in target["artifacts"]:
            output_owners[output].append(f"target:{target['id']}")
    for action in trace["custom_commands"]:
        for output in action["outputs"] + action["byproducts"]:
            output_owners[output].append(action["id"])
        if not action["outputs"] and not action["byproducts"] and not action["target_form"]:
            findings.append(
                {
                    "severity": "review",
                    "code": "custom-command-no-declared-output",
                    "subject": action["id"],
                    "detail": "Custom command has neither OUTPUT/BYPRODUCTS nor TARGET form.",
                }
            )
    for action in trace["custom_targets"]:
        if (
            action["commands"]
            and not action["byproducts"]
            and action["classification"]["basis"] != "qa-runner-target"
        ):
            findings.append(
                {
                    "severity": "review",
                    "code": "custom-target-no-declared-byproducts",
                    "subject": action["id"],
                    "detail": "Command-bearing custom target declares no BYPRODUCTS.",
                }
            )
    for output, owners in sorted(output_owners.items()):
        unique_owners = sorted(set(owners))
        if output and len(unique_owners) > 1:
            findings.append(
                {
                    "severity": "hazard",
                    "code": "multiple-output-owners",
                    "subject": output,
                    "owners": unique_owners,
                    "detail": "Observed output has more than one candidate owner.",
                }
            )
    for cleanup in trace["cleanup_operations"]:
        for path in cleanup["paths"]:
            owners = sorted(set(output_owners.get(path, [])))
            if owners:
                cleanup_action = cleanup.get("source", {}).get("action")
                own_output_only = bool(cleanup_action and owners == [cleanup_action])
                findings.append(
                    {
                        "severity": "review" if own_output_only else "hazard",
                        "code": "cleanup-rewrites-own-output" if own_output_only else "cleanup-touches-other-output",
                        "subject": path,
                        "owners": owners,
                        "source": cleanup["source"],
                        "detail": (
                            "An action deletes its own output before regeneration."
                            if own_output_only
                            else "A configured cleanup operation touches output owned by another action."
                        ),
                    }
                )
    for target in file_api["targets"]:
        if target["classification"]["basis"] == "fallback-review":
            findings.append(
                {
                    "severity": "review",
                    "code": "fallback-target-classification",
                    "subject": target["name"],
                    "detail": "Target uses the Phase 0 fallback classification and needs human review.",
                }
            )
    for action in trace["custom_commands"] + trace["custom_targets"]:
        if action["classification"]["basis"] == "fallback-review":
            findings.append(
                {
                    "severity": "review",
                    "code": "fallback-action-classification",
                    "subject": action["id"],
                    "detail": "Custom action uses the Phase 0 fallback classification and needs human review.",
                }
            )
    for test in tests:
        if test["nested_build"]:
            findings.append(
                {
                    "severity": "hazard",
                    "code": "test-invokes-build",
                    "subject": test["name"],
                    "detail": "CTest entry invokes cmake --build and can overlap the test worker pool.",
                }
            )
        if test["qa_tier"] == "measurement" and not test["run_serial"]:
            findings.append(
                {
                    "severity": "hazard",
                    "code": "measurement-not-run-serial",
                    "subject": test["name"],
                    "detail": "Performance measurement is not intrinsically isolated when broad CTest is run.",
                }
            )
        if not test["labels"]:
            findings.append(
                {
                    "severity": "review",
                    "code": "test-has-no-labels",
                    "subject": test["name"],
                    "detail": "CTest entry has no existing component/capability labels.",
                }
            )
    return sorted(findings, key=lambda item: (item["severity"], item["code"], item["subject"]))


def create_manifest(catalogue: dict[str, Any]) -> dict[str, Any]:
    actions: list[dict[str, Any]] = []
    target_action_ids = {target["id"]: f"target:{target['id']}" for target in catalogue["targets"]}
    for target in catalogue["targets"]:
        classification = target["classification"]
        actions.append(
            {
                "id": target_action_ids[target["id"]],
                "kind": "cmake-target",
                "product_layer": classification["product_layer"],
                "wave": classification["wave"],
                "qa_tier": classification["qa_tier"],
                "tool": "cmake --build --target",
                "argv": [target["name"]],
                "inputs": [item["path"] for item in target["sources"]],
                "outputs": target["artifacts"],
                "byproducts": [],
                "needs": [target_action_ids[item["id"]] for item in target["dependencies"] if item["id"] in target_action_ids],
                "import_policy": {"status": "not-observed", "source_roots": [], "binary_roots": [], "allowed_kinds": []},
                "metadata_policy": {"status": "not-observed", "required": [], "preserved": [], "stripped": []},
                "work_dir": target["build_dir"],
                "resources": {"cpu": 1, "memory_weight": 1, "io_weight": 1, "exclusive": []},
                "qa_tags": [],
                "observations": {
                    "definition": target["definition"],
                    "cmake_type": target["type"],
                    "classification_basis": classification["basis"],
                },
            }
        )
    for action in catalogue["custom_commands"] + catalogue["custom_targets"]:
        classification = action["classification"]
        import_records = [item for item in catalogue["import_invocations"] if item["source"].get("action") == action["id"]]
        source_roots = ordered_unique(root for item in import_records for root in item["source_roots"])
        binary_roots = ordered_unique(root for item in import_records for root in item["binary_roots"])
        allowed_kinds = ["source"] if source_roots else []
        if binary_roots:
            allowed_kinds.append("rxbin")
        if any(item["import_rxas"] for item in import_records):
            allowed_kinds.append("rxas")
        actions.append(
            {
                "id": f"observed:{action['id']}",
                "kind": action["kind"],
                "product_layer": classification["product_layer"],
                "wave": classification["wave"],
                "qa_tier": classification["qa_tier"],
                "tool": "observed-cmake-command",
                "argv": action["raw_args"],
                "inputs": action["depends"],
                "outputs": action["outputs"],
                "byproducts": action["byproducts"],
                "needs": [],
                "import_policy": {
                    "status": "observed" if import_records else "not-observed",
                    "source_roots": source_roots,
                    "binary_roots": binary_roots,
                    "allowed_kinds": sorted(set(allowed_kinds)),
                },
                "metadata_policy": {"status": "not-observed", "required": [], "preserved": [], "stripped": []},
                "work_dir": action["working_directory"] or "<UNRESOLVED>",
                "resources": {
                    "cpu": 1,
                    "memory_weight": 1,
                    "io_weight": 1,
                    "exclusive": ["console"] if action["uses_terminal"] else [],
                },
                "qa_tags": [],
                "observations": {
                    "definition": action["definition"],
                    "classification_basis": classification["basis"],
                    "cmake_job_pool": action["job_pool"],
                    "all": action["all"],
                    "target_form": action["target_form"],
                },
            }
        )
    tests = [
        {
            "id": f"test:{item['name']}",
            "name": item["name"],
            "product_layer": item["product_layer"],
            "wave": item["wave"],
            "qa_tier": item["qa_tier"],
            "labels": item["labels"],
            "fixtures_setup": item["fixtures_setup"],
            "fixtures_required": item["fixtures_required"],
            "fixtures_cleanup": item["fixtures_cleanup"],
            "resource_locks": item["resource_locks"],
            "run_serial": item["run_serial"],
            "timeout_seconds": item["timeout_seconds"],
            "observations": {"definition": item["definition"], "nested_build": item["nested_build"]},
        }
        for item in catalogue["tests"]
    ]
    return {
        "schema_version": MANIFEST_SCHEMA,
        "status": "observed-provisional-not-executable",
        "source_commit": catalogue["source_commit"],
        "configuration": catalogue["configuration"],
        "actions": sorted(actions, key=lambda item: item["id"]),
        "tests": sorted(tests, key=lambda item: item["id"]),
    }


def validate_manifest(manifest: dict[str, Any]) -> dict[str, Any]:
    schema_errors: list[str] = []
    graph_findings: list[dict[str, Any]] = []
    if manifest.get("schema_version") != MANIFEST_SCHEMA:
        schema_errors.append(f"schema_version must be {MANIFEST_SCHEMA}")
    actions = manifest.get("actions")
    tests = manifest.get("tests")
    if not isinstance(actions, list):
        schema_errors.append("actions must be an array")
        actions = []
    if not isinstance(tests, list):
        schema_errors.append("tests must be an array")
        tests = []
    action_ids: set[str] = set()
    action_by_id: dict[str, dict[str, Any]] = {}
    required_action_fields = {
        "id",
        "kind",
        "product_layer",
        "wave",
        "qa_tier",
        "tool",
        "argv",
        "inputs",
        "outputs",
        "byproducts",
        "needs",
        "import_policy",
        "metadata_policy",
        "work_dir",
        "resources",
        "qa_tags",
    }
    for index, action in enumerate(actions):
        if not isinstance(action, dict):
            schema_errors.append(f"actions[{index}] must be an object")
            continue
        missing = sorted(required_action_fields - set(action))
        if missing:
            schema_errors.append(f"actions[{index}] missing: {', '.join(missing)}")
        action_id = action.get("id")
        if not isinstance(action_id, str) or not action_id:
            schema_errors.append(f"actions[{index}].id must be a non-empty string")
            continue
        if action_id in action_ids:
            schema_errors.append(f"duplicate action id: {action_id}")
        action_ids.add(action_id)
        action_by_id[action_id] = action
        if action.get("product_layer") not in PRODUCT_LAYERS:
            schema_errors.append(f"{action_id}: invalid product_layer {action.get('product_layer')!r}")
        if action.get("qa_tier") not in QA_TIERS:
            schema_errors.append(f"{action_id}: invalid qa_tier {action.get('qa_tier')!r}")
        wave = action.get("wave")
        if not isinstance(wave, int) or not 0 <= wave <= 7:
            schema_errors.append(f"{action_id}: wave must be an integer from 0 through 7")
        for field in ("argv", "inputs", "outputs", "byproducts", "needs", "qa_tags"):
            if not isinstance(action.get(field), list):
                schema_errors.append(f"{action_id}: {field} must be an array")
    output_owners: dict[str, list[str]] = collections.defaultdict(list)
    for action in actions:
        if not isinstance(action, dict) or not isinstance(action.get("id"), str):
            continue
        for output in action.get("outputs", []) + action.get("byproducts", []):
            if output:
                output_owners[str(output)].append(action["id"])
        for dependency in action.get("needs", []):
            if dependency not in action_by_id:
                graph_findings.append(
                    {"severity": "error", "code": "unknown-dependency", "action": action["id"], "dependency": dependency}
                )
                continue
            dependency_wave = action_by_id[dependency].get("wave")
            if isinstance(dependency_wave, int) and isinstance(action.get("wave"), int) and dependency_wave > action["wave"]:
                graph_findings.append(
                    {
                        "severity": "review",
                        "code": "wave-inversion",
                        "action": action["id"],
                        "dependency": dependency,
                        "action_wave": action["wave"],
                        "dependency_wave": dependency_wave,
                    }
                )
    for output, owners in sorted(output_owners.items()):
        unique = sorted(set(owners))
        if len(unique) > 1:
            graph_findings.append(
                {"severity": "error", "code": "multiple-output-owners", "output": output, "owners": unique}
            )
    for index, test in enumerate(tests):
        if not isinstance(test, dict):
            schema_errors.append(f"tests[{index}] must be an object")
            continue
        if test.get("product_layer") not in PRODUCT_LAYERS:
            schema_errors.append(f"tests[{index}]: invalid product_layer")
        if test.get("qa_tier") not in QA_TIERS - {"none", "graph"}:
            schema_errors.append(f"tests[{index}]: invalid qa_tier")
    return {
        "schema_valid": not schema_errors,
        "schema_errors": schema_errors,
        "graph_clean": not any(item["severity"] == "error" for item in graph_findings),
        "graph_findings": graph_findings,
        "counts": {
            "actions": len(actions),
            "tests": len(tests),
            "schema_errors": len(schema_errors),
            "graph_findings": len(graph_findings),
        },
    }


def render_summary(catalogue: dict[str, Any], manifest_validation: dict[str, Any]) -> str:
    summary = catalogue["summary"]
    findings_by_code = counter_dict(item["code"] for item in catalogue["findings"])
    lines = [
        "# Current CMake Build Catalogue Summary",
        "",
        f"- Source commit: `{catalogue['source_commit']}`",
        f"- Platform: `{catalogue['platform']['system']} {catalogue['platform']['machine']}`",
        f"- Configuration: `{catalogue['configuration']['build_type']}`",
        "- Status: observation-only Phase 0 export; not an executable replacement graph",
        "",
        "## Inventory",
        "",
        "| Item | Count |",
        "| --- | ---: |",
    ]
    for key in (
        "targets",
        "custom_targets",
        "custom_commands",
        "declared_outputs_and_byproducts",
        "cleanup_operations",
        "rxc_import_invocations",
        "tests",
        "fixtures_setup",
        "fixtures_required",
        "resource_locks",
        "run_serial_tests",
        "nested_build_tests",
        "artifact_files",
    ):
        lines.append(f"| {key.replace('_', ' ')} | {summary[key]} |")
    lines.extend(["", "## Provisional product layers", "", "| Layer | Targets/actions |", "| --- | ---: |"])
    for key, value in summary["product_layers"].items():
        lines.append(f"| {key} | {value} |")
    lines.extend(["", "## Provisional QA tiers", "", "| Tier | Tests |", "| --- | ---: |"])
    for key, value in summary["qa_tiers"].items():
        lines.append(f"| {key} | {value} |")
    lines.extend(["", "## Findings", "", "| Code | Count |", "| --- | ---: |"])
    for key, value in findings_by_code.items():
        lines.append(f"| {key} | {value} |")
    lines.extend(
        [
            "",
            "## Manifest projection validation",
            "",
            f"- Schema valid: `{str(manifest_validation['schema_valid']).lower()}`",
            f"- Graph clean: `{str(manifest_validation['graph_clean']).lower()}`",
            f"- Schema errors: `{len(manifest_validation['schema_errors'])}`",
            f"- Graph findings: `{len(manifest_validation['graph_findings'])}`",
            "",
            "Graph findings describe the existing CMake projection and are Phase 1 inputs;",
            "they do not mean the Phase 0 exporter changed build behaviour.",
            "",
        ]
    )
    return "\n".join(lines)


def export_catalogue(args: argparse.Namespace) -> int:
    source_root = args.source.absolute()
    build_root = args.build.absolute()
    output_root = args.output.resolve()
    normalizer = PathNormalizer(source_root, build_root)
    file_api = load_file_api(source_root, build_root, normalizer)
    cmake_root = file_api["cmake"].get("paths", {}).get("root")
    trace = load_trace(args.trace, source_root, normalizer, file_api["source_to_build"], cmake_root)
    tests = load_ctest(args.ctest_json, normalizer)
    artifacts = inventory_artifacts(file_api, trace, build_root, normalizer)
    findings = build_findings(file_api, trace, tests)

    targets = file_api["targets"]
    custom_actions = trace["custom_commands"] + trace["custom_targets"]
    summary = {
        "targets": len(targets),
        "abstract_targets": len(file_api["abstract_targets"]),
        "custom_targets": len(trace["custom_targets"]),
        "custom_commands": len(trace["custom_commands"]),
        "excluded_system_custom_definitions": trace["excluded_system_custom_definitions"],
        "declared_outputs_and_byproducts": sum(
            len(item["outputs"]) + len(item["byproducts"]) for item in trace["custom_commands"]
        ),
        "cleanup_operations": len(trace["cleanup_operations"]),
        "rxc_import_invocations": len(trace["import_invocations"]),
        "tests": len(tests),
        "fixtures_setup": sum(bool(item["fixtures_setup"]) for item in tests),
        "fixtures_required": sum(bool(item["fixtures_required"]) for item in tests),
        "resource_locks": sum(bool(item["resource_locks"]) for item in tests),
        "run_serial_tests": sum(item["run_serial"] for item in tests),
        "nested_build_tests": sum(item["nested_build"] for item in tests),
        "artifact_files": len(artifacts),
        "artifact_bytes": sum(item["size"] for item in artifacts),
        "product_layers": counter_dict(
            [item["classification"]["product_layer"] for item in targets + custom_actions]
        ),
        "waves": counter_dict([item["classification"]["wave"] for item in targets + custom_actions]),
        "qa_tiers": counter_dict(item["qa_tier"] for item in tests),
        "existing_test_labels": counter_dict(label for item in tests for label in item["labels"]),
        "findings": counter_dict(item["severity"] for item in findings),
    }
    catalogue = {
        "schema_version": CATALOGUE_SCHEMA,
        "source_commit": args.source_commit,
        "platform": {
            "system": platform.system(),
            "release": platform.release(),
            "machine": platform.machine(),
        },
        "configuration": {
            "build_type": file_api["configuration"],
            "generator": file_api["cmake"].get("generator", {}).get("name"),
            "cmake_version": file_api["cmake"].get("version", {}).get("string"),
        },
        "directories": file_api["directories"],
        "abstract_targets": file_api["abstract_targets"],
        "targets": targets,
        "custom_commands": trace["custom_commands"],
        "custom_targets": trace["custom_targets"],
        "cleanup_operations": trace["cleanup_operations"],
        "import_invocations": trace["import_invocations"],
        "tests": tests,
        "artifacts": artifacts,
        "findings": findings,
        "summary": summary,
    }
    catalogue = strip_internal_fields(catalogue)
    manifest = create_manifest(catalogue)
    validation = validate_manifest(manifest)
    output_root.mkdir(parents=True, exist_ok=True)
    write_json(output_root / "catalogue.json", catalogue)
    write_json(output_root / "manifest-projection.json", manifest)
    write_json(output_root / "manifest-validation.json", validation)
    with (output_root / "catalogue-summary.md").open("w", encoding="utf-8", newline="\n") as handle:
        handle.write(render_summary(catalogue, validation))
    if validation["schema_errors"]:
        for error in validation["schema_errors"]:
            print(error, file=sys.stderr)
        return 1
    print(json.dumps(summary, sort_keys=True))
    return 0


def validate_command(args: argparse.Namespace) -> int:
    validation = validate_manifest(read_json(args.manifest))
    if args.output:
        write_json(args.output, validation)
    else:
        print(json.dumps(validation, indent=2, sort_keys=True))
    if not validation["schema_valid"]:
        return 1
    if args.strict and not validation["graph_clean"]:
        return 2
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    export_parser = subparsers.add_parser("export", help="export a catalogue and provisional manifest")
    export_parser.add_argument("--source", type=Path, required=True)
    export_parser.add_argument("--build", type=Path, required=True)
    export_parser.add_argument("--trace", type=Path, required=True)
    export_parser.add_argument("--ctest-json", type=Path, required=True)
    export_parser.add_argument("--source-commit", required=True)
    export_parser.add_argument("--output", type=Path, required=True)
    export_parser.set_defaults(func=export_catalogue)
    validate_parser = subparsers.add_parser("validate", help="validate a build manifest")
    validate_parser.add_argument("--manifest", type=Path, required=True)
    validate_parser.add_argument("--output", type=Path)
    validate_parser.add_argument("--strict", action="store_true", help="also fail on graph findings")
    validate_parser.set_defaults(func=validate_command)
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    try:
        return args.func(args)
    except CatalogueError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
