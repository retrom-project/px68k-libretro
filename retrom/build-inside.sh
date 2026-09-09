#!/usr/bin/env bash
set -euo pipefail
emmake make -f Makefile.libretro platform=emscripten C68K=0 STATIC_LINKING=1 GIT_VERSION="$RETROM_CORE_REVISION" -j4
cp px68k_libretro_emscripten.bc .retrom-build/px68k.a
emcc retrom/bridge.c .retrom-build/px68k.a -I. -Ilibretro-common/include -Ilibretro -Ix68k \
  -O2 -o /output/px68k-retrom.mjs -sDEFAULT_TO_CXX=1 -sMODULARIZE=1 -sEXPORT_ES6=1 \
  -sALLOW_MEMORY_GROWTH=1 -sENVIRONMENT=web,node -sFILESYSTEM=1 \
  -sEXPORTED_FUNCTIONS=_malloc,_free,_retrom_abi,_retrom_load,_retrom_step,_retrom_key,_retrom_ready,_retrom_width,_retrom_height,_retrom_fps,_retrom_pixels,_retrom_audio,_retrom_audio_count,_retrom_state,_retrom_state_size,_retrom_restore,_retrom_stop \
  -sEXPORTED_RUNTIME_METHODS=FS,HEAPU8,HEAP16,stringToUTF8,lengthBytesUTF8
