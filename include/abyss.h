#ifndef ABYSS_H
#define ABYSS_H

#include <stdint.h>
#include <stdbool.h>

#define SCREEN_W 320
#define SCREEN_H 240
#define TILE_SIZE 16
#define MAP_W 80
#define MAP_H 15
#define MAP_CELLS (MAP_W * MAP_H)
#define LEVEL_COUNT 20
#define SAVE_APPVAR "ABYSSSV"
#define SAVE_MAGIC 0x41594253UL
#define SAVE_VERSION 1

/* Tile IDs. The map is rebuilt into one reusable RAM buffer on level load. */
enum {
    T_EMPTY = 0, T_SOLID, T_PLATFORM, T_LADDER, T_HAZARD,
    T_CHEST, T_RUNE, T_EXIT, T_SECRET
};

typedef enum {
    ST_TITLE = 0, ST_LOBBY, ST_LEVELS, ST_UPGRADES,
    ST_COLLECTION, ST_STATS, ST_PLAY, ST_PAUSE,
    ST_LEVEL_CLEAR, ST_GAME_OVER, ST_ENDING
} GameState;

typedef struct {
    uint32_t magic;
    uint32_t completed_mask;
    uint32_t relic_mask;
    uint32_t treasure_mask;
    uint32_t secret_mask;
    uint32_t deaths;
    uint32_t treasures_found;
    uint32_t secrets_found;
    uint32_t bosses_cleared;
    uint16_t coins;
    uint8_t unlocked_level; /* zero-based highest selectable level */
    uint8_t armor_level;
    uint8_t reach_level;
    uint8_t hp_level;
    uint8_t ability_mask;
    uint8_t reserved[5];
    uint16_t checksum;
} SaveData;

typedef struct {
    int16_t x, y;
    int8_t vx, vy;
    int8_t facing;
    uint8_t grounded;
    uint8_t climbing;
    uint8_t invuln;
    uint8_t phase_timer;
    uint8_t phase_cooldown;
    uint8_t ward_timer;
    uint8_t drop_timer;
    uint16_t regen_timer;
    uint16_t hp;
} Player;

typedef struct {
    uint8_t map[MAP_CELLS];
    Player player;
    int16_t camera_x;
    int16_t guardian_x;
    int8_t guardian_dir;
    uint8_t guardian_active;
    uint8_t guardian_big;
    uint8_t rune_total;
    uint8_t rune_activated;
    uint8_t level_id;
    uint8_t world_id;
    uint8_t exit_open;
    uint8_t message_timer;
    char message[28];
} World;

typedef struct {
    GameState state;
    uint8_t menu_choice;
    uint8_t selected_level;
    uint8_t level_scroll;
    uint8_t upgrade_choice;
    uint8_t current_level;
    uint8_t previous_keys;
    uint8_t previous_arrows;
    uint16_t tick;
    uint8_t save_warning;
    uint8_t menu_notice;
} Game;

extern SaveData g_save;
extern World g_world;
extern Game g_game;

extern const char *const g_level_names[LEVEL_COUNT];
extern const char *const g_world_names[4];

void save_load(void);
void save_write(void);
void save_reset(void);
uint16_t save_checksum(const SaveData *data);

void world_start(uint8_t level_id);
void world_update(uint8_t arrows, uint8_t actions, uint8_t pressed_actions);
void world_interact(void);
void world_complete(void);
uint8_t world_tile(int tx, int ty);
void world_set_message(const char *message);

void renderer_begin(void);
void renderer_end(void);
void renderer_set_world_palette(uint8_t world_id);
void draw_title(void);
void draw_lobby(void);
void draw_levels(void);
void draw_upgrades(void);
void draw_collection(void);
void draw_stats(void);
void draw_world(void);
void draw_pause(void);
void draw_level_clear(void);
void draw_game_over(void);
void draw_ending(void);

uint16_t player_max_hp(void);
void game_unlock_abilities(uint8_t level_id);

#endif
