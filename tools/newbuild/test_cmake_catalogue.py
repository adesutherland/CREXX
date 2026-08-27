#!/usr/bin/env python3

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

import cmake_catalogue as catalogue
import capture_phase0


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

    def test_qa_classification_recognizes_explicit_qualification(self) -> None:
        self.assertEqual(catalogue.classify_test(["qualification", "performance"], "candidate"), "qualification")

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
            "import_policy": {"status": "declared", "source_roots": [], "binary_roots": [], "allowed_kinds": []},
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
        self.assertEqual(capture_phase0.parse_time_output(darwin, "Darwin")["maximum_resident_set_bytes"], 123456)
        parsed_linux = capture_phase0.parse_time_output(linux, "Linux")
        self.assertEqual(parsed_linux["elapsed_seconds"], 5.5)
        self.assertEqual(parsed_linux["maximum_resident_set_bytes"], 2 * 1024 * 1024)

    def test_gzip_output_is_reproducible(self) -> None:
        with tempfile.TemporaryDirectory() as root:
            first = Path(root) / "first.txt"
            second = Path(root) / "second.txt"
            first.write_bytes(b"same contents\n")
            second.write_bytes(b"same contents\n")
            first_gzip = capture_phase0.gzip_file(first)
            second_gzip = capture_phase0.gzip_file(second)
            self.assertEqual(first_gzip.read_bytes(), second_gzip.read_bytes())


if __name__ == "__main__":
    unittest.main()
