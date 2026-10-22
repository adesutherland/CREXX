#!/usr/bin/env python3
"""Independent QUEUE qualification, sharing only insertion-test orchestration."""
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'queue-insertion'))
from queue_instruction_checks import main

if __name__ == '__main__':
    main('QUEUE', Path(__file__).resolve().parent)
