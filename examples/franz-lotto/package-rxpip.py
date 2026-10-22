#!/usr/bin/env python3
"""Build the source-only crexx-lotto RXPIP artifact from its owning sources."""
import argparse
import gzip
import io
from pathlib import Path
import tarfile


def build_package(destination: Path) -> None:
    root = Path(__file__).resolve().parent
    members = {
        "lotto.crexx": root / "crexx/lotto.crexx",
        "lotto_logic.crexx": root / "crexx/lotto_logic.crexx",
        "lotto_tests.crexx": root / "crexx/lotto_tests.crexx",
        "lotto-README.md": root / "rxpip/lotto-README.md",
        "lotto-LICENSE.txt": root / "LICENSE.txt",
    }
    # The original relative licence link describes the repository layout.
    # Adjust only that comment for RXPIP's flat layout; keep all code/tags.
    with destination.open("xb") as output:
        with gzip.GzipFile(fileobj=output, mode="wb", filename="", mtime=0) as zipped:
            with tarfile.open(fileobj=zipped, mode="w", format=tarfile.USTAR_FORMAT) as archive:
                for name, source in sorted(members.items()):
                    data = source.read_bytes()
                    if name.endswith(".crexx"):
                        data = data.replace(b"see ../LICENSE.txt", b"see lotto-LICENSE.txt")
                    info = tarfile.TarInfo(name)
                    info.size = len(data)
                    info.mode = 0o644
                    info.mtime = 0
                    archive.addfile(info, io.BytesIO(data))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path, help="new .rxpkg file (never overwritten)")
    args = parser.parse_args()
    build_package(args.output)
    print(f"Created {args.output.resolve()}")
