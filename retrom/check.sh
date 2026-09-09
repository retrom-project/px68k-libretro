#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p .retrom-build
cc -std=c11 -Wall -Wextra -Werror -I. -Ilibretro-common/include -Ilibretro -Ix68k retrom/tests/bridge_test.c -o .retrom-build/bridge-test
.retrom-build/bridge-test
cc -std=c11 -ffunction-sections -fdata-sections -I. -Ilibretro -Ilibretro-common/include -Ix68k retrom/tests/disk_state_test.c -Wl,--gc-sections -o .retrom-build/disk-state-test
.retrom-build/disk-state-test
