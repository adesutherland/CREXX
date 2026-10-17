#!/usr/bin/env python3
"""Qualify generated source against independent lexer oracles, without saved runs."""
import runpy
import sys
from pathlib import Path
if len(sys.argv) != 3:
    raise SystemExit('usage: check-source-emission.py BUILD OUTPUT')
sys.argv += ['--only', 'bounded-unicode', 'gennorm2', '--source']
runpy.run_path(str(Path(__file__).with_name('check-bootstrap.py')), run_name='__main__')
