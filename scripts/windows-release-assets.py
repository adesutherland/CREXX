#!/usr/bin/env python3
"""Pin a Windows release input and publish derived assets without stale uploads."""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import uuid


def gh(*args):
    return subprocess.check_output(["gh", *args], text=True)


def api(path, *args):
    output = gh("api", path, *args)
    return json.loads(output) if output.strip() else None


def digest(path):
    result = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            result.update(block)
    return "sha256:" + result.hexdigest()


def release(repo, tag):
    return api(f"repos/{repo}/releases/tags/{tag}")


def tag_commit(repo, tag):
    obj = api(f"repos/{repo}/git/ref/tags/{tag}")["object"]
    for _ in range(10):
        if obj["type"] == "commit":
            return obj["sha"]
        if obj["type"] != "tag":
            break
        obj = api(f"repos/{repo}/git/tags/{obj['sha']}")["object"]
    raise RuntimeError("Release tag does not resolve to a commit")


def named_asset(info, name):
    matches = [a for a in info["assets"] if a["name"] == name]
    if len(matches) != 1:
        raise RuntimeError(f"Expected exactly one release asset: {name}")
    return matches[0]


def capture(repo, tag, name):
    info = release(repo, tag)
    asset = named_asset(info, name)
    if not (asset.get("digest") or "").startswith("sha256:"):
        raise RuntimeError("The source ZIP has no GitHub SHA-256 digest")
    state = dict(repo=repo, tag=tag, commit=tag_commit(repo, tag),
                 release_id=info["id"], asset={k: asset[k] for k in
                 ("id", "name", "size", "digest", "updated_at")})
    verify(state)
    return state


def verify(state):
    info = release(state["repo"], state["tag"])
    current = named_asset(info, state["asset"]["name"])
    if (info["id"] != state["release_id"] or
            any(current.get(k) != v for k, v in state["asset"].items()) or
            tag_commit(state["repo"], state["tag"]) != state["commit"]):
        raise RuntimeError("Release changed during packaging/signing; rerun against the new snapshot")
    return info


def download(state, path):
    verify(state)
    with Path(path).open("wb") as stream:
        subprocess.run(["gh", "api",
                        f"repos/{state['repo']}/releases/assets/{state['asset']['id']}",
                        "-H", "Accept: application/octet-stream"], stdout=stream, check=True)
    if digest(path) != state["asset"]["digest"]:
        raise RuntimeError("Downloaded source ZIP does not match GitHub's SHA-256 digest")


def verify_payload(state, directory):
    fields = dict(line.split("=", 1) for line in
                  (Path(directory) / "BUILDINFO").read_text().splitlines() if "=" in line)
    if fields.get("commit") != state["commit"]:
        raise RuntimeError("ZIP BUILDINFO does not match the release tag commit")


def publish(state, paths):
    """Stage unique uploads, verify source again, then replace only named assets.

    If the tag/input moves, remove only the asset IDs uploaded by this invocation.
    Snapshot CI also invalidates signed assets after advancing its tag/payload.
    """
    paths = [Path(p).resolve() for p in paths]
    if len({p.name for p in paths}) != len(paths):
        raise RuntimeError("Duplicate output asset names")
    if state["asset"]["name"] in {p.name for p in paths}:
        raise RuntimeError("Derived assets must not overwrite the source ZIP")
    expected = {p.name: digest(p) for p in paths}
    repo, tag = state["repo"], state["tag"]
    prefix = f"repos/{repo}/releases/assets"
    owned = []
    staged_names = []
    success = False
    try:
        verify(state)
        with tempfile.TemporaryDirectory(prefix="crexx-sign-upload-") as temp:
            nonce = uuid.uuid4().hex
            for path in paths:
                staged = Path(temp) / f"{path.stem}-sign-{nonce}.tmp{path.suffix}"
                shutil.copyfile(path, staged)
                staged_names.append(staged.name)
                gh("release", "upload", tag, str(staged), "-R", repo)
                asset = named_asset(release(repo, tag), staged.name)
                owned.append(asset["id"])
                if asset.get("digest") != expected[path.name]:
                    raise RuntimeError(f"Uploaded digest mismatch: {path.name}")
            verify(state)
            for path, asset_id in zip(paths, owned):
                info = verify(state)
                for previous in info["assets"]:
                    if previous["name"] == path.name:
                        api(f"{prefix}/{previous['id']}", "--method", "DELETE")
                api(f"{prefix}/{asset_id}", "--method", "PATCH", "-f", f"name={path.name}")
            info = verify(state)
            for path, asset_id in zip(paths, owned):
                current = named_asset(info, path.name)
                if current["id"] != asset_id or current.get("digest") != expected[path.name]:
                    raise RuntimeError(f"Published asset verification failed: {path.name}")
            success = True
    finally:
        if not success:
            # Also recover an upload whose network response was lost. Unique names
            # and IDs prevent deleting assets published by another signing run.
            try:
                for item in release(repo, tag)["assets"]:
                    if item["id"] in owned or item["name"] in staged_names:
                        api(f"{prefix}/{item['id']}", "--method", "DELETE")
            except (RuntimeError, subprocess.CalledProcessError) as error:
                print(f"Could not clean up this signing run's uploads: {error}")
                raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    command = sub.add_parser("capture")
    for name in ("repo", "tag", "asset", "state"):
        command.add_argument("--" + name, required=True)
    for name in ("download", "verify", "payload", "publish"):
        command = sub.add_parser(name)
        command.add_argument("--state", required=True)
        if name in ("download", "payload"):
            command.add_argument("path")
        if name == "publish":
            command.add_argument("paths", nargs="+")
    args = parser.parse_args()
    if args.command == "capture":
        state = capture(args.repo, args.tag, args.asset)
        Path(args.state).write_text(json.dumps(state, indent=2) + "\n")
        print(f"Pinned {args.repo}@{args.tag}: {state['commit']} / asset {state['asset']['id']}")
        return
    state = json.loads(Path(args.state).read_text())
    if args.command == "download":
        download(state, args.path)
    elif args.command == "verify":
        verify(state)
    elif args.command == "payload":
        verify_payload(state, args.path)
    else:
        publish(state, args.paths)


if __name__ == "__main__":
    main()
