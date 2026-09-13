import copy
import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch


SCRIPTS = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("assets", SCRIPTS / "windows-release-assets.py")
assets = importlib.util.module_from_spec(spec)
spec.loader.exec_module(assets)


class PublicationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.commit = "a" * 40
        self.source = dict(id=1, name="CREXX-dev-snapshot-windows-x64.zip", size=123,
                           digest="sha256:" + "1" * 64, updated_at="today")
        self.info = dict(id=10, assets=[copy.deepcopy(self.source)])
        self.state = dict(repo="owner/repo", tag="dev-snapshot", commit=self.commit,
                          release_id=10, asset=copy.deepcopy(self.source))
        self.next_id = 100
        self.after_upload = lambda: None
        self.after_patch = lambda: None
        self.calls = []
        self.addCleanup(patch.stopall)
        patch.object(assets, "api", self.api).start()
        patch.object(assets, "gh", self.gh).start()

    def api(self, path, *args):
        self.calls.append((path, args))
        if path.endswith("/git/ref/tags/dev-snapshot"):
            return dict(object=dict(type="commit", sha=self.commit))
        if path.endswith("/releases/tags/dev-snapshot"):
            return copy.deepcopy(self.info)
        asset_id = int(path.rsplit("/", 1)[1])
        if args[1] == "DELETE":
            self.info["assets"] = [a for a in self.info["assets"] if a["id"] != asset_id]
        elif args[1] == "PATCH":
            for item in self.info["assets"]:
                if item["id"] == asset_id:
                    item["name"] = args[3].removeprefix("name=")
            self.after_patch()
        else:
            self.fail(f"Unexpected API mutation: {args}")

    def gh(self, *args):
        self.assertEqual(args[:2], ("release", "upload"))
        path = Path(args[3])
        self.info["assets"].append(dict(id=self.next_id, name=path.name,
                                        digest=assets.digest(path)))
        self.next_id += 1
        self.after_upload()
        return ""

    def output(self, name="CREXX-dev-snapshot-windows-x64-signed-setup.exe"):
        path = self.root / name
        path.write_bytes(b"signed output fixture")
        return path

    def test_capture_and_payload_commit(self):
        state = assets.capture("owner/repo", "dev-snapshot", self.source["name"])
        self.assertEqual(state, self.state)
        (self.root / "BUILDINFO").write_bytes(f"commit={self.commit}\r\n".encode())
        assets.verify_payload(state, self.root)
        (self.root / "BUILDINFO").write_text("commit=old\n")
        with self.assertRaisesRegex(RuntimeError, "BUILDINFO"):
            assets.verify_payload(state, self.root)

    def test_replaced_source_same_name_is_rejected(self):
        self.info["assets"][0]["id"] = 2
        with self.assertRaisesRegex(RuntimeError, "changed"):
            assets.verify(self.state)

    def test_publish_both_preserves_unsigned_assets(self):
        output = self.output()
        signed_zip = self.output("CREXX-dev-snapshot-windows-x64-signed.zip")
        unsigned = dict(id=2, name="CREXX-dev-snapshot-windows-x64-unsigned-setup.exe")
        self.info["assets"].append(unsigned)
        assets.publish(self.state, [output, signed_zip])
        self.assertEqual({a["name"] for a in self.info["assets"]},
                         {self.source["name"], unsigned["name"], output.name, signed_zip.name})
        self.assertEqual(self.info["assets"][0], self.source)

    def test_tag_moves_during_upload_removes_only_own_uploads(self):
        old = dict(id=3, name=self.output().name)
        self.info["assets"].append(old)
        self.after_upload = lambda: setattr(self, "commit", "b" * 40)
        with self.assertRaisesRegex(RuntimeError, "changed"):
            assets.publish(self.state, [self.output()])
        self.assertEqual(self.info["assets"], [self.source, old])

    def test_lost_upload_response_cleans_unique_name(self):
        def fail():
            raise subprocess.CalledProcessError(1, "gh")
        self.after_upload = fail
        with self.assertRaises(subprocess.CalledProcessError):
            assets.publish(self.state, [self.output()])
        self.assertEqual(self.info["assets"], [self.source])

    def test_cleanup_does_not_delete_newer_asset_with_same_name(self):
        output = self.output()
        newer = dict(id=999, name=output.name, digest="newer")
        def replace():
            self.commit = "b" * 40
            self.info["assets"] = [self.source, newer]
        self.after_patch = replace
        with self.assertRaisesRegex(RuntimeError, "changed"):
            assets.publish(self.state, [output])
        self.assertEqual(self.info["assets"], [self.source, newer])

    def test_corrupt_upload_is_removed(self):
        self.after_upload = lambda: self.info["assets"][-1].update(digest="wrong")
        with self.assertRaisesRegex(RuntimeError, "digest mismatch"):
            assets.publish(self.state, [self.output()])
        self.assertEqual(self.info["assets"], [self.source])

    def test_source_cannot_be_overwritten(self):
        with self.assertRaisesRegex(RuntimeError, "source ZIP"):
            assets.publish(self.state, [self.output(self.source["name"])])
        self.assertEqual(self.calls, [])


class SigningTests(unittest.TestCase):
    def test_signs_pe_plugins_and_fails_on_signature_error(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for name in ("rxc.exe", "library.dll", "plugin.rxplugin", "README.md"):
                (root / name).write_text("fixture")
            harness = r'''
set -euo pipefail
source "$1/windows-signing-common.sh"
file() { case "$2" in *.exe|*.dll|*.rxplugin) echo 'PE32+ executable';; *) echo text;; esac; }
osslsigncode() { test -e "$3.verified"; }
jsign() {
  local target="${@: -1}"
  test "${FAIL_SIGN:-0}" != 1 || return 9
  touch "$target.verified"
}
sign_windows_payload "$2" provider alias timestamp
'''
            result = subprocess.run(["bash", "-c", harness, "bash", str(SCRIPTS), str(root)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertTrue((root / "plugin.rxplugin.verified").exists())
            self.assertFalse((root / "README.md.verified").exists())
            for marker in root.glob("*.verified"):
                marker.unlink()
            result = subprocess.run(["bash", "-c", "export FAIL_SIGN=1\n" + harness,
                                     "bash", str(SCRIPTS), str(root)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 9)


if __name__ == "__main__":
    unittest.main()
