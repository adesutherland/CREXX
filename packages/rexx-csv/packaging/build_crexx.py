#!/usr/bin/env python3
"""Build the cREXX-only rexx-csv source artifact; never upload it."""
import argparse
import gzip
import io
from pathlib import Path
import tarfile

ROOT = Path(__file__).resolve().parents[1]


def build(destination):
    members = {
        "rexx_csv.crexx": ROOT / "crexx/rexx_csv.crexx",
        "crexx_demo.crexx": ROOT / "examples/crexx_demo.crexx",
        "rexx-csv-README.md": ROOT / "packaging/README-crexx.md",
        "rexx-csv-SPEC.md": ROOT / "SPEC.md",
        "rexx-csv-LICENSE.txt": ROOT / "LICENSE",
    }
    with destination.open("xb") as output:
        with gzip.GzipFile(fileobj=output, filename="", mode="wb", mtime=0) as zipped:
            with tarfile.open(fileobj=zipped, mode="w", format=tarfile.USTAR_FORMAT) as archive:
                for name, source in sorted(members.items()):
                    data = source.read_bytes()
                    if name.endswith(".crexx"):
                        data = data.replace(b"../SPEC.md", b"rexx-csv-SPEC.md")
                        data = data.replace(b"../LICENSE", b"rexx-csv-LICENSE.txt")
                    info = tarfile.TarInfo(name)
                    info.size = len(data)
                    info.mode = 0o644
                    info.mtime = 0
                    archive.addfile(info, io.BytesIO(data))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path, help="new .rxpkg file, never overwritten")
    args = parser.parse_args()
    build(args.output)
    print("Created", args.output.resolve())
