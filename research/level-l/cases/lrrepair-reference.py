#!/usr/bin/env python3
"""Independent insert/delete-distance oracle for a complete finite language."""
import itertools
from pathlib import Path


def distance(source, target):
    previous = list(range(len(target) + 1))
    for i, x in enumerate(source, 1):
        row = [i]
        for j, y in enumerate(target, 1):
            row.append(min(previous[j] + 1, row[j - 1] + 1,
                           previous[j - 1] + (0 if x == y else 2)))
        previous = row
    return previous[-1]


def main():
    accepted = ("a", "ab", "ba")
    rows = []
    for size in range(5):
        for letters in itertools.product("abc", repeat=size):
            source = "".join(letters)
            cost = min(distance(source, target) for target in accepted)
            rows.append((source.encode().hex() or "-") + " " + str(cost))
    path = Path(__file__).resolve().parents[1] / "evidence/lrrepair-finite-oracle.txt"
    path.write_text("\n".join(rows) + "\n")
    print(f"lrrepair_reference_cases={len(rows)}")


if __name__ == "__main__":
    main()
