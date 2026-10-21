#!/usr/bin/env python3
"""Independent PUSH qualification over prepared compiler products."""
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'queue-insertion'))
from queue_instruction_checks import main

if __name__ == '__main__':
    main('PUSH', Path(__file__).resolve().parent)
