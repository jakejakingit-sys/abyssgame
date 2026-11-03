#include <string.h>
#include "abyss.h"

World g_world;

const char *const g_world_names[4] = {
    "FORGOTTEN CRYPT", "SUNKEN MINES", "INFERNAL DEPTHS", "THE VOID"
};

const char *const g_level_names[LEVEL_COUNT] = {
    "THE ENTRANCE", "FORGOTTEN HALLS", "BURIAL GROUNDS", "BONE CHAMBERS", "BONE KING'S THRONE",
    "MINE ENTRANCE", "CRYSTAL CAVERNS", "COLLAPSING TUNNELS", "DEEP SHAFTS", "BROODMOTHER'S NEST",
    "ASHEN PATH", "BURNING HALLS", "MAGMA BRIDGE", "FURNACE CHAMBERS", "INFERNO'S ARENA",
    "FORGOTTEN GATE", "SHATTERED REALITY", "LOST KINGDOM", "ABYSSAL CORE", "THE FORGOTTEN ONE"
};

static uint8_t is_boss_level(uint8_t level) {
    return (uint8_t)(((level + 1u) % 5u) == 0u);
}

static uint8_t is_trial_level(uint8_t level) {
    return (uint8_t)(is_boss_level(level) || level == 3 || level == 8 || level == 13 || level == 18);
}

static void map_put(int x, int y, uint8_t tile) {
    if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H)
        g_world.map[y * MAP_W + x] = tile;
}

uint8_t world_tile(int tx, int ty) {
    if (tx < 0 || tx >= MAP_W || ty < 0 || ty >= MAP_H) return T_SOLID;
    return g_world.map[ty * MAP_W + tx];
}

void world_set_message(const char *message) {
    uint8_t i = 0;
    while (i < sizeof(g_world.message) - 1 && message[i]) {
        g_world.message[i] = message[i];
        ++i;
    }
    g_world.message[i] = '\0';
    g_world.message_timer = 100;
}

static void build_map(uint8_t level) {
    uint8_t x, y;
    uint8_t offset = (uint8_t)((level * 7u + (level / 4u) * 3u) % 5u);
    uint8_t boss = is_boss_level(level);
    memset(g_world.map, T_EMPTY, sizeof(g_world.map));

    /* Continuous ground keeps the first route readable; hazards and raised
       platforms create the challenge without requiring a large map asset. */
    for (x = 0; x < MAP_W; ++x) map_put(x, 13, T_SOLID);

    for (x = (uint8_t)(7 + offset); x < 73; x = (uint8_t)(x + 13)) {
        for (y = 10; y < 13; ++y) map_put(x, y, T_LADDER);
        for (y = (uint8_t)(x % 3 ? 10 : 9); y < (uint8_t)(x % 3 ? 11 : 10); ++y) {
            map_put((int)x + 2, y, T_PLATFORM);
            map_put((int)x + 3, y, T_PLATFORM);
            map_put((int)x + 4, y, T_PLATFORM);
        }
    }

    /* World-specific hazard cadence. */
    for (x = (uint8_t)(12 + offset); x < 70; x = (uint8_t)(x + 17)) {
        map_put(x, 12, T_HAZARD);
        if ((x & 1u) == 0u) map_put((int)x + 1, 12, T_HAZARD);
    }

    /* One main cache and one concealed cache per level. */
    map_put((int)(18u + (level % 3u) * 2u), 12, T_CHEST);
    map_put(48 + (level % 4u), 8, T_SECRET);
    map_put(49 + (level % 4u), 9, T_LADDER);
    map_put(49 + (level % 4u), 10, T_LADDER);
    map_put(49 + (level % 4u), 11, T_LADDER);

    if (boss) {
        map_put(22, 12, T_RUNE);
        map_put(44, 12, T_RUNE);
        map_put(65, 12, T_RUNE);
        g_world.rune_total = 3;
    } else {
        /* Optional lore rune on ordinary stages. */
        if ((level % 3u) == 1u) map_put(34, 12, T_RUNE);
        g_world.rune_total = 0;
    }

    map_put(78, 12, T_EXIT);
}

void game_unlock_abilities(uint8_t level_id) {
    /* Progression abilities are exploration tools, never attack moves. */
    g_save.ability_mask |= 1u; /* Echo */
    if (level_id >= 2)  g_save.ability_mask |= (1u << 1); /* Wall Grip */
    if (level_id >= 5)  g_save.ability_mask |= (1u << 2); /* Phase Step */
    if (level_id >= 8)  g_save.ability_mask |= (1u << 3); /* Ward */
    if (level_id >= 11) g_save.ability_mask |= (1u << 4); /* Long Echo */
    if (level_id >= 15) g_save.ability_mask |= (1u << 5); /* Glide */
    if (level_id >= 19) g_save.ability_mask |= (1u << 6); /* Gate Key */
}

uint16_t player_max_hp(void) {
    return (uint16_t)(100u + (uint16_t)g_save.hp_level * 20u);
}

void world_start(uint8_t level_id) {
    uint16_t i;
    if (level_id >= LEVEL_COUNT) level_id = LEVEL_COUNT - 1;
    memset(&g_world, 0, sizeof(g_world));
    g_world.level_id = level_id;
    g_world.world_id = (uint8_t)(level_id / 5u);
    g_world.guardian_active = is_trial_level(level_id);
    g_world.guardian_big = is_boss_level(level_id);
    g_world.guardian_x = 210;
    g_world.guardian_dir = 1;
    build_map(level_id);

    g_world.player.x = 32;
    g_world.player.y = 190;
    g_world.player.facing = 1;
    g_world.player.hp = player_max_hp();
    g_world.exit_open = (uint8_t)(g_world.rune_total == 0);

    /* Remove permanent cache objects from maps already recorded in the save. */
    if (g_save.treasure_mask & (1UL << level_id)) {
        for (i = 0; i < MAP_CELLS; ++i)
            if (g_world.map[i] == T_CHEST) g_world.map[i] = T_EMPTY;
    }
    if (g_save.secret_mask & (1UL << level_id)) {
        for (i = 0; i < MAP_CELLS; ++i)
            if (g_world.map[i] == T_SECRET) g_world.map[i] = T_EMPTY;
    }
    game_unlock_abilities(level_id);
}

static uint8_t solid_at(int px, int py) {
    uint8_t t = world_tile(px / TILE_SIZE, py / TILE_SIZE);
    return (uint8_t)(t == T_SOLID);
}

static uint8_t ladder_at(int px, int py) {
    return (uint8_t)(world_tile(px / TILE_SIZE, py / TILE_SIZE) == T_LADDER);
}

static void player_damage(uint8_t amount) {
    uint8_t reduction;
    if (g_world.player.invuln || g_world.player.ward_timer) return;
    reduction = (uint8_t)(g_save.armor_level * 8u);
    if (reduction > 40) reduction = 40;
    amount = (uint8_t)(amount - ((uint16_t)amount * reduction) / 100u);
    if (amount < 1) amount = 1;
    if (g_world.player.hp > amount) g_world.player.hp = (uint16_t)(g_world.player.hp - amount);
    else g_world.player.hp = 0;
    g_world.player.invuln = 42;
    g_world.player.regen_timer = 0;
    g_world.player.vy = -3;
    g_world.player.vx = (int8_t)(-g_world.player.facing * 2);
}

static void player_physics(uint8_t arrows, uint8_t actions, uint8_t pressed_actions) {
    Player *p = &g_world.player;
    int old_y = p->y;
    int next_x;
    uint8_t on_ladder;
    (void)actions;

    if (arrows & 0x02u) { p->vx = (int8_t)(p->vx > -3 ? p->vx - 1 : -3); p->facing = -1; }
    else if (arrows & 0x04u) { p->vx = (int8_t)(p->vx < 3 ? p->vx + 1 : 3); p->facing = 1; }
    else if (p->vx > 0) --p->vx;
    else if (p->vx < 0) ++p->vx;

    if ((pressed_actions & 0x01u) && (g_save.ability_mask & (1u << 2)) && !p->phase_cooldown) {
        p->phase_timer = 7;
        p->phase_cooldown = 70;
        p->ward_timer = (uint8_t)((g_save.ability_mask & (1u << 3)) ? 20 : 0);
    }
    if (p->phase_timer) {
        p->vx = (int8_t)(p->facing * 6);
        --p->phase_timer;
    }
    if (p->phase_cooldown) --p->phase_cooldown;
    if (p->ward_timer) --p->ward_timer;
    if (p->drop_timer) --p->drop_timer;
    if (p->invuln) --p->invuln;

    on_ladder = (uint8_t)(ladder_at(p->x + 6, p->y + 9) || ladder_at(p->x + 6, p->y + 17));
    p->climbing = 0;
    if (on_ladder && (arrows & 0x08u)) { p->vy = -2; p->climbing = 1; }
    else if (on_ladder && (arrows & 0x01u)) { p->vy = 2; p->climbing = 1; }

    if (!p->climbing) {
        if ((arrows & 0x01u) && p->grounded &&
            world_tile((p->x + 6) / TILE_SIZE, (p->y + 18) / TILE_SIZE) == T_PLATFORM) {
            p->drop_timer = 9;
            p->y = (int16_t)(p->y + 3);
            p->vy = 1;
            p->grounded = 0;
        }
        if ((pressed_actions & 0x02u) && p->grounded) {
            p->vy = -7;
            p->grounded = 0;
        }
        if ((g_save.ability_mask & (1u << 5)) && (arrows & 0x01u) && p->vy > 1)
            p->vy = 1;
        if (p->vy < 5) ++p->vy;
    }

    next_x = p->x + p->vx;
    if (p->vx > 0 && (solid_at(next_x + 11, p->y + 2) || solid_at(next_x + 11, p->y + 16))) {
        next_x = ((next_x + 11) / TILE_SIZE) * TILE_SIZE - 12;
        p->vx = 0;
        if ((g_save.ability_mask & (1u << 1)) && !p->grounded && p->vy > 1) p->vy = 1;
    } else if (p->vx < 0 && (solid_at(next_x, p->y + 2) || solid_at(next_x, p->y + 16))) {
        next_x = (next_x / TILE_SIZE + 1) * TILE_SIZE;
        p->vx = 0;
        if ((g_save.ability_mask & (1u << 1)) && !p->grounded && p->vy > 1) p->vy = 1;
    }
    if (next_x < 0) next_x = 0;
    if (next_x > MAP_W * TILE_SIZE - 14) next_x = MAP_W * TILE_SIZE - 14;
    p->x = (int16_t)next_x;

    p->y = (int16_t)(p->y + p->vy);
    p->grounded = 0;
    if (p->vy >= 0) {
        int feet_y = p->y + 18;
        uint8_t left_tile = world_tile((p->x + 1) / TILE_SIZE, feet_y / TILE_SIZE);
        uint8_t right_tile = world_tile((p->x + 10) / TILE_SIZE, feet_y / TILE_SIZE);
        uint8_t crossed_platform = (uint8_t)((old_y + 18) <= ((feet_y / TILE_SIZE) * TILE_SIZE));
        if (left_tile == T_SOLID || right_tile == T_SOLID ||
            (!p->drop_timer && crossed_platform && (left_tile == T_PLATFORM || right_tile == T_PLATFORM))) {
            p->y = (int16_t)((feet_y / TILE_SIZE) * TILE_SIZE - 18);
            p->vy = 0;
            p->grounded = 1;
        }
    } else if (solid_at(p->x + 2, p->y) || solid_at(p->x + 9, p->y)) {
        p->y = (int16_t)(((p->y / TILE_SIZE) + 1) * TILE_SIZE);
        p->vy = 0;
    }

    if (p->y > 240) {
        player_damage(15);
        p->y = 190;
        p->vy = 0;
    }

    if (world_tile((p->x + 6) / TILE_SIZE, (p->y + 17) / TILE_SIZE) == T_HAZARD)
        player_damage(12);

    if (++p->regen_timer >= 130) {
        p->regen_timer = 0;
        if (p->hp < player_max_hp()) ++p->hp;
    }
}

void world_interact(void) {
    Player *p = &g_world.player;
    int px = p->x + 6;
    int py = p->y + 9;
    int x, y, dx, dy, score;
    int target_x = -1, target_y = -1;
    int best_score = 0x7FFF;
    uint8_t tile = T_EMPTY;
    uint8_t reach = (uint8_t)(22u + (uint16_t)g_save.reach_level * 4u +
                              ((g_save.ability_mask & (1u << 4)) ? 10u : 0u));
    int min_x = (px - reach - 8) / TILE_SIZE;
    int max_x = (px + reach + 8) / TILE_SIZE;
    int min_y = (py - 24) / TILE_SIZE;
    int max_y = (py + 24) / TILE_SIZE;

    /* Select the closest usable object in a small neighborhood. Rune Reach
       expands the horizontal radius while vertical targeting stays readable. */
    for (y = min_y; y <= max_y; ++y) {
        for (x = min_x; x <= max_x; ++x) {
            uint8_t candidate = world_tile(x, y);
            if (candidate != T_CHEST && candidate != T_RUNE &&
                candidate != T_EXIT && candidate != T_SECRET) continue;
            dx = x * TILE_SIZE + 8 - px;
            dy = y * TILE_SIZE + 8 - py;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            if (dx > reach || dy > 24) continue;
            score = dx + dy * 2;
            if (score < best_score) {
                best_score = score;
                target_x = x;
                target_y = y;
                tile = candidate;
            }
        }
    }

    if (target_x < 0) {
        world_set_message("NOTHING WITHIN REACH");
        return;
    }
    if (tile == T_CHEST) {
        g_save.coins = (uint16_t)(g_save.coins + 12u + (g_world.level_id % 4u) * 2u);
        g_save.treasures_found++;
        g_save.treasure_mask |= (1UL << g_world.level_id);
        map_put(target_x, target_y, T_EMPTY);
        world_set_message("CACHE FOUND  +COINS");
        save_write();
    } else if (tile == T_SECRET) {
        g_save.coins = (uint16_t)(g_save.coins + 18u);
        g_save.secrets_found++;
        g_save.secret_mask |= (1UL << g_world.level_id);
        map_put(target_x, target_y, T_EMPTY);
        world_set_message("SECRET DISCOVERED");
        save_write();
    } else if (tile == T_RUNE) {
        map_put(target_x, target_y, T_EMPTY);
        if (g_world.rune_activated < 3) ++g_world.rune_activated;
        if (g_world.rune_total && g_world.rune_activated >= g_world.rune_total) {
            g_world.exit_open = 1;
            world_set_message("THREE RUNES AWAKENED");
        } else {
            world_set_message("RUNE RESONATES");
        }
    } else if (tile == T_EXIT) {
        if (g_world.exit_open && (g_world.level_id != 19 || (g_save.ability_mask & (1u << 6)))) world_complete();
        else world_set_message("THE GATE NEEDS 3 RUNES");
    }
}

void world_update(uint8_t arrows, uint8_t actions, uint8_t pressed_actions) {
    Player *p = &g_world.player;
    (void)actions;
    player_physics(arrows, actions, pressed_actions);

    if (g_world.guardian_active) {
        int limit = g_world.guardian_big ? 16 : 11;
        g_world.guardian_x = (int16_t)(g_world.guardian_x + g_world.guardian_dir * (g_world.guardian_big ? 2 : 1));
        if (g_world.guardian_x < 145) g_world.guardian_dir = 1;
        if (g_world.guardian_x > 670 - limit) g_world.guardian_dir = -1;
        if ((p->x + 12 > g_world.guardian_x - limit) &&
            (p->x < g_world.guardian_x + limit) && p->y > 170) {
            player_damage(g_world.guardian_big ? 18 : 12);
        }
    }

    if (g_world.camera_x < p->x - 132) g_world.camera_x = (int16_t)(p->x - 132);
    if (g_world.camera_x < 0) g_world.camera_x = 0;
    if (g_world.camera_x > MAP_W * TILE_SIZE - SCREEN_W)
        g_world.camera_x = MAP_W * TILE_SIZE - SCREEN_W;
    if (g_world.message_timer) --g_world.message_timer;

    if (g_world.player.hp == 0) {
        ++g_save.deaths;
        save_write();
        g_game.state = ST_GAME_OVER;
        return;
    }

    if (pressed_actions & 0x04u) world_interact();
}

void world_complete(void) {
    uint32_t bit = (1UL << g_world.level_id);
    if (!(g_save.completed_mask & bit)) {
        g_save.completed_mask |= bit;
        if (g_save.unlocked_level < LEVEL_COUNT - 1 && g_save.unlocked_level <= g_world.level_id)
            g_save.unlocked_level = (uint8_t)(g_world.level_id + 1u);
    }
    if (is_boss_level(g_world.level_id)) {
        uint8_t world_bit = (uint8_t)(1u << g_world.world_id);
        if (!(g_save.relic_mask & world_bit)) {
            g_save.relic_mask |= world_bit;
            ++g_save.bosses_cleared;
        }
        if (g_world.level_id == 19) g_game.state = ST_ENDING;
        else g_game.state = ST_LEVEL_CLEAR;
    } else {
        g_game.state = ST_LEVEL_CLEAR;
    }
    game_unlock_abilities(g_world.level_id);
    save_write();
}
