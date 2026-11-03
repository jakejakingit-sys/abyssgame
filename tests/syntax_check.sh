#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
cc=${CC:-cc}
"$cc" -std=c99 -Wall -Wextra -Wshadow -Wconversion -Werror \
  -fsyntax-only -Itests/stubs -Iinclude \
  src/main.c src/world.c src/save.c src/render.c
echo "ABYSS source syntax check: PASS (host stubs; not a CEdev build)"
