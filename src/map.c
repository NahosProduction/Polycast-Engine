#include "map.h"
#include <stdio.h>
#include <string.h>

void map_init(Map* map) {
    map->width = MAP_WIDTH_DEFAULT;
    map->height = MAP_HEIGHT_DEFAULT;
    map->player_start_x = MAP_WIDTH_DEFAULT / 2.0f;
    map->player_start_y = MAP_HEIGHT_DEFAULT / 2.0f;
    
    // Initialiser toutes les tiles à vide
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int y = 0; y < MAP_HEIGHT_MAX; y++) {
            for (int x = 0; x < MAP_WIDTH_MAX; x++) {
                map->layers[layer][y][x].type = TILE_EMPTY;
                map->layers[layer][y][x].texture_id = 0;
            }
        }
    }
}

int map_load(Map* map, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Erreur : impossible d'ouvrir %s pour lecture\n", filename);
        return 0;
    }
    
    map_init(map);
    
    char line[256];
    // Lire la première ligne pour vérifier s'il y a une taille
    if (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "SIZE", 4) == 0) {
            int w, h;
            if (sscanf(line, "SIZE %d %d", &w, &h) == 2) {
                if (w > 0 && w <= MAP_WIDTH_MAX && h > 0 && h <= MAP_HEIGHT_MAX) {
                    map->width = w;
                    map->height = h;
                    printf("Taille de map chargée: %dx%d\n", map->width, map->height);
                } else {
                    printf("Taille invalide dans le fichier, utilisation par défaut\n");
                }
            }
        } else {
            // Ancien format sans taille, revenir au début du fichier
            rewind(file);
        }
    }
    
    // Lire le player start si présent
    if (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "PLAYER_START", 12) == 0) {
            float px, py;
            if (sscanf(line, "PLAYER_START %f %f", &px, &py) == 2) {
                map->player_start_x = px;
                map->player_start_y = py;
                printf("Player start chargé: %.1f,%.1f\n", map->player_start_x, map->player_start_y);
            }
        } else {
            // Pas de player start, revenir en arrière et utiliser défaut
            fseek(file, -strlen(line), SEEK_CUR);
        }
    }
    
    // Charger les données de la map
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            int floor_type, floor_tex, ceiling_type, ceiling_tex, wall_type, wall_tex;
            
            // Lire les données pour les 3 layers : floor, ceiling, wall
            if (fscanf(file, "%d,%d %d,%d %d,%d", 
                      &floor_type, &floor_tex, 
                      &ceiling_type, &ceiling_tex, 
                      &wall_type, &wall_tex) == 6) {
                
                // Floor layer
                map->layers[LAYER_FLOOR][y][x].type = floor_type;
                map->layers[LAYER_FLOOR][y][x].texture_id = floor_tex;
                
                // Ceiling layer
                map->layers[LAYER_CEILING][y][x].type = ceiling_type;
                map->layers[LAYER_CEILING][y][x].texture_id = ceiling_tex;
                
                // Wall layer
                map->layers[LAYER_WALL][y][x].type = wall_type;
                map->layers[LAYER_WALL][y][x].texture_id = wall_tex;
            } else {
                // En cas d'erreur de lecture, initialiser à vide
                for (int l = 0; l < NUM_LAYERS; l++) {
                    map->layers[l][y][x].type = TILE_EMPTY;
                    map->layers[l][y][x].texture_id = 0;
                }
            }
        }
    }
    
    fclose(file);
    printf("Map chargée depuis %s (%dx%d, start: %.1f,%.1f)\n", 
           filename, map->width, map->height, map->player_start_x, map->player_start_y);
    return 1;
}

int map_is_wall(Map* map, int x, int y) {
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return 1; // Considérer les bordures comme des murs
    }
    return map->layers[LAYER_WALL][y][x].type == TILE_SOLID;
}

int map_get_wall_texture(Map* map, int x, int y) {
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return 0;
    }
    return map->layers[LAYER_WALL][y][x].texture_id;
}

int map_get_floor_texture(Map* map, int x, int y) {
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return 0;
    }
    return map->layers[LAYER_FLOOR][y][x].texture_id;
}

int map_get_ceiling_texture(Map* map, int x, int y) {
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return 0;
    }
    return map->layers[LAYER_CEILING][y][x].texture_id;
}