"""Recompute maintained oracles without silently rewriting source fixtures."""
import sys

def write_fixture(path, text):
    if '--update' in sys.argv:
        path.parent.mkdir(parents=True,exist_ok=True)
        path.write_text(text)
    else:
        assert path.read_text() == text, f'Fixture differs: {path}; review before using --update'
