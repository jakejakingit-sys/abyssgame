#include <stdio.h>
#include <string.h>
#include "abyss.h"
#include <fileioc.h>

Game g_game;
static uint8_t fake_handle = 1;
static uint8_t saved_bytes[sizeof(SaveData)];

/* Minimal host-side fileioc stand-ins for exercising the save/world logic. */
uint8_t ti_Open(const char *name, const char *mode) {
    (void)name; (void)mode;
    return fake_handle;
}
size_t ti_Read(void *data, size_t size, size_t count, uint8_t handle) {
    (void)handle;
    if (size * count > sizeof(saved_bytes)) return 0;
    memcpy(data, saved_bytes, size * count);
    return count;
}
size_t ti_Write(const void *data, size_t size, size_t count, uint8_t handle) {
    (void)handle;
    if (size * count > sizeof(saved_bytes)) return 0;
    memcpy(saved_bytes, data, size * count);
    return count;
}
int ti_Close(uint8_t handle) { (void)handle; return 1; }
bool ti_ArchiveHasRoom(uint32_t bytes) { (void)bytes; return true; }
int ti_SetArchiveStatus(bool archive, uint8_t handle) {
    (void)archive; (void)handle; return 1;
}
void ti_SetGCBehavior(void (*before)(void), void (*after)(void)) {
    (void)before; (void)after;
}

static int fail(const char *message) {
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int main(void) {
    int i;
    const int runes[3] = {22, 44, 65};

    save_reset();
    if (g_save.checksum != save_checksum(&g_save))
        return fail("initial save checksum");
    g_save.coins = 37;
    g_save.checksum = save_checksum(&g_save);
    if (g_save.checksum != save_checksum(&g_save))
        return fail("checksum must ignore its own field and struct tail padding");

    world_start(0);
    if (g_world.world_id != 0 || g_world.player.hp != 100)
        return fail("first stage initialization");
    /* Reach upgrade / proximity selection must collect a chest from one tile away. */
    g_world.player.x = 270;
    g_world.player.y = 190;
    world_interact();
    if (!(g_save.treasure_mask & 1u) || g_save.coins <= 37u)
        return fail("nearby chest interaction and persistence");

    world_start(0);
    for (i = 0; i < MAP_CELLS; ++i)
        if (g_world.map[i] == T_CHEST) return fail("collected chest reappeared");

    world_start(4);
    if (!g_world.guardian_big || g_world.rune_total != 3)
        return fail("major trial setup");
    for (i = 0; i < 3; ++i) {
        g_world.player.x = (int16_t)(runes[i] * TILE_SIZE + 2);
        g_world.player.y = 190;
        world_interact();
    }
    if (!g_world.exit_open || g_world.rune_activated != 3)
        return fail("three rune gate condition");
    g_world.player.x = (int16_t)(78 * TILE_SIZE + 2);
    g_world.player.y = 190;
    world_interact();
    if (!(g_save.completed_mask & (1UL << 4)) || g_save.unlocked_level != 5)
        return fail("boss trial completion unlocks next level");
    if (!(g_save.relic_mask & 1u) || g_save.bosses_cleared != 1)
        return fail("world relic / trial statistic");

    puts("ABYSS gameplay/save logic tests: PASS (host stubs; not a CEdev build)");
    return 0;
}
