#!/usr/bin/env python3
"""Verify the fixed PX68K engine and storage provenance."""
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
BASELINE = "561dcba6b11d04c9a6d7ca62998d5fb3f544aa49"

def main():
    fork = json.loads((ROOT / "retrom-fork.json").read_text())
    assert fork["schemaVersion"] == 1
    assert fork["forkRepository"] == "https://github.com/retrom-project/px68k-libretro"
    assert fork["defaultBranch"] == "retrom/g561dcba6b11d"
    assert fork["upstreamMirrorBranch"] == "master"
    assert fork["adapterAbi"] == "px68k-host-v1"
    assert fork["upstreams"] == [
        {"role": "engine", "repository": "https://github.com/uraraworks/px68k-libretro", "refType": "COMMIT", "ref": BASELINE, "commit": BASELINE},
        {"role": "storage", "repository": "https://github.com/libretro/px68k-libretro", "refType": "COMMIT", "ref": "0ad84d7058a12b7db4f7f7a906e87fad4e2f26f6", "commit": "0ad84d7058a12b7db4f7f7a906e87fad4e2f26f6"},
    ]
    assert fork["releaseAssets"] == ["px68k-retrom.mjs", "px68k-retrom.wasm", "LICENSES.txt", "rpg-runtime-release.json"]
    revision = "HEAD^2" if os.environ.get("GITHUB_EVENT_NAME") == "pull_request" else "HEAD"
    subprocess.run(["git", "merge-base", "--is-ancestor", BASELINE, revision], cwd=ROOT, check=True)
    assert not subprocess.check_output(["git", "rev-list", "--min-parents=2", f"{BASELINE}..{revision}"], cwd=ROOT).strip()
    print("PX68K fork source contract: ok")

if __name__ == "__main__":
    main()
