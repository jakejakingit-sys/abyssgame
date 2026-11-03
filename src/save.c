#include <string.h>
#include <stddef.h>
#include <fileioc.h>
#include "abyss.h"
#include "renderer.h"

SaveData g_save;

uint16_t save_checksum(const SaveData *data) {
    const uint8_t *bytes = (const uint8_t *)data;
    uint16_t sum = 0x5A3C;
    size_t i;
    /* Use the checksum field offset, not sizeof(struct): target ABIs may
       add tail padding after the final uint16_t. */
    for (i = 0; i < offsetof(SaveData, checksum); ++i) {
        sum = (uint16_t)((sum << 3) | (sum >> 13));
        sum = (uint16_t)(sum ^ bytes[i]);
    }
    return sum;
}

void save_reset(void) {
    memset(&g_save, 0, sizeof(g_save));
    g_save.magic = SAVE_MAGIC;
    g_save.reserved[0] = SAVE_VERSION;
    g_save.unlocked_level = 0;
    g_save.ability_mask = 1u; /* Echo / interact is available from the start. */
    g_save.checksum = save_checksum(&g_save);
}

void save_load(void) {
    uint8_t handle;
    SaveData candidate;
    save_reset();
    handle = ti_Open(SAVE_APPVAR, "r");
    if (!handle) return;
    if (ti_Read(&candidate, sizeof(candidate), 1, handle) == 1) {
        if (candidate.magic == SAVE_MAGIC && candidate.reserved[0] == SAVE_VERSION &&
            candidate.checksum == save_checksum(&candidate) &&
            candidate.unlocked_level < LEVEL_COUNT &&
            candidate.armor_level <= 5 && candidate.reach_level <= 5 &&
            candidate.hp_level <= 5) {
            g_save = candidate;
        }
    }
    ti_Close(handle);
}

void save_write(void) {
    uint8_t handle;
    g_save.magic = SAVE_MAGIC;
    g_save.checksum = save_checksum(&g_save);
    handle = ti_Open(SAVE_APPVAR, "w");
    if (!handle) {
        g_game.save_warning = 1;
        return;
    }
    if (ti_Write(&g_save, sizeof(g_save), 1, handle) != 1) {
        ti_Close(handle);
        g_game.save_warning = 1;
        return;
    }
    /* AppVars are writable in RAM. Archive only after writing, and let
       fileioc restore the LCD around an OS garbage-collection prompt. */
    if (!ti_ArchiveHasRoom(sizeof(g_save)) || !ti_SetArchiveStatus(true, handle)) {
        g_game.save_warning = 1;
    } else {
        g_game.save_warning = 0;
    }
    ti_Close(handle);
}
