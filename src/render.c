#include <graphx.h>
#include "abyss.h"
#include "renderer.h"

#define C_BG 0
#define C_BG2 1
#define C_STONE 2
#define C_STONE_HI 3
#define C_ACCENT 4
#define C_ACCENT2 5
#define C_TEXT 6
#define C_DARK 7
#define C_PLAYER 8
#define C_PLAYER_HI 9
#define C_GOLD 10
#define C_RUNE 11
#define C_HAZARD 12
#define C_GREEN 13
#define C_RED 14
#define C_PANEL 15

static const uint16_t world_palettes[4][16] = {
    {0x0842,0x10A4,0x2128,0x39CE,0x5AD6,0x8C63,0xFFFF,0x0000,0xF81F,0xFCA0,0xFE60,0x8E7F,0xF800,0x07E0,0xA145,0x18C6},
    {0x0208,0x0430,0x0A55,0x1D18,0x3DBF,0x6F5F,0xFFFF,0x0000,0x07FF,0x5FFF,0xFEC0,0xA7FF,0xF800,0x07E0,0xA145,0x10B5},
    {0x2000,0x3800,0x5000,0x8000,0xB200,0xE340,0xFFFF,0x0000,0xF800,0xFC40,0xFEC0,0xFBE0,0xF800,0x07E0,0xA145,0x4800},
    {0x1008,0x200F,0x3816,0x501F,0x703F,0xA07F,0xFFFF,0x0000,0xB81F,0xD45F,0xF6BF,0x9FFF,0xF800,0x07E0,0xA145,0x3015}
};

static uint8_t count_bits(uint32_t value) {
    uint8_t count = 0;
    while (value) { value &= value - 1u; ++count; }
    return count;
}

static void txt(const char *s, int x, int y, uint8_t color) {
    gfx_SetTextFGColor(color);
    gfx_PrintStringXY(s, x, y);
}

static void num(uint32_t n, int x, int y, uint8_t color) {
    gfx_SetTextFGColor(color);
    gfx_SetTextXY(x, y);
    gfx_PrintUInt((unsigned int)n, 1);
}

static void panel(int x, int y, int w, int h) {
    gfx_SetColor(C_PANEL); gfx_FillRectangle(x, y, w, h);
    gfx_SetColor(C_ACCENT); gfx_Rectangle(x, y, w, h);
    gfx_SetColor(C_DARK); gfx_HorizLine(x + 3, y + h - 3, w - 6);
}

static void heading(const char *title, const char *sub) {
    gfx_FillScreen(C_BG);
    gfx_SetColor(C_BG2); gfx_FillRectangle(0, 0, 320, 33);
    gfx_SetColor(C_ACCENT); gfx_HorizLine(0, 33, 320);
    txt("ABYSS", 10, 8, C_GOLD);
    txt(title, 78, 8, C_TEXT);
    if (sub) txt(sub, 10, 42, C_ACCENT2);
}

void renderer_set_world_palette(uint8_t world_id) {
    if (world_id > 3) world_id = 0;
    gfx_SetPalette(world_palettes[world_id], sizeof(world_palettes[world_id]), 0);
}

void renderer_begin(void) {
    gfx_Begin();
    gfx_SetDrawScreen(); /* Intentionally render directly to LCD VRAM. */
    renderer_set_world_palette(0);
    gfx_SetTextConfig(gfx_text_noclip);
    gfx_SetTextTransparentColor(C_BG);
    gfx_SetTextScale(1, 1);
}

void renderer_end(void) { gfx_End(); }

void renderer_before_gc(void) { gfx_End(); }
void renderer_after_gc(void) {
    gfx_Begin();
    gfx_SetDrawScreen();
    renderer_set_world_palette(g_world.world_id);
    gfx_SetTextConfig(gfx_text_noclip);
    gfx_SetTextTransparentColor(C_BG);
    gfx_SetTextScale(1, 1);
}

void draw_title(void) {
    gfx_FillScreen(C_BG);
    gfx_SetColor(C_BG2); gfx_FillRectangle(0, 0, 320, 240);
    gfx_SetColor(C_STONE); gfx_FillRectangle(0, 170, 320, 70);
    gfx_SetColor(C_STONE_HI); gfx_HorizLine(0, 170, 320);
    gfx_SetColor(C_ACCENT); gfx_FillRectangle(36, 72, 248, 4);
    gfx_SetColor(C_ACCENT2); gfx_FillRectangle(58, 78, 204, 2);
    txt("A B Y S S", 94, 91, C_GOLD);
    txt("THE WORLD BENEATH", 84, 121, C_TEXT);
    txt("A PIXEL DUNGEON EXPLORATION RPG", 37, 143, C_ACCENT2);
    gfx_SetColor(C_GOLD); gfx_FillRectangle(92, 190, 136, 2);
    txt("PRESS ENTER TO BEGIN", 83, 205, C_TEXT);
    txt("ARROWS MOVE   2ND JUMPS", 65, 225, C_ACCENT2);
}

void draw_lobby(void) {
    static const char *items[4] = {"PLAY", "UPGRADES", "COLLECTION", "STATS"};
    uint8_t i;
    heading("THE WAYSTATION", "A quiet place between the worlds");
    panel(22, 70, 276, 145);
    for (i = 0; i < 4; ++i) {
        uint8_t c = (g_game.menu_choice == i) ? C_GOLD : C_TEXT;
        if (g_game.menu_choice == i) {
            gfx_SetColor(C_ACCENT); gfx_FillRectangle(40, 83 + i * 30, 240, 23);
        }
        txt(items[i], 56, 90 + i * 30, c);
    }
    txt("COINS", 212, 90, C_ACCENT2); num(g_save.coins, 260, 90, C_GOLD);
    txt("LEVELS", 212, 121, C_ACCENT2); num((uint32_t)count_bits(g_save.completed_mask), 260, 121, C_TEXT);
    txt("ENTER: SELECT", 34, 223, C_ACCENT2);
    if (g_game.save_warning) txt("SAVE IN RAM - ARCHIVE SPACE NEEDED", 34, 205, C_RED);
}

void draw_levels(void) {
    uint8_t row, i;
    heading("EXPEDITION MAP", "Choose an unlocked passage");
    panel(12, 60, 296, 166);
    for (row = 0; row < 8; ++row) {
        i = (uint8_t)(g_game.level_scroll + row);
        if (i >= LEVEL_COUNT) break;
        if (i == g_game.selected_level) {
            gfx_SetColor(C_ACCENT); gfx_FillRectangle(20, 67 + row * 19, 280, 18);
        }
        txt((i < 9) ? "0" : "", 26, 72 + row * 19, C_ACCENT2);
        num((uint32_t)i + 1u, 34, 72 + row * 19, C_ACCENT2);
        txt(g_level_names[i], 56, 72 + row * 19,
            (i == g_game.selected_level) ? C_GOLD : ((i <= g_save.unlocked_level) ? C_TEXT : C_STONE_HI));
        if (g_save.completed_mask & (1UL << i)) txt("DONE", 258, 72 + row * 19, C_GREEN);
        else if (i > g_save.unlocked_level) txt("LOCK", 258, 72 + row * 19, C_RED);
        else if ((i + 1u) % 5u == 0u) txt("TRIAL", 250, 72 + row * 19, C_GOLD);
    }
    txt("UP/DOWN: MOVE   ENTER: ENTER   CLEAR: BACK", 15, 229, C_ACCENT2);
}

void draw_upgrades(void) {
    static const char *names[3] = {"ARMOR", "RUNE REACH", "MAXIMUM HP"};
    static const char *desc[3] = {"Reduce hazard impact", "Interact from farther away", "Increase your health capacity"};
    uint8_t levels[3] = {g_save.armor_level, g_save.reach_level, g_save.hp_level};
    uint8_t i;
    heading("WAYSTATION WORKSHOP", "Spend coins on permanent upgrades");
    panel(14, 65, 292, 151);
    for (i = 0; i < 3; ++i) {
        uint16_t price = (uint16_t)(20u + levels[i] * 30u + (i * 5u));
        if (g_game.upgrade_choice == i) {
            gfx_SetColor(C_ACCENT); gfx_FillRectangle(22, 72 + i * 45, 276, 40);
        }
        txt(names[i], 31, 76 + i * 45, g_game.upgrade_choice == i ? C_GOLD : C_TEXT);
        txt(desc[i], 31, 91 + i * 45, C_ACCENT2);
        txt("LV", 216, 76 + i * 45, C_TEXT); num(levels[i], 236, 76 + i * 45, C_GOLD);
        if (levels[i] < 5) { txt("COST", 258, 76 + i * 45, C_TEXT); num(price, 290, 76 + i * 45, C_GOLD); }
        else txt("MAX", 265, 76 + i * 45, C_GREEN);
    }
    if (g_game.menu_notice == 1) txt("NOT ENOUGH COINS", 108, 207, C_RED);
    else if (g_game.menu_notice == 2) txt("UPGRADE AT MAX LEVEL", 96, 207, C_GREEN);
    txt("COINS", 20, 225, C_ACCENT2); num(g_save.coins, 70, 225, C_GOLD);
    txt("ENTER: BUY   CLEAR: BACK", 151, 225, C_ACCENT2);
}

void draw_collection(void) {
    uint8_t i;
    heading("RELIC ARCHIVE", "Records of places you have uncovered");
    panel(18, 66, 284, 155);
    txt("WORLD RELICS", 32, 79, C_GOLD);
    for (i = 0; i < 4; ++i) {
        txt(g_world_names[i], 32, 99 + i * 19, C_TEXT);
        if (g_save.relic_mask & (1u << i)) txt("FOUND", 238, 99 + i * 19, C_GREEN);
        else txt("UNKNOWN", 222, 99 + i * 19, C_STONE_HI);
    }
    txt("TREASURES FOUND", 32, 184, C_ACCENT2); num(g_save.treasures_found, 178, 184, C_GOLD);
    txt("SECRETS FOUND", 32, 200, C_ACCENT2); num(g_save.secrets_found, 178, 200, C_GOLD);
    txt("CAMPAIGN COMPLETION", 32, 216, C_ACCENT2);
    num((uint32_t)count_bits(g_save.completed_mask) * 5u, 178, 216, C_GOLD);
    txt("%", 204, 216, C_GOLD);
    txt("CLEAR: BACK", 224, 229, C_ACCENT2);
}

void draw_stats(void) {
    heading("CHRONICLE", "Your journey through the Abyss");
    panel(18, 65, 284, 150);
    txt("LEVELS COMPLETED", 34, 81, C_TEXT); num((uint32_t)count_bits(g_save.completed_mask), 238, 81, C_GOLD);
    txt("TRIALS CLEARED", 34, 108, C_TEXT); num(g_save.bosses_cleared, 238, 108, C_GOLD);
    txt("TOTAL DEATHS", 34, 135, C_TEXT); num(g_save.deaths, 238, 135, C_TEXT);
    txt("TREASURES", 34, 162, C_TEXT); num(g_save.treasures_found, 238, 162, C_GOLD);
    txt("SECRETS", 34, 189, C_TEXT); num(g_save.secrets_found, 238, 189, C_GOLD);
    txt("CLEAR: BACK", 224, 225, C_ACCENT2);
}

static void draw_background(void) {
    uint8_t i;
    gfx_FillScreen(C_BG);
    gfx_SetColor(C_BG2);
    for (i = 0; i < 8; ++i) {
        int x = (int)i * 47 - (g_world.camera_x / 4) % 47;
        gfx_FillRectangle(x, 55 + (i % 3) * 13, 22, 125 - (i % 3) * 11);
    }
    gfx_SetColor(C_STONE);
    gfx_FillRectangle(0, 184, 320, 56);
    gfx_SetColor(C_STONE_HI);
    gfx_HorizLine(0, 184, 320);
}

static void draw_tile(int tile, int sx, int sy) {
    switch (tile) {
        case T_SOLID:
            gfx_SetColor(C_STONE); gfx_FillRectangle(sx, sy, 16, 16);
            gfx_SetColor(C_STONE_HI); gfx_HorizLine(sx, sy, 16);
            gfx_SetColor(C_DARK); gfx_HorizLine(sx, sy + 15, 16);
            break;
        case T_PLATFORM:
            gfx_SetColor(C_STONE); gfx_FillRectangle(sx, sy + 5, 16, 11);
            gfx_SetColor(C_ACCENT2); gfx_HorizLine(sx, sy + 4, 16);
            break;
        case T_LADDER:
            gfx_SetColor(C_GOLD); gfx_VertLine(sx + 4, sy, 16);
            gfx_VertLine(sx + 12, sy, 16);
            gfx_HorizLine(sx + 4, sy + 4, 9);
            gfx_HorizLine(sx + 4, sy + 10, 9);
            break;
        case T_HAZARD:
            gfx_SetColor(C_HAZARD); gfx_FillRectangle(sx, sy + 5, 16, 11);
            gfx_SetColor(C_GOLD); gfx_Line(sx, sy + 5, sx + 8, sy);
            gfx_Line(sx + 8, sy, sx + 15, sy + 5);
            break;
        case T_CHEST:
            gfx_SetColor(C_GOLD); gfx_FillRectangle(sx + 2, sy + 4, 12, 10);
            gfx_SetColor(C_DARK); gfx_Rectangle(sx + 2, sy + 4, 12, 10);
            gfx_SetColor(C_TEXT); gfx_FillRectangle(sx + 7, sy + 7, 2, 3);
            break;
        case T_RUNE:
            gfx_SetColor(C_RUNE); gfx_FillRectangle(sx + 5, sy + 2, 6, 12);
            gfx_SetColor(C_TEXT); gfx_FillRectangle(sx + 7, sy + 5, 2, 6);
            gfx_SetColor(C_ACCENT2); gfx_HorizLine(sx + 2, sy + 8, 12);
            break;
        case T_EXIT:
            gfx_SetColor(C_ACCENT); gfx_FillRectangle(sx + 2, sy - 12, 12, 28);
            gfx_SetColor(C_GOLD); gfx_Rectangle(sx + 2, sy - 12, 12, 28);
            gfx_SetColor(C_BG); gfx_FillRectangle(sx + 5, sy - 8, 6, 24);
            break;
        case T_SECRET:
            if (g_world.player.x > (sx + g_world.camera_x - 40) &&
                g_world.player.x < (sx + g_world.camera_x + 40)) {
                gfx_SetColor(C_RUNE); gfx_FillRectangle(sx + 6, sy + 5, 4, 4);
            }
            break;
        default: break;
    }
}

static void draw_player(void) {
    int x = g_world.player.x - g_world.camera_x;
    int y = g_world.player.y;
    if (g_world.player.invuln && (g_game.tick & 3u) == 0u) return;
    gfx_SetColor(C_PLAYER); gfx_FillRectangle(x + 3, y + 5, 8, 10);
    gfx_SetColor(C_PLAYER_HI); gfx_FillRectangle(x + 4, y, 7, 7);
    gfx_SetColor(C_DARK); gfx_FillRectangle(x + (g_world.player.facing > 0 ? 9 : 4), y + 3, 2, 2);
    gfx_SetColor(C_ACCENT); gfx_FillRectangle(x + 1, y + 14, 4, 4);
    gfx_SetColor(C_STONE_HI); gfx_FillRectangle(x + 7, y + 15, 4, 3);
    if (g_world.player.phase_timer) {
        gfx_SetColor(C_ACCENT2); gfx_Rectangle(x - 4, y - 3, 21, 24);
    }
}

void draw_world(void) {
    int first, last, tx, ty, sx;
    uint8_t tile;
    renderer_set_world_palette(g_world.world_id);
    draw_background();
    first = g_world.camera_x / 16;
    last = first + 21;
    if (last >= MAP_W) last = MAP_W - 1;
    for (ty = 0; ty < MAP_H; ++ty) {
        for (tx = first; tx <= last; ++tx) {
            tile = world_tile(tx, ty);
            if (tile == T_EMPTY) continue;
            sx = tx * 16 - g_world.camera_x;
            draw_tile(tile, sx, ty * 16);
        }
    }
    if (g_world.guardian_active) {
        int gx = g_world.guardian_x - g_world.camera_x;
        gfx_SetColor(C_ACCENT2); gfx_FillCircle(gx, 187, g_world.guardian_big ? 16 : 10);
        gfx_SetColor(C_RUNE); gfx_FillCircle(gx, 184, g_world.guardian_big ? 9 : 6);
        gfx_SetColor(C_TEXT); gfx_FillRectangle(gx - 4, 181, 2, 2);
        gfx_FillRectangle(gx + 2, 181, 2, 2);
    }
    draw_player();

    gfx_SetColor(C_BG2); gfx_FillRectangle(0, 0, 320, 29);
    txt(g_level_names[g_world.level_id], 6, 4, C_TEXT);
    txt("HP", 6, 17, C_ACCENT2);
    gfx_SetColor(C_DARK); gfx_FillRectangle(30, 18, 81, 7);
    gfx_SetColor(C_GREEN); gfx_FillRectangle(31, 19,
        (int)((uint32_t)g_world.player.hp * 79u / player_max_hp()), 5);
    txt("COINS", 126, 17, C_GOLD); num(g_save.coins, 173, 17, C_TEXT);
    if (g_world.guardian_big) {
        txt("RUNE TRIAL", 220, 4, C_GOLD);
        num(g_world.rune_activated, 286, 4, C_TEXT);
        txt("/3", 296, 4, C_TEXT);
    }
    if (g_world.message_timer) {
        gfx_SetColor(C_BG2); gfx_FillRectangle(42, 211, 236, 19);
        gfx_SetColor(C_ACCENT); gfx_Rectangle(42, 211, 236, 19);
        txt(g_world.message, 50, 217, C_GOLD);
    } else {
        txt("ARROWS MOVE  2ND JUMP  UP INTERACT  ALPHA SPECIAL", 6, 228, C_ACCENT2);
    }
}

void draw_pause(void) {
    draw_world();
    panel(78, 79, 164, 80);
    txt("PAUSED", 130, 91, C_GOLD);
    txt("ENTER: RESUME", 101, 116, C_TEXT);
    txt("CLEAR: WAYSTATION", 91, 136, C_ACCENT2);
}

void draw_level_clear(void) {
    heading("PASSAGE RESTORED", "The route ahead is now open");
    panel(28, 75, 264, 116);
    txt(g_level_names[g_game.current_level], 46, 91, C_GOLD);
    txt("THE WAY FORWARD IS SAFE", 55, 119, C_TEXT);
    txt("PROGRESS SAVED", 94, 145, C_GREEN);
    txt("ENTER: RETURN TO WAYSTATION", 54, 208, C_ACCENT2);
}

void draw_game_over(void) {
    heading("LOST IN THE DEPTHS", "The expedition will begin again");
    panel(36, 78, 248, 102);
    txt("YOUR MAP AND COINS REMAIN", 56, 100, C_TEXT);
    txt("THE LEVEL WILL RESET", 75, 126, C_ACCENT2);
    txt("ENTER: RETRY", 115, 154, C_GOLD);
}

void draw_ending(void) {
    heading("THE ABYSS REMEMBERS", "A final light returns to the surface");
    panel(24, 70, 272, 133);
    txt("THE FOUR RELICS JOIN", 66, 91, C_GOLD);
    txt("THE FORGOTTEN GATE OPENS", 42, 117, C_TEXT);
    txt("THE WAYSTATION IS AT PEACE", 43, 143, C_ACCENT2);
    txt("CAMPAIGN COMPLETE", 78, 173, C_GREEN);
    txt("ENTER: WAYSTATION", 91, 218, C_TEXT);
}
