"""Normal installed-SDK/native-object proof. No sanitizer or model workload.

Usage: python3 check_native_objects_package.py BUILD SOURCE FRESH_WORK
Prepare stage-c1-toolchain, stage-product and stage-optional in BUILD first. Every write,
including installation, stays under a newly created FRESH_WORK directory.
"""
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
import sys

build, source, work = [Path(p).resolve() for p in sys.argv[1:]]
work.mkdir(parents=True, exist_ok=False)
prefix = work / "installed Ω"
sdk = work / "SDK source"
sdk.mkdir()
sdk_build = work / "SDK build"
suffix = ".exe" if os.name == "nt" else ""
env = dict(os.environ, CREXX_HOME=str(prefix))
clean = dict(env)
for key in ("CREXX_HOME", "CREXX_PROVIDER_PATH", "LD_LIBRARY_PATH",
            "DYLD_LIBRARY_PATH", "DYLD_FALLBACK_LIBRARY_PATH"):
    clean.pop(key, None)
number = 0


def run(label, argv, marker=None, environment=None, cwd=None):
    global number
    number += 1
    result = subprocess.run(list(map(str, argv)), cwd=cwd or work,
                            env=environment or env, capture_output=True,
                            text=True, timeout=300)
    output = result.stdout + result.stderr
    (work / f"{number:02}-{label}.log").write_text(
        f"argv={argv!r}\nrc={result.returncode}\n{output}")
    assert result.returncode == 0, (label, output[-4000:])
    assert not any(x in output for x in ("FAIL:", "PANIC:", "ERROR:")), (label, output[-4000:])
    if marker:
        assert marker in output, (label, output[-4000:])
    print(f"PASS: {label}", flush=True)


run("install", ["cmake", "--install", build, "--prefix", prefix])
for name in ("rxpa_objects.c", "rxpa_objects_cpp.cpp"):
    shutil.copy2(source / "tests/rxpa" / name, sdk / name)
(sdk / "CMakeLists.txt").write_text('''cmake_minimum_required(VERSION 3.24)
project(native_objects_sdk C CXX)
find_package(CREXX CONFIG REQUIRED)
set(CREXX_RXPA_PLUGIN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugin")
add_dynamic_plugin_target(_rxpa_objects rxpa_objects.c)
add_static_plugin_target(_rxpa_objects rxpa_objects.c)
add_decl_plugin_target(_rxpa_objects rxpa_objects.c)
add_rxpa_provider_package(_rxpa_objects OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/providers")
foreach(mode IN ITEMS normal decl)
    add_library(cpp_${mode} OBJECT rxpa_objects_cpp.cpp)
    target_link_libraries(cpp_${mode} PRIVATE CREXX::RXPA)
    target_compile_features(cpp_${mode} PRIVATE cxx_std_17)
endforeach()
target_compile_definitions(cpp_decl PRIVATE DECL_ONLY)
''')
run("sdk-configure", ["cmake", "-G", "Ninja", "-S", sdk, "-B", sdk_build,
                      f"-DCMAKE_PREFIX_PATH={prefix}", "-DCMAKE_BUILD_TYPE=Release"])
run("sdk-build", ["cmake", "--build", sdk_build, "--parallel", "4"])
shutil.copy2(sdk_build / "plugin/rx_rxpa_objects.rxplugin", prefix / "bin")
for artifact in (sdk_build / "providers").iterdir():
    shutil.copy2(artifact, prefix / "bin/providers" / artifact.name)
records = []
for fixture in ("rxpa_objects", "rxpa_objects_class_only", "rxpa_objects_workers", "rxpa_objects_text_worker"):
    consumer = work / (fixture + ".crexx")
    shutil.copy2(source / "tests/rxpa" / consumer.name, consumer)
    program = work / (fixture + "-dynamic")
    run(fixture + "-compile", [prefix / ("bin/rxc" + suffix), "--no-exe-import",
        "-i", prefix / "bin", "-o", program, consumer])
    run(fixture + "-assemble", [prefix / ("bin/rxas" + suffix), "-o", program, program])
    linked = str(program) + "-linked"
    run(fixture + "-link", [prefix / ("bin/rxlink" + suffix), "-o", linked, program,
        prefix / "bin/library", prefix / "bin/classlib", prefix / "bin/rxfnsg"])
    for engine in ("rxbvm", "rxtvm"):
        vm = prefix / "bin" / (engine + suffix)
        if engine == "rxtvm" and not vm.exists():
            continue
        run(fixture + "-" + engine, [vm, linked], "PASS: C native", environment=clean)
    native = work / (fixture + " native Ω") / "consumer"
    native.parent.mkdir()
    run(fixture + "-native-build", [prefix / ("bin/crexx" + suffix), "--program", native,
        consumer, "--jobs", "1", "--native"], "PUBLISHED: native program")
    executable = native.with_suffix(suffix) if suffix else native
    relocated = work / (fixture + " relocated Å")
    relocated.mkdir()
    shutil.copy2(executable, relocated / executable.name)
    # This fixture has no extra dynamic engine dependencies. Running only the
    # executable also proves it cannot fall back to the original provider DSO.
    run(fixture + "-relocated", [relocated / executable.name], "PASS: C native",
        environment=clean, cwd=relocated)
    records.append({"fixture": fixture, "executable": str(relocated / executable.name),
                    "sha256": hashlib.sha256(executable.read_bytes()).hexdigest()})
(work / "packages.json").write_text(json.dumps(records, indent=2) + "\n")
print("PASS: installed C/C++ SDK and relocated C native object consumers")
