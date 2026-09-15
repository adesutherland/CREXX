"""Compare immutable GGUF metadata and tensor bytes, using the pinned reader.

Arguments: LLAMA_SOURCE APPROVED_MODELS CANDIDATE_MODELS OUTPUT_JSON.
This is artifact inspection, not a claim of original conversion ancestry.
"""
import hashlib
import json
from pathlib import Path
import sys

source, approved, candidates, output = [Path(p).resolve() for p in sys.argv[1:5]]
sys.path.insert(0, str(source / "gguf-py"))
from gguf import GGUFReader, GGUFValueType


def inspect(path):
    reader = GGUFReader(path)
    metadata = {}
    for key, field in reader.fields.items():
        value = field.contents()
        if field.types and field.types[0] == GGUFValueType.ARRAY:
            value = {"count": len(value), "sha256": hashlib.sha256(
                json.dumps(value, ensure_ascii=True, separators=(",", ":")).encode()).hexdigest()}
        metadata[key] = {"types": [v.name for v in field.types], "value": value}
    tensors = {tensor.name: {"shape": tensor.shape.tolist(), "type": tensor.tensor_type.name,
                            "bytes": tensor.n_bytes,
                            "sha256": hashlib.sha256(tensor.data.tobytes()).hexdigest()}
               for tensor in reader.tensors}
    return {"sha256": hashlib.sha256(path.read_bytes()).hexdigest(), "metadata": metadata, "tensors": tensors}


results = {}
for name in ("bge-small-en-v1.5-f16.gguf", "smollm2-360m-instruct-q8_0.gguf"):
    old, new = inspect(approved / name), inspect(candidates / name)
    row = {"approved": old, "candidate": new}
    for surface in ("metadata", "tensors"):
        row[surface + "_differences"] = {key: {"approved": old[surface].get(key), "candidate": new[surface].get(key)}
            for key in sorted(set(old[surface]) | set(new[surface])) if old[surface].get(key) != new[surface].get(key)}
    results[name] = row
    print(name, "changed metadata:", list(row["metadata_differences"]),
          "changed tensors:", len(row["tensors_differences"]), flush=True)
output.write_text(json.dumps(results, indent=2) + "\n")
