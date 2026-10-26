#!/usr/bin/env python3

from __future__ import annotations

import json
import re
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

import cmake_catalogue as catalogue
import capture_build_graph


class CatalogueUnitTests(unittest.TestCase):
    def test_expands_only_control_leading_cmake_lists(self) -> None:
        args = [
            "OUTPUT",
            "member.rxbin",
            "COMMAND;/usr/bin/cmake;-E;rm;-f;library.rxbin",
            "COMMAND",
            "$<TARGET_FILE:rxc>",
            "-i",
            "/build/bin;/build/classlib",
        ]
        expanded = catalogue.expand_control_tokens(args)
        self.assertEqual(expanded[2:8], ["COMMAND", "/usr/bin/cmake", "-E", "rm", "-f", "library.rxbin"])
        self.assertEqual(expanded[-1], "/build/bin;/build/classlib")

    def test_sections_stop_at_expanded_control_keyword(self) -> None:
        args = catalogue.expand_control_tokens(
            ["OUTPUT", "member.rxbin", "COMMAND;/usr/bin/cmake;-E;rm;-f;library.rxbin", "DEPENDS", "member.crexx"]
        )
        self.assertEqual(catalogue.sections(args, "OUTPUT"), [["member.rxbin"]])
        self.assertEqual(catalogue.sections(args, "COMMAND"), [["/usr/bin/cmake", "-E", "rm", "-f", "library.rxbin"]])

    def test_path_normalizer_handles_resolved_tmp_alias(self) -> None:
        with tempfile.TemporaryDirectory() as root:
            source = Path(root) / "source"
            build = Path(root) / "build"
            source.mkdir()
            build.mkdir()
            normalizer = catalogue.PathNormalizer(source, build)
            self.assertEqual(normalizer.text(str(build / "bin" / "rxc")), "<BUILD>/bin/rxc")
            self.assertEqual(normalizer.text(str(build.resolve() / "bin" / "rxc")), "<BUILD>/bin/rxc")

    def test_classification_uses_product_and_test_paths(self) -> None:
        self.assertEqual(
            catalogue.classify_surface("<SOURCE>/lib/rxfnsb/rexx/CMakeLists.txt", "abs")["product_layer"],
            "B0",
        )
        result = catalogue.classify_surface("<SOURCE>/compiler/tests/CMakeLists.txt", "compiler_case", "UTILITY")
        self.assertEqual((result["product_layer"], result["wave"], result["qa_tier"]), ("Optional", 7, "comprehensive"))
        self.assertEqual(catalogue.classify_surface("<SOURCE>/lib/veclib/CMakeLists.txt", "veclib")["product_layer"], "B1")
        self.assertEqual(catalogue.classify_surface("<SOURCE>/lib/veclib/CMakeLists.txt", "generated-action")["product_layer"], "B1")
        self.assertEqual(catalogue.classify_surface("<SOURCE>/CMakeLists.txt", "concurrency-qa")["qa_tier"], "comprehensive")
        prep = catalogue.classify_surface("<SOURCE>/CMakeLists.txt", "qa-prep-linked-opt-runtime", "UTILITY")
        self.assertEqual((prep["product_layer"], prep["wave"], prep["qa_tier"]), ("Product", 7, "none"))
        self.assertEqual(prep["basis"], "qa-prep-target")
        qa_runner = catalogue.classify_surface("<SOURCE>/CMakeLists.txt", "qa-measurement", "UTILITY")
        self.assertEqual(
            (qa_runner["product_layer"], qa_runner["wave"], qa_runner["qa_tier"]),
            ("Optional", 7, "measurement"),
        )
        self.assertEqual(qa_runner["basis"], "qa-runner-target")
        aggregate = catalogue.classify_surface(
            "<SOURCE>/lib/rxfnsb/tests_functional/CMakeLists.txt",
            "linked_opt_runtime_artifacts",
            "UTILITY",
        )
        self.assertEqual((aggregate["product_layer"], aggregate["wave"]), ("Optional", 7))
        artifact = catalogue.classify_surface(
            "<SOURCE>/lib/rxfnsb/tests_functional/CMakeLists.txt",
            "ts_abs_noopt_artifact",
            "UTILITY",
        )
        self.assertEqual((artifact["product_layer"], artifact["wave"]), ("Optional", 7))
        test_executable = catalogue.classify_surface(
            "<SOURCE>/interpreter/CMakeLists.txt",
            "test_rxpa_concurrency",
            "EXECUTABLE",
        )
        self.assertEqual((test_executable["product_layer"], test_executable["wave"]), ("Optional", 7))
        functional_group = catalogue.classify_surface(
            "<SOURCE>/lib/rxfnsb/tests_functional/CMakeLists.txt",
            "ts_abs",
            "UTILITY",
        )
        self.assertEqual((functional_group["product_layer"], functional_group["wave"]), ("Optional", 7))
        fixture_executable = catalogue.classify_surface(
            "<SOURCE>/interpreter/CMakeLists.txt",
            "persistent_worker_executor-rxvml",
            "EXECUTABLE",
        )
        self.assertEqual((fixture_executable["product_layer"], fixture_executable["wave"]), ("Optional", 7))
        embedded_vm = catalogue.classify_surface(
            "<SOURCE>/interpreter/CMakeLists.txt", "rxvme", "EXECUTABLE"
        )
        self.assertEqual((embedded_vm["product_layer"], embedded_vm["wave"]), ("Product", 7))

    def test_classification_recognizes_named_build_stages(self) -> None:
        expected = {
            "stage-c0-native": ("C0", 1),
            "stage-c1-toolchain": ("C1", 2),
            "stage-b0-bootstrap": ("B0", 3),
            "stage-x-exits": ("X", 4),
            "stage-b1-substrate": ("B1", 5),
            "stage-c-rexx-tools": ("C", 6),
            "stage-g-library": ("G", 6),
            "stage-l-libraries": ("L", 6),
            "stage-product": ("Product", 7),
            "stage-optional": ("Optional", 7),
        }
        for target, classification in expected.items():
            with self.subTest(target=target):
                result = catalogue.classify_surface(
                    "<SOURCE>/CMakeLists.txt", target, "UTILITY"
                )
                self.assertEqual(
                    (result["product_layer"], result["wave"]), classification
                )
                self.assertEqual(result["basis"], "stage-target")

    @unittest.skipUnless(shutil.which("ninja"), "Private stage graph requires Ninja")
    def test_research_stage_prepares_selected_install_payload(self) -> None:
        root = Path(__file__).resolve().parents[3]
        stages = (root / "cmake/CrexxBuildStages.cmake").read_text()
        # Stub unrelated required leaves from the actual declarations. The
        # independent payload oracle below names installed research producers.
        required = set()
        for block in re.findall(r"crexx_add_build_stage\((stage-[^)]*)\)", stages):
            if "REQUIRED_TARGETS" in block:
                leaves = block.split("REQUIRED_TARGETS", 1)[1].split("OPTIONAL_TARGETS", 1)[0]
                required.update(name for name in leaves.split() if not name.startswith("stage-"))
        self.assertTrue(required)
        self.assertTrue(all(re.fullmatch(r"[A-Za-z0-9_-]+", name) for name in required))
        c_payload = {"llccompiler", "llctarget", "llcdynamic", "llrxc-c"}
        g_payload = {"llgcompiler", "llrxc-g"}
        with tempfile.TemporaryDirectory(prefix="crexx-stage-") as temporary:
            source = Path(temporary).resolve() / "source"
            source.mkdir()
            (source / "BuildStages.cmake").write_text(stages)
            (source / "CMakeLists.txt").write_text('''cmake_minimum_required(VERSION 3.24)
project(ResearchInstallStage NONE)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/prepared")
foreach(target IN LISTS STUB_TARGETS)
  add_custom_target(${target} COMMAND "${CMAKE_COMMAND}" -E touch
    "${CMAKE_BINARY_DIR}/prepared/${target}")
endforeach()
include("${CMAKE_SOURCE_DIR}/BuildStages.cmake")
if(TARGET stage-research-compilers)
  file(WRITE "${CMAKE_BINARY_DIR}/research-stage" "present")
endif()
''')
            for profile in ("off", "c", "g", "cg"):
                with self.subTest(profile=profile):
                    selected = (c_payload if "c" in profile else set()) | (g_payload if "g" in profile else set())
                    if profile != "off":
                        selected = selected | {"llcontract_runtime_package"}
                    build = source.parent / profile
                    def run(*arguments: str) -> None:
                        result = subprocess.run(["cmake", *map(str, arguments)], text=True,
                                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                        self.assertEqual(result.returncode, 0, result.stdout[-8000:])
                    run("-S", source, "-B", build, "-G", "Ninja",
                        "-DCREXX_BUILD_RESEARCH_COMPILERS=" + ("OFF" if profile == "off" else "ON"),
                        "-DSTUB_TARGETS=" + ";".join(sorted(required | selected)))
                    run("--build", build, "--target", "stage-product")
                    prepared = {path.name for path in (build / "prepared").iterdir()}
                    self.assertTrue(prepared)
                    self.assertFalse(prepared & (c_payload | g_payload | {"llcontract_runtime_package"}))
                    self.assertEqual((build / "research-stage").exists(), profile != "off")
                    if profile != "off":
                        run("--build", build, "--target", "stage-research-compilers")
                        prepared_after = {path.name for path in (build / "prepared").iterdir()}
                        self.assertEqual(prepared_after - prepared, selected)

    def test_qa_classification_recognizes_explicit_qualification(self) -> None:
        self.assertEqual(catalogue.classify_test(["qualification", "performance"], "candidate"), "qualification")

    def test_cmake_performance_tiers_preserve_correctness_qualification(self) -> None:
        root = Path(__file__).resolve().parents[3]
        with tempfile.TemporaryDirectory(prefix="crexx-qa-tiers-") as temporary:
            source = Path(temporary) / "source"
            build = Path(temporary) / "build"
            source.mkdir()
            (source / "CMakeLists.txt").write_text('''cmake_minimum_required(VERSION 3.24)
project(PerformanceQaTiers NONE)
enable_testing()
include("${TIERS_MODULE}")
add_test(NAME correctness COMMAND "${CMAKE_COMMAND}" -E true)
set_tests_properties(correctness PROPERTIES
  LABELS "performance;qualification" CREXX_PREP_TARGETS correctness_owner)
add_test(NAME timing_smoke COMMAND "${CMAKE_COMMAND}" -E true)
set_tests_properties(timing_smoke PROPERTIES
  LABELS "performance;smoke" CREXX_PREP_TARGETS timing_owner)
crexx_finalize_directory_qa_tiers()
get_property(qualification GLOBAL PROPERTY CREXX_QA_PREP_QUALIFICATION_TARGETS)
get_property(measurement GLOBAL PROPERTY CREXX_QA_PREP_MEASUREMENT_TARGETS)
file(WRITE "${CMAKE_BINARY_DIR}/prep-tiers.txt" "${qualification}\\n${measurement}\\n")
''', encoding="utf-8")
            configured = subprocess.run(
                ["cmake", "-S", str(source), "-B", str(build),
                 "-DTIERS_MODULE=" + str(root / "cmake/CrexxQaTiers.cmake")],
                text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            )
            self.assertEqual(configured.returncode, 0, configured.stdout[-8000:])
            discovery = subprocess.run(
                ["ctest", "--test-dir", str(build), "--show-only=json-v1"],
                text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            )
            self.assertEqual(discovery.returncode, 0, discovery.stdout[-8000:])
            tests = {test["name"]: {prop["name"]: prop["value"]
                                   for prop in test["properties"]}
                     for test in json.loads(discovery.stdout)["tests"]}
            qualification = tests["correctness"]
            self.assertIn("qualification", qualification["LABELS"])
            self.assertNotIn("performance-measurement", qualification["LABELS"])
            self.assertEqual(catalogue.classify_test(qualification["LABELS"], "correctness"), "qualification")
            measurement = tests["timing_smoke"]
            self.assertIn("performance-measurement", measurement["LABELS"])
            self.assertNotIn("smoke", measurement["LABELS"])
            self.assertTrue(measurement["RUN_SERIAL"])
            self.assertEqual(catalogue.classify_test(measurement["LABELS"], "timing_smoke"), "measurement")
            self.assertEqual((build / "prep-tiers.txt").read_text().splitlines(),
                             ["correctness_owner", "timing_owner"])


    def test_literal_performance_matrix_sources_exist(self) -> None:
        root = Path(__file__).resolve().parents[3]
        directory = root / "tests/performance"
        definitions = (directory / "CMakeLists.txt").read_text(encoding="utf-8")
        # The graph catalogue validates declared owners/dependencies, without
        # requiring filesystem existence for their source inputs. Check literal
        # workload owners before explicit measurement preparation reaches them.
        sources = re.findall(
            r"\bSOURCE\s+\$\{CMAKE_CURRENT_SOURCE_DIR\}/([^\s$]+\.crexx)\b",
            definitions,
        )
        self.assertTrue(sources)
        for source in sources:
            with self.subTest(source=source):
                self.assertTrue((directory / source).is_file(), source)


    def test_nested_build_detection_follows_cmake_scripts_but_allows_scratch_builds(self) -> None:
        with tempfile.TemporaryDirectory() as root:
            build = Path(root) / "build"
            build.mkdir()
            script = Path(root) / "qualification.cmake"
            script.write_text(
                'execute_process(COMMAND "${CMAKE_COMMAND}" --build "${CREXX_BUILD_DIR}" --target prereqs)\n',
                encoding="utf-8",
            )
            command = ["cmake", f"-DCREXX_BUILD_DIR={build}", "-P", str(script)]
            self.assertTrue(catalogue.ctest_invokes_active_build(command, build))

            script.write_text(
                'execute_process(COMMAND "${CMAKE_COMMAND}" --build "${consumer_build}")\n',
                encoding="utf-8",
            )
            self.assertFalse(catalogue.ctest_invokes_active_build(command, build))
            self.assertTrue(catalogue.ctest_invokes_active_build(["cmake", "--build", str(build)], build))

    def test_import_policy_accepts_direct_and_cmake_env_invocations(self) -> None:
        with tempfile.TemporaryDirectory() as root:
            base = Path(root)
            source = base / "source"
            build = base / "build"
            source.mkdir()
            build.mkdir()
            normalizer = catalogue.PathNormalizer(source, build)
            direct = catalogue.extract_import_policy(
                ["$<TARGET_FILE:rxc>", "-s", f"{source}/one;{source}/two", "-i", f"{build}/bin", "--no-exe-import"],
                normalizer,
                {"file": "test", "line": 1},
            )
            wrapped = catalogue.extract_import_policy(
                ["cmake", "-E", "env", "RXCP_DISABLE_EXIT=1", "$<TARGET_FILE:rxc>", "--import-rxas"],
                normalizer,
                {"file": "test", "line": 2},
            )
        self.assertEqual(direct[0]["source_roots"], ["<SOURCE>/one", "<SOURCE>/two"])
        self.assertEqual(direct[0]["binary_roots"], ["<BUILD>/bin"])
        self.assertFalse(direct[0]["exe_import"])
        self.assertTrue(wrapped[0]["import_rxas"])

    def test_import_policy_rejects_rxc_passed_to_another_tool(self) -> None:
        with tempfile.TemporaryDirectory() as root:
            base = Path(root)
            source = base / "source"
            build = base / "build"
            source.mkdir()
            build.mkdir()
            normalizer = catalogue.PathNormalizer(source, build)
            observed = catalogue.extract_import_policy(
                ["parser_tester", "-p", "$<TARGET_FILE:rxc>", "-s", f"{source}/program.crexx"],
                normalizer,
                {"file": "test", "line": 3},
            )
        self.assertEqual(observed, [])

    def test_ordered_unique_preserves_import_search_order(self) -> None:
        self.assertEqual(catalogue.ordered_unique(["second", "first", "second", "third"]), ["second", "first", "third"])

    def test_manifest_import_invocation_preserves_policy_and_visibility(self) -> None:
        invocation = catalogue.manifest_import_invocation(
            {
                "tool": "<BUILD>/bin/rxc",
                "source_roots": ["<SOURCE>/first", "<SOURCE>/second"],
                "binary_roots": ["<BUILD>/private"],
                "import_rxas": True,
                "exe_import": False,
            }
        )
        self.assertEqual(invocation["source_roots"], ["<SOURCE>/first", "<SOURCE>/second"])
        self.assertEqual(invocation["binary_roots"], ["<BUILD>/private"])
        self.assertEqual(invocation["allowed_kinds"], ["rxas", "rxbin", "rxplugin", "source"])
        self.assertTrue(invocation["auto_import_rxas"])
        self.assertEqual(invocation["executable_directory"], "excluded")
        self.assertIsNone(invocation["resolution_report"])
        self.assertEqual(invocation["expected_providers"], [])

    def test_manifest_validator_requires_report_and_provider_digest_for_declared_imports(self) -> None:
        policy = {
            "status": "declared",
            "source_roots": [],
            "binary_roots": ["<BUILD>/private"],
            "allowed_kinds": ["rxbin"],
            "invocations": [
                {
                    "tool": "<BUILD>/bin/rxc",
                    "source_roots": [],
                    "binary_roots": ["<BUILD>/private"],
                    "allowed_kinds": ["rxbin"],
                    "auto_import_rxas": False,
                    "executable_directory": "excluded",
                    "resolution_report": None,
                    "expected_providers": [
                        {
                            "request": "provider",
                            "kind": "rxbin",
                            "path": "<BUILD>/private/provider.rxbin",
                            "sha256": "not-a-digest",
                        }
                    ],
                }
            ],
        }
        errors: list[str] = []
        catalogue._validate_import_policy(policy, "action.import_policy", errors)
        self.assertTrue(any("resolution_report is required" in error for error in errors))
        self.assertTrue(any("lowercase SHA-256 digest" in error for error in errors))

    def test_manifest_validator_keeps_observed_v1_projection_compatible(self) -> None:
        observed = {
            "status": "observed",
            "source_roots": ["<SOURCE>/imports"],
            "binary_roots": [],
            "allowed_kinds": ["source"],
        }
        observed_errors: list[str] = []
        catalogue._validate_import_policy(observed, "observed.import_policy", observed_errors)
        self.assertEqual(observed_errors, [])

        declared = {**observed, "status": "declared"}
        declared_errors: list[str] = []
        catalogue._validate_import_policy(declared, "declared.import_policy", declared_errors)
        self.assertTrue(any("invocations is required" in error for error in declared_errors))

    def test_import_resolution_report_schema_has_stable_identity(self) -> None:
        schema = catalogue.read_json(Path(__file__).with_name("import-resolution-report.schema.json"))
        self.assertEqual(
            schema["properties"]["schema_version"]["const"],
            "crexx.import-resolution-report/v1",
        )

    def test_finds_non_verbatim_custom_target_with_trailing_dollar(self) -> None:
        custom_target = {
            "id": "add_custom_target:<SOURCE>/CMakeLists.txt:1:1",
            "commands": [["ctest", "--label-regex", "^essential$"]],
            "verbatim": False,
            "byproducts": [],
            "classification": {"basis": "qa-runner-target"},
        }
        findings = catalogue.build_findings(
            {"targets": []},
            {
                "custom_commands": [],
                "custom_targets": [custom_target],
                "cleanup_operations": [],
            },
            [],
        )
        self.assertEqual(
            [item["code"] for item in findings],
            ["non-verbatim-trailing-dollar"],
        )

        custom_target["verbatim"] = True
        self.assertEqual(
            catalogue.build_findings(
                {"targets": []},
                {
                    "custom_commands": [],
                    "custom_targets": [custom_target],
                    "cleanup_operations": [],
                },
                [],
            ),
            [],
        )

    def test_manifest_validator_detects_duplicate_output_owner(self) -> None:
        base_action = {
            "kind": "test",
            "product_layer": "C0",
            "wave": 1,
            "qa_tier": "none",
            "tool": "tool",
            "argv": [],
            "inputs": [],
            "outputs": ["<BUILD>/same"],
            "byproducts": [],
            "needs": [],
            "import_policy": {
                "status": "declared",
                "source_roots": [],
                "binary_roots": [],
                "allowed_kinds": [],
                "invocations": [],
            },
            "metadata_policy": {"status": "declared", "required": [], "preserved": [], "stripped": []},
            "work_dir": "<BUILD>/work",
            "resources": {"cpu": 1, "memory_weight": 1, "io_weight": 1, "exclusive": []},
            "qa_tags": [],
        }
        first = {**base_action, "id": "first"}
        second = {**base_action, "id": "second"}
        manifest = {
            "schema_version": catalogue.MANIFEST_SCHEMA,
            "status": "draft",
            "source_commit": "0" * 40,
            "configuration": {},
            "actions": [first, second],
            "tests": [],
        }
        validation = catalogue.validate_manifest(manifest)
        self.assertTrue(validation["schema_valid"])
        self.assertFalse(validation["graph_clean"])
        self.assertEqual(validation["graph_findings"][0]["code"], "multiple-output-owners")

    def test_time_parsers_normalize_peak_rss(self) -> None:
        darwin = "        1.25 real         2.50 user         0.50 sys\n  123456 maximum resident set size\n"
        linux = (
            "\tUser time (seconds): 4.0\n"
            "\tSystem time (seconds): 1.0\n"
            "\tElapsed (wall clock) time (h:mm:ss or m:ss): 0:05.50\n"
            "\tMaximum resident set size (kbytes): 2048\n"
        )
        self.assertEqual(capture_build_graph.parse_time_output(darwin, "Darwin")["maximum_resident_set_bytes"], 123456)
        parsed_linux = capture_build_graph.parse_time_output(linux, "Linux")
        self.assertEqual(parsed_linux["elapsed_seconds"], 5.5)
        self.assertEqual(parsed_linux["maximum_resident_set_bytes"], 2 * 1024 * 1024)

    def test_gzip_output_is_reproducible(self) -> None:
        with tempfile.TemporaryDirectory() as root:
            first = Path(root) / "first.txt"
            second = Path(root) / "second.txt"
            first.write_bytes(b"same contents\n")
            second.write_bytes(b"same contents\n")
            first_gzip = capture_build_graph.gzip_file(first)
            second_gzip = capture_build_graph.gzip_file(second)
            self.assertEqual(first_gzip.read_bytes(), second_gzip.read_bytes())


if __name__ == "__main__":
    unittest.main()
