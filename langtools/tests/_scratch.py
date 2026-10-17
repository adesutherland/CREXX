"""Owned diagnostic workspace; deleted on success or failure, never archived."""
import atexit
from pathlib import Path
import tempfile

def scratch():
    owner = tempfile.TemporaryDirectory(prefix='crexx-level-l-check-')
    atexit.register(owner.cleanup)
    return Path(owner.name)/'work'
