"""Focused process-level ownership regression for the ordinary crexx driver."""

import argparse
import concurrent.futures
import pathlib
import subprocess
import tempfile
import time


SOURCE = """options levelb
import rxfnsb
import rxfs
import rxplatform
main: procedure = .int
  arg argv = .string[]
  marker = argv[1]
  gate = argv[2]
  label = argv[3]
  if charout(marker, "ready") <> 0 then return 3
  ignored = charout(marker)
  if gate <> "-" then do
    do tick = 1 to 800
      if rxfs..isfile(gate) then leave
      ignored = rxplatform..sleep(25)
    end
    if rxfs..isfile(gate) = 0 then return 4
  end
  say "LOCK PASS" label
  return 0
"""


def wait_for(path, process, seconds=30):
    deadline = time.monotonic() + seconds
    while not path.exists() and time.monotonic() < deadline:
        if process.poll() is not None:
            out, err = process.communicate()
            raise AssertionError(f"first driver exited early: {process.returncode}\n{out}\n{err}")
        time.sleep(0.025)
    assert path.exists(), f"timed out waiting for {path}"


def command(crexx, flags, marker, gate, label, source="same.crexx"):
    return [str(crexx), *flags, source, "--args", marker, gate, label]


def check_scenario(crexx, root, keep, source="same.crexx"):
    root.mkdir()
    (root / source).write_text(SOURCE, encoding="utf-8")
    flags = [] if keep else ["--nokeep"]
    first = subprocess.Popen(
        command(crexx, flags, "first.marker", "release.flag", "first", source),
        cwd=root, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    try:
        wait_for(root / "first.marker", first)
        image = root / "same.rxbin"
        original = image.read_bytes()
        refused = subprocess.run(
            command(crexx, [*flags, "--lock-timeout", "0"],
                    "refused.marker", "-", "refused", source),
            cwd=root, capture_output=True, text=True, timeout=30)
        assert refused.returncode != 0, f"busy output accepted: {refused.stdout}"
        assert "timed out waiting for output lock" in refused.stderr, refused.stderr
        assert not (root / "refused.marker").exists()
        assert image.read_bytes() == original, "contender changed live image"
        if keep:
            refused_reader = subprocess.run(
                command(crexx, ["--nocompile", "--lock-timeout", "0"],
                        "reader.marker", "-", "reader", source),
                cwd=root, capture_output=True, text=True, timeout=30)
            assert refused_reader.returncode != 0
            assert "timed out waiting for output lock" in refused_reader.stderr
            assert not (root / "reader.marker").exists()

        second = subprocess.Popen(
            command(crexx, flags, "second.marker", "-", "second", source),
            cwd=root, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        try:
            time.sleep(0.5)
            assert second.poll() is None, "second driver did not wait"
            assert not (root / "second.marker").exists(), "second program ran during first"
            (root / "release.flag").touch()
            first_out, first_err = first.communicate(timeout=30)
            second_out, second_err = second.communicate(timeout=30)
            assert first.returncode == 0, (first_out, first_err)
            assert second.returncode == 0, (second_out, second_err)
            assert "LOCK PASS first" in first_out, first_out
            assert "LOCK PASS second" in second_out, second_out
        finally:
            if second.poll() is None:
                second.kill()
                second.communicate()
    finally:
        (root / "release.flag").touch()
        if first.poll() is None:
            first.kill()
            first.communicate()

    assert (root / "same.crexx-driver.lock").is_file(), "lock file must persist"
    assert image.is_file() is keep, "--keep/--nokeep bytecode contract changed"
    assert (root / "same.rxas").is_file() is keep, "--keep/--nokeep RXAS contract changed"
    if keep:
        reused = subprocess.run(
            command(crexx, ["--nocompile"], "reused.marker", "-", "reused", source),
            cwd=root, capture_output=True, text=True, timeout=30)
        assert reused.returncode == 0, (reused.stdout, reused.stderr)
        assert "LOCK PASS reused" in reused.stdout


def check_compile_only(crexx, root):
    root.mkdir()
    source = 'options levelb\nmain: procedure = .int\n  return 0\n'
    for name in ("left", "right"):
        (root / f"{name}.crexx").write_text(source, encoding="utf-8")
    commands = ([str(crexx), "left.crexx", "right.crexx", "--noexec"],
                [str(crexx), "right.crexx", "left.crexx", "--noexec"])
    with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
        results = list(pool.map(
            lambda argv: subprocess.run(argv, cwd=root, capture_output=True,
                                        text=True, timeout=30), commands))
    for result in results:
        assert result.returncode == 0, (result.stdout, result.stderr)
    for name in ("left", "right"):
        assert (root / f"{name}.rxbin").is_file()
        assert (root / f"{name}.crexx-driver.lock").is_file()


def check_lock_error(crexx, root):
    root.mkdir()
    (root / "unavailable.crexx").write_text(SOURCE, encoding="utf-8")
    (root / "unavailable.crexx-driver.lock").mkdir()
    started = time.monotonic()
    result = subprocess.run(
        command(crexx, [], "marker", "-", "error", "unavailable.crexx"),
        cwd=root, capture_output=True, text=True, timeout=10)
    assert result.returncode != 0, result.stdout
    assert "cannot acquire output lock" in result.stderr, result.stderr
    assert time.monotonic() - started < 5, "I/O failure was treated as contention"
    assert not (root / "marker").exists()


def check_prebuilt(crexx, rxvme, root):
    root.mkdir()
    (root / "prebuilt.crexx").write_text(
        'options levelb\nmain: procedure = .int\n  say "PREBUILT PASS"\n  return 0\n',
        encoding="utf-8")
    built = subprocess.run([str(crexx), "prebuilt.crexx", "--noexec"],
                           cwd=root, capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, (built.stdout, built.stderr)

    def run(_):
        return subprocess.run([str(rxvme), "prebuilt"], cwd=root,
                              capture_output=True, text=True, timeout=30)

    with concurrent.futures.ThreadPoolExecutor(max_workers=8) as pool:
        results = list(pool.map(run, range(24)))
    for result in results:
        assert result.returncode == 0, (result.stdout, result.stderr)
        assert "PREBUILT PASS" in result.stdout, result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--crexx", type=pathlib.Path, required=True)
    parser.add_argument("--rxvme", type=pathlib.Path, required=True)
    parser.add_argument("--work", type=pathlib.Path, required=True)
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="driver output lock ", dir=args.work) as directory:
        root = pathlib.Path(directory)
        check_scenario(args.crexx, root / "keep", True)
        check_scenario(args.crexx, root / "nokeep", False)
        check_scenario(args.crexx, root / "rxpp", True, "same.rxpp")
        check_compile_only(args.crexx, root / "compile_only")
        check_lock_error(args.crexx, root / "lock_error")
        check_prebuilt(args.crexx, args.rxvme, root / "prebuilt")
    print("PASS: crexx ordinary output lock, cleanup, and prebuilt control")


if __name__ == "__main__":
    main()
