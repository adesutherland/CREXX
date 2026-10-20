"""Verify that concurrent GLOBALV writers preserve both updates."""

from pathlib import Path
import os
import subprocess
import sys


def main():
    executable = sys.argv[1]
    environment = os.environ.copy()
    for variable in ("GLOBALV_HOME", "GLOBALV_SESSION"):
        Path(environment[variable]).mkdir(parents=True, exist_ok=True)

    subprocess.run([executable, "INIT"], env=environment, check=True,
                   capture_output=True, text=True)

    first = subprocess.Popen(
        [executable, "SELECT", "RACE", "SETL", "ALPHA", "one"],
        env=environment, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        text=True)
    second = subprocess.Popen(
        [executable, "SELECT", "RACE", "SETL", "BETA", "two"],
        env=environment, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        text=True)
    first_output, first_error = first.communicate(timeout=40)
    second_output, second_error = second.communicate(timeout=40)

    listing = subprocess.run(
        [executable, "SELECT", "RACE", "LIST"], env=environment,
        capture_output=True, text=True, check=True)

    if first.returncode != 0 or second.returncode != 0:
        raise RuntimeError(
            f"writer rc={first.returncode}/{second.returncode}; "
            f"stderr={first_error}{second_error}; "
            f"stdout={first_output}{second_output}")
    if "ALPHA" not in listing.stdout or "BETA" not in listing.stdout:
        raise RuntimeError("concurrent update was lost:\n" + listing.stdout)

    print("PASS: GLOBALV concurrent writers preserved both updates")


if __name__ == "__main__":
    main()
