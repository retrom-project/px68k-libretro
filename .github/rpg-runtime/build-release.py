#!/usr/bin/env python3
"""Build a clean, fixed-toolchain PX68K release and hash every asset."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]

def git(*args):
    return subprocess.check_output(["git", *args], cwd=ROOT, text=True).strip()

def validate_identity(repository, tag, commit, fork):
    if repository != fork["forkRepository"]:
        raise ValueError("RELEASE_REPOSITORY_INVALID")
    if not re.fullmatch(fork["releaseTagPattern"], tag):
        raise ValueError("RELEASE_TAG_INVALID")
    if not re.fullmatch(r"[0-9a-f]{40}", commit) or commit != git("rev-parse", "HEAD"):
        raise ValueError("RELEASE_COMMIT_INVALID")
    if git("status", "--porcelain=v1"):
        raise ValueError("RELEASE_SOURCE_DIRTY")

def finalize(output, repository, tag, commit, fork):
    expected = sorted(set(fork["releaseAssets"]) - {"rpg-runtime-release.json"})
    if sorted(p.name for p in output.iterdir()) != sorted(expected + ["retrom-core-candidate.json"]):
        raise ValueError("RELEASE_ASSETS_INVALID")
    candidate = json.loads((output / "retrom-core-candidate.json").read_text())
    if candidate["commit"] != commit or candidate["dirty"] or candidate["repository"] != repository or candidate["adapterAbi"] != fork["adapterAbi"]:
        raise ValueError("RELEASE_CANDIDATE_INVALID")
    files = []
    for name in expected:
        path = output / name
        if path.is_symlink() or not path.is_file() or not path.stat().st_size:
            raise ValueError("RELEASE_ASSET_INVALID")
        files.append({"filename": name, "sizeBytes": path.stat().st_size, "sha256": hashlib.sha256(path.read_bytes()).hexdigest()})
    if files != candidate["files"]:
        raise ValueError("RELEASE_ASSET_DIGEST_INVALID")
    metadata = {"schemaVersion": 1, "repository": repository, "tag": tag, "commit": commit, "adapterAbi": fork["adapterAbi"], "upstreams": fork["upstreams"], "files": files}
    (output / "rpg-runtime-release.json").write_text(json.dumps(metadata, indent=2) + "\n")
    (output / "retrom-core-candidate.json").unlink()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--repository", required=True)
    parser.add_argument("--tag", required=True)
    parser.add_argument("--commit", required=True)
    args = parser.parse_args()
    fork = json.loads((ROOT / "retrom-fork.json").read_text())
    validate_identity(args.repository, args.tag, args.commit, fork)
    subprocess.run(["python3", str(ROOT / ".github/rpg-runtime/verify-source.py")], check=True)
    args.output.mkdir(parents=True, exist_ok=True)
    subprocess.run(["bash", str(ROOT / ".github/rpg-runtime/build-candidate.sh"), str(args.output.resolve())], check=True)
    validate_identity(args.repository, args.tag, args.commit, fork)
    finalize(args.output.resolve(), args.repository, args.tag, args.commit, fork)

if __name__ == "__main__":
    main()
