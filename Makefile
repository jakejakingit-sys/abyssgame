
NAME = ABYSS
DESCRIPTION = "Abyss - Dungeon Explorer"
COMPRESSED = YES
ARCHIVED = YES
OUTPUT_MAP = YES

CFLAGS = -Wall -Wextra -Oz -flto
CFLAGS += -Iinclude -Isrc

include $(shell cedev-config --makefile)

