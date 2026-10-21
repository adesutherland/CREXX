#!/usr/bin/env python3
"""Independent PUSH qualification over prepared compiler products."""
from pathlib import Path
from queue_instruction_checks import main

if __name__ == '__main__':
    main('PUSH', Path(__file__).resolve().parent)
