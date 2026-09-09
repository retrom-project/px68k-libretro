# PX68K Retrom fork

`master` mirrors uraraworks/px68k-libretro at the fixed commit in `retrom-fork.json`.
Retrom changes belong on `retrom/g561dcba6b11d`, through short feature branches and squash PRs. Do not merge into the upstream mirror or add merge commits after the baseline.

Run `python3 .github/rpg-runtime/verify-source.py`, `bash retrom/check.sh`, and `python3 -m unittest discover -s retrom/tests -p 'test_*.py'` before pushing. Build WebAssembly with `.github/rpg-runtime/build-candidate.sh` in the same PFB; the pinned Emscripten image is the supported toolchain.

Release immutable annotated tags matching `retrom-core-g561dcba6b11d-rN` from the maintenance branch. The release workflow is the supported publisher. It verifies tag ancestry and builds the closed asset set recorded in `retrom-fork.json`, including complete inherited license notices. Do not publish games, firmware, local paths or test saves. Preserve upstream license restrictions and provenance; do not relicense inherited code.

Retrom's host owns UI, audio output, filesystem inputs and scheduling. A physical gamepad button maps to one native action only. Direction and confirm are required; cancellation is optional. Never also synthesize Enter/Escape for a native action. Physical keyboard input remains independent, and host UI Back is unaffected.
