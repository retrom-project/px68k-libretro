# Retrom PX68K host

The maintenance baseline is `uraraworks/px68k-libretro` commit
`561dcba6b11d04c9a6d7ca62998d5fb3f544aa49`; the upstream mirror remains `master`.
The maintenance branch and provenance are recorded in `retrom-fork.json`.

Retrom builds Musashi (`C68K=0`) into an isolated Emscripten ES module. The host
owns frame pacing, keyboard and two standard joypads, RGB video, stereo 44100 Hz
audio and in-memory checkpoints. `px68k_no_wait_mode=enabled` makes each host
step execute a guest frame without a second wall-clock scheduler. A logging
callback is required by the upstream controller and keyboard code.

The experimental WebX68k SCSI implementation requires its website's custom
JavaScript callbacks. The six storage files (`x68k/scsi.c`, `scsi.h`, `sasi.c`,
`sasi.h`, `mem_wrap.c`, `x68kmemory.h`) instead use libretro upstream commit
`0ad84d7058a12b7db4f7f7a906e87fad4e2f26f6`. WebX68k-specific debugger calls
are removed from the libretro entry point. No browser-specific callback shim
is silently substituted for storage behavior.

The initial Retrom target accepts a single DIM, XDF or HDF image. DIM/XDF
checkpoint sections include mounted image bytes, current track and sector;
the adapter checkpoint includes HDF filesystem bytes and machine state.
D88 and playlists are not exposed because their writable state and drive
switching are not yet covered by the same restore contract. BIOS files
`iplrom.dat` and `cgrom.dat` must be supplied by the host; they are not build assets.

Run `retrom/check.sh` for the deterministic bridge contract test. Build a local
candidate with `.github/rpg-runtime/build-candidate.sh <empty-absolute-directory>`
or the PFB's explicit `pfb-core-build CORE=px68k`. The pinned Emscripten image,
asset digests and source tree digest are recorded by this workflow. Product
acceptance belongs to Retrom's `ACC-PX68K-001` case.

Licensing is mixed. `COPYING`, inherited `doc/kero_src.txt` and the FMGen
notice retain their own terms, including the inherited noncommercial clause.
`LICENSES.txt` bundles their complete text; this work does not relicense those
components or establish commercial distribution permission. Local candidates
are not release authorization.
