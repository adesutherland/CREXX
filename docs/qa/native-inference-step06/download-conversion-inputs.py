"""S6-D01 approved immutable checkpoint inputs; writes only to the supplied path.

Usage: python download-conversion-inputs.py FRESH_DIRECTORY
No model code is executed. Safetensors are chosen instead of pickle checkpoints.
"""
import hashlib
import json
from pathlib import Path
import sys
import urllib.request

root = Path(sys.argv[1]).resolve()
root.mkdir(parents=True, exist_ok=False)
models = (
    ("bge", "BAAI/bge-small-en-v1.5", "5c38ec7c405ec4b44b94cc5a9bb96e735b38267a",
     ("README.md", "config.json", "model.safetensors", "config_sentence_transformers.json",
      "modules.json", "sentence_bert_config.json", "1_Pooling/config.json",
      "special_tokens_map.json", "tokenizer.json", "tokenizer_config.json", "vocab.txt")),
    ("smol", "HuggingFaceTB/SmolLM2-360M-Instruct", "a10cc1512eabd3dde888204e902eca88bddb4951",
     ("README.md", "config.json", "model.safetensors", "generation_config.json",
      "special_tokens_map.json", "tokenizer.json", "tokenizer_config.json", "vocab.json", "merges.txt")),
)
manifest = []
for name, repo, revision, paths in models:
    api = f"https://huggingface.co/api/models/{repo}/tree/{revision}?recursive=true"
    with urllib.request.urlopen(api, timeout=60) as response:
        entries = json.load(response)
    (root / (name + "-tree.json")).write_text(json.dumps({"url": api, "entries": entries}, indent=2) + "\n")
    entries = {entry["path"]: entry for entry in entries if entry["type"] == "file"}
    for relative in paths:
        expected = entries[relative]
        url = f"https://huggingface.co/{repo}/resolve/{revision}/{relative}"
        dest = root / name / relative
        dest.parent.mkdir(parents=True, exist_ok=True)
        partial = dest.with_name(dest.name + ".part")
        digest = hashlib.sha256()
        git_blob = hashlib.sha1(f"blob {expected['size']}\0".encode())
        with urllib.request.urlopen(url, timeout=120) as response, partial.open("wb") as output:
            while chunk := response.read(1024 * 1024):
                output.write(chunk)
                digest.update(chunk)
                git_blob.update(chunk)
        assert partial.stat().st_size == expected["size"], relative
        if "lfs" in expected:
            assert digest.hexdigest() == expected["lfs"]["oid"], relative
        else:
            assert git_blob.hexdigest() == expected["oid"], relative
        partial.rename(dest)
        manifest.append({"model": name, "path": relative, "url": url,
                         "revision": revision, "sha256": digest.hexdigest(),
                         "size": dest.stat().st_size, "repository_entry": expected})
        (root / "inputs.json").write_text(json.dumps(manifest, indent=2) + "\n")
        print("Verified:", name, relative, digest.hexdigest(), flush=True)
print("PASS: immutable conversion inputs", flush=True)
