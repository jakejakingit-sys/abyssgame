#include <string.h>
#include <keypadc.h>
#include <fileioc.h>
#include <sys/timers.h>
#include <graphx.h>
#include "abyss.h"
#include "renderer.h"

Game g_game;

static uint8_t count_bits(uint32_t value) {
    uint8_t count = 0;
    while (value) { value &= value - 1u; ++count; }
    return count;
}

static void begin_level(uint8_t level) {
    if (level >= LEVEL_COUNT) return;
    g_game.current_level = level;
    world_start(level);
    save_write();
    g_game.state = ST_PLAY;
}

static void buy_upgrade(void) {
    uint8_t *level;
    uint16_t price;
    if (g_game.upgrade_choice == 0) level = &g_save.armor_level;
    else if (g_game.upgrade_choice == 1) level = &g_save.reach_level;
    else level = &g_save.hp_level;
    if (*level >= 5) {
        g_game.menu_notice = 2;
        return;
    }
    price = (uint16_t)(20u + (uint16_t)(*level) * 30u + (uint16_t)g_game.upgrade_choice * 5u);
    if (g_save.coins < price) {
        g_game.menu_notice = 1;
        return;
    }
    g_save.coins = (uint16_t)(g_save.coins - price);
    ++(*level);
    g_game.menu_notice = 0;
    save_write();
}

static void update_game(uint8_t arrows, uint8_t actions,
                        uint8_t pressed_arrows, uint8_t pressed_actions,
                        uint8_t enter_pressed, uint8_t clear_pressed) {
    switch (g_game.state) {
        case ST_TITLE:
            if (enter_pressed || (pressed_actions & 0x02u)) {
                g_game.state = ST_LOBBY;
                g_game.menu_choice = 0;
            }
            break;

        case ST_LOBBY:
            if (pressed_arrows & kb_Down) g_game.menu_choice = (uint8_t)((g_game.menu_choice + 1u) & 3u);
            if (pressed_arrows & kb_Up) g_game.menu_choice = (uint8_t)((g_game.menu_choice + 3u) & 3u);
            if (enter_pressed) {
                if (g_game.menu_choice == 0) {
                    g_game.selected_level = 0;
                    g_game.level_scroll = 0;
                    g_game.state = ST_LEVELS;
                } else if (g_game.menu_choice == 1) {
                    g_game.upgrade_choice = 0;
                    g_game.menu_notice = 0;
                    g_game.state = ST_UPGRADES;
                } else if (g_game.menu_choice == 2) g_game.state = ST_COLLECTION;
                else g_game.state = ST_STATS;
            }
            break;

        case ST_LEVELS:
            if (pressed_arrows & kb_Down) {
                if (g_game.selected_level < LEVEL_COUNT - 1) ++g_game.selected_level;
            }
            if (pressed_arrows & kb_Up) {
                if (g_game.selected_level) --g_game.selected_level;
            }
            if (g_game.selected_level < g_game.level_scroll) g_game.level_scroll = g_game.selected_level;
            if (g_game.selected_level >= g_game.level_scroll + 8)
                g_game.level_scroll = (uint8_t)(g_game.selected_level - 7u);
            if (enter_pressed && g_game.selected_level <= g_save.unlocked_level)
                begin_level(g_game.selected_level);
            if (clear_pressed) g_game.state = ST_LOBBY;
            break;

        case ST_UPGRADES:
            if (pressed_arrows & kb_Down) { g_game.upgrade_choice = (uint8_t)((g_game.upgrade_choice + 1u) % 3u); g_game.menu_notice = 0; }
            if (pressed_arrows & kb_Up) { g_game.upgrade_choice = (uint8_t)((g_game.upgrade_choice + 2u) % 3u); g_game.menu_notice = 0; }
            if (enter_pressed) buy_upgrade();
            if (clear_pressed) g_game.state = ST_LOBBY;
            break;

        case ST_COLLECTION:
        case ST_STATS:
            if (clear_pressed || enter_pressed) g_game.state = ST_LOBBY;
            break;

        case ST_PLAY:
            if (clear_pressed) g_game.state = ST_PAUSE;
            else world_update(arrows, actions, pressed_actions);
            break;

        case ST_PAUSE:
            if (enter_pressed) g_game.state = ST_PLAY;
            if (clear_pressed) g_game.state = ST_LOBBY;
            break;

        case ST_LEVEL_CLEAR:
            if (enter_pressed || clear_pressed) g_game.state = ST_LOBBY;
            break;

        case ST_GAME_OVER:
            if (enter_pressed) begin_level(g_game.current_level);
            if (clear_pressed) g_game.state = ST_LOBBY;
            break;

        case ST_ENDING:
            if (enter_pressed || clear_pressed) g_game.state = ST_LOBBY;
            break;
    }
}

static void draw_game(void) {
    switch (g_game.state) {
        case ST_TITLE: draw_title(); break;
        case ST_LOBBY: draw_lobby(); break;
        case ST_LEVELS: draw_levels(); break;
        case ST_UPGRADES: draw_upgrades(); break;
        case ST_COLLECTION: draw_collection(); break;
        case ST_STATS: draw_stats(); break;
        case ST_PLAY: draw_world(); break;
        case ST_PAUSE: draw_pause(); break;
        case ST_LEVEL_CLEAR: draw_level_clear(); break;
        case ST_GAME_OVER: draw_game_over(); break;
        case ST_ENDING: draw_ending(); break;
    }
}

int main(void) {
    uint8_t arrows, group1, group2, keys, system_keys;
    uint8_t pressed_arrows, pressed_actions;
    uint8_t enter_pressed, clear_pressed;

    memset(&g_game, 0, sizeof(g_game));
    g_game.state = ST_TITLE;
    save_load();
    ti_SetGCBehavior(renderer_before_gc, renderer_after_gc);
    renderer_begin();
    kb_DisableOnLatch();

    for (;;) {
        kb_Scan();
        if (kb_On) break;
        arrows = kb_Data[7];
        group1 = kb_Data[1]; /* 2ND is in keypad group 1. */
        group2 = kb_Data[2]; /* ALPHA is in keypad group 2. */
        system_keys = kb_Data[6]; /* ENTER and CLEAR are in group 6. */
        keys = (uint8_t)(((system_keys & kb_Enter) ? 0x01u : 0u) |
                         ((group1 & kb_2nd) ? 0x02u : 0u) |
                         ((group2 & kb_Alpha) ? 0x04u : 0u));
        pressed_arrows = (uint8_t)(arrows & (uint8_t)~g_game.previous_arrows);
        pressed_actions = 0;
        if ((keys & 0x04u) && !(g_game.previous_keys & 0x04u)) pressed_actions |= 0x01u;
        if ((keys & 0x02u) && !(g_game.previous_keys & 0x02u)) pressed_actions |= 0x02u;
        if ((pressed_arrows & kb_Up) || ((keys & 0x01u) && !(g_game.previous_keys & 0x01u)))
            pressed_actions |= 0x04u;
        enter_pressed = (uint8_t)((keys & 0x01u) && !(g_game.previous_keys & 0x01u));
        clear_pressed = (uint8_t)((system_keys & kb_Clear) != 0);
        g_game.previous_keys = keys;
        g_game.previous_arrows = arrows;
        ++g_game.tick;

        update_game(arrows, group1, pressed_arrows, pressed_actions,
                    enter_pressed, clear_pressed);
        draw_game();
        delay(24); /* Target: roughly 40 updates/sec; actual rate depends on draw cost. */
    }

    renderer_end();
    return 0;
}
