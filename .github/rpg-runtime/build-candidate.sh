#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
output=${1:?absolute empty output directory required}
python3 "$root/.github/rpg-runtime/candidate_descriptor.py" prepare "$output"
mkdir -p "$root/.retrom-build"
docker run --rm --user "$(id -u):$(id -g)" \
  -e RETROM_CORE_REVISION="$(git -C "$root" rev-parse --short=12 HEAD)" \
  -e EM_CACHE=/src/.retrom-build/emscripten-cache \
  -v "$root:/src" -v "$output:/output" -w /src \
  emscripten/emsdk@sha256:90b757eb11fa9a0e3ce4d2d9f76d932a56018e4accc37b5a28b2783751e60eb7 \
  bash retrom/build-inside.sh
python3 "$root/retrom/licenses.py" "$output/LICENSES.txt"
python3 "$root/.github/rpg-runtime/candidate_descriptor.py" finalize "$output" --core-id px68k
