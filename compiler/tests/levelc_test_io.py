"""Keep UTF-8 assembly and native text stdout separate from payload assertions."""
import os


def native_stdout(expected, windows=None):
    """Model the existing Windows CRT text stream without normalizing results."""
    if windows is None:
        windows = os.name == "nt"
    return expected.replace(b"\n", b"\r\n") if windows else expected
