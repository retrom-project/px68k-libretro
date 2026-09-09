import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("release", ROOT / ".github/rpg-runtime/build-release.py")
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)
FORK = json.loads((ROOT / "retrom-fork.json").read_text())
COMMIT = "a" * 40

class ReleaseTests(unittest.TestCase):
    def test_rejects_wrong_identity_and_dirty_source(self):
        for repo, tag, commit in [("https://example.com/fork", "retrom-core-g561dcba6b11d-r1", COMMIT), (FORK["forkRepository"], "v1", COMMIT), (FORK["forkRepository"], "retrom-core-g561dcba6b11d-r1", "b" * 40)]:
            with patch.object(release, "git", return_value=COMMIT), self.assertRaises(ValueError):
                release.validate_identity(repo, tag, commit, FORK)
        with patch.object(release, "git", side_effect=[COMMIT, " M libretro.c"]), self.assertRaisesRegex(ValueError, "DIRTY"):
            release.validate_identity(FORK["forkRepository"], "retrom-core-g561dcba6b11d-r1", COMMIT, FORK)

    def test_records_only_verified_assets_and_rejects_tampering(self):
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp)
            files = []
            for name in sorted(FORK["releaseAssets"][:-1]):
                data = name.encode()
                (output / name).write_bytes(data)
                files.append({"filename": name, "sizeBytes": len(data), "sha256": hashlib.sha256(data).hexdigest()})
            candidate = {"commit": COMMIT, "dirty": False, "repository": FORK["forkRepository"], "adapterAbi": FORK["adapterAbi"], "files": files}
            (output / "retrom-core-candidate.json").write_text(json.dumps(candidate))
            (output / "px68k-retrom.wasm").write_bytes(b"tampered")
            with self.assertRaisesRegex(ValueError, "DIGEST"):
                release.finalize(output, FORK["forkRepository"], "retrom-core-g561dcba6b11d-r1", COMMIT, FORK)
            (output / "px68k-retrom.wasm").write_bytes(b"px68k-retrom.wasm")
            release.finalize(output, FORK["forkRepository"], "retrom-core-g561dcba6b11d-r1", COMMIT, FORK)
            metadata = json.loads((output / "rpg-runtime-release.json").read_text())
            self.assertEqual(metadata["files"], files)
            self.assertEqual(sorted(p.name for p in output.iterdir()), sorted(FORK["releaseAssets"]))
