#ifndef MAP_H
#define MAP_H

#include <SDL2/SDL.h>

#define MAP_WIDTH_MAX 40
#define MAP_HEIGHT_MAX 30
#define MAP_WIDTH_DEFAULT 20
#define MAP_HEIGHT_DEFAULT 15
#define TILE_SIZE 32

// Layers de la map
enum {
    LAYER_FLOOR = 0,
    LAYER_CEILING = 1,
    LAYER_WALL = 2,
    NUM_LAYERS = 3
};

// Types de tiles
enum {
    TILE_EMPTY = 0,
    TILE_SOLID = 1
};

typedef struct {
    int type;       // TILE_EMPTY ou TILE_SOLID
    int texture_id; // ID de la texture
} Tile;

typedef struct {
    Tile layers[NUM_LAYERS][MAP_HEIGHT_MAX][MAP_WIDTH_MAX];
    int width;
    int height;
    float player_start_x;
    float player_start_y;
} Map;

// Fonctions publiques
int map_load(Map* map, const char* filename);
int map_is_wall(Map* map, int x, int y);
int map_get_wall_texture(Map* map, int x, int y);
int map_get_floor_texture(Map* map, int x, int y);
int map_get_ceiling_texture(Map* map, int x, int y);
void map_init(Map* map);

#endif