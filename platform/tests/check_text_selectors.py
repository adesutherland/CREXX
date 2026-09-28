"""Exercise the shared external-text selector on maintained core CLIs."""
import argparse
import subprocess


def main():
    parser = argparse.ArgumentParser()
    for tool in ("rxc", "rxas", "rxlink", "rxdas", "rxbvm"):
        parser.add_argument("--" + tool, required=True)
    paths = parser.parse_args()
    for tool in ("rxc", "rxas", "rxlink", "rxdas", "rxbvm"):
        path = getattr(paths, tool)
        for page, expected in (("UTF8", 0), ("not-a-page", 1 if tool == "rxlink" else 2)):
            result = subprocess.run((path, "-E", page, "-h"),
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                    timeout=120)
            if result.returncode != expected:
                raise AssertionError(
                    f"{tool} -E {page}: {result.returncode}, expected {expected}\n"
                    f"stdout={result.stdout!r}\nstderr={result.stderr!r}")
            if page == "UTF8" and b"-E encoding" not in result.stdout:
                raise AssertionError(f"{tool} help omits the text selector")
    print("PASS: five core CLIs accept UTF8 and reject unsupported pages")


if __name__ == "__main__":
    main()
