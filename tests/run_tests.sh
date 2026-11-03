#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
cc=${CC:-cc}
"$cc" -std=c99 -Wall -Wextra -Wshadow -Wconversion -Werror \
  -Itests/stubs -Iinclude tests/gameplay_test.c src/world.c src/save.c \
  -o /tmp/abyss_gameplay_test
/tmp/abyss_gameplay_test
./tests/syntax_check.sh
rm -f /tmp/abyss_gameplay_test
