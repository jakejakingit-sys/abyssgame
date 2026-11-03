
NAME = ABYSS
DESCRIPTION = "Abyss - Dungeon Explorer"
COMPRESSED = YES
ARCHIVED = YES
OUTPUT_MAP = YES

# Disable LTO for the first reliable link.
# CEdev handles LTO internally; do not add -flto.
LTO = NO

# CEdev automatically includes src.
CFLAGS = -Wall -Wextra -Oz -Iinclude

include $(shell cedev-config --makefile)
