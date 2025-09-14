#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "lighting.h"

#define TILE_SIZE 32
#define MAP_WIDTH_MAX 40
#define MAP_HEIGHT_MAX 30
#define MAP_WIDTH_MIN 10
#define MAP_HEIGHT_MIN 8
#define TOOLBAR_HEIGHT 1
#define MAX_TEXTURES 8

// Variables de taille de map
int current_map_width = 20;
int current_map_height = 15;

// Player start
float player_start_x = 10.0f;
float player_start_y = 7.0f;

enum {
    BRUSH_EMPTY = 0,
    BRUSH_SOLID = 1
};

enum {
    LAYER_FLOOR = 0,
    LAYER_CEILING = 1,
    LAYER_WALL = 2
};

enum {
    MODE_TILES = 0,
    MODE_LIGHTS = 1,
    MODE_PLAYER_START = 2
};

typedef struct {
    int type;
    int texture_id;
} Tile;

Tile map[3][MAP_HEIGHT_MAX][MAP_WIDTH_MAX];
int current_brush = BRUSH_SOLID;
int current_texture = 0;
int current_layer = LAYER_WALL;
int current_mode = MODE_TILES;

// Variables pour l'éditeur de lumières
LightManager light_manager;
int selected_light = -1;
float light_color_r = 1.0f, light_color_g = 1.0f, light_color_b = 1.0f; // Blanc par défaut
float light_intensity = 1.5f;  // Intensité modérée
float light_radius = 5.0f;

SDL_Texture* textures[MAX_TEXTURES];
int texture_count = 0;
int mouse_down = 0;

int load_textures(SDL_Renderer* renderer) {
    char path[64];
    texture_count = 0;
    for (int i = 0; i < MAX_TEXTURES; i++) {
        snprintf(path, sizeof(path), "textures/texture%d.bmp", i + 1);
        SDL_Surface* surface = SDL_LoadBMP(path);
        if (!surface) break;
        textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (textures[i]) {
            texture_count++;
        } else {
            break;
        }
    }
    return texture_count;
}

void save_map(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Erreur : impossible d'ouvrir %s pour écriture\n", filename);
        return;
    }
    
    // Sauvegarder la taille de la map et le player start
    fprintf(file, "SIZE %d %d\n", current_map_width, current_map_height);
    fprintf(file, "PLAYER_START %.2f %.2f\n", player_start_x, player_start_y);
    
    for (int y = 0; y < current_map_height; y++) {
        for (int x = 0; x < current_map_width; x++) {
            fprintf(file, "%d,%d %d,%d %d,%d  ", 
                map[LAYER_FLOOR][y][x].type, map[LAYER_FLOOR][y][x].texture_id,
                map[LAYER_CEILING][y][x].type, map[LAYER_CEILING][y][x].texture_id,
                map[LAYER_WALL][y][x].type, map[LAYER_WALL][y][x].texture_id);
        }
        fprintf(file, "\n");
    }
    fclose(file);
    printf("Carte sauvegardée dans %s (taille: %dx%d, start: %.1f,%.1f)\n", 
           filename, current_map_width, current_map_height, player_start_x, player_start_y);
    
    // Sauvegarder aussi les lumières
    char light_filename[512];
    snprintf(light_filename, sizeof(light_filename), "%s.lights", filename);
    lighting_save_to_file(&light_manager, light_filename);
}

void load_map(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Erreur : impossible d'ouvrir %s pour lecture\n", filename);
        return;
    }
    
    char line[256];
    // Lire la première ligne pour la taille
    if (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "SIZE", 4) == 0) {
            int w, h;
            if (sscanf(line, "SIZE %d %d", &w, &h) == 2) {
                if (w >= MAP_WIDTH_MIN && w <= MAP_WIDTH_MAX && h >= MAP_HEIGHT_MIN && h <= MAP_HEIGHT_MAX) {
                    current_map_width = w;
                    current_map_height = h;
                    printf("Taille de map: %dx%d\n", current_map_width, current_map_height);
                } else {
                    printf("Taille invalide, utilisation par défaut\n");
                }
            }
        } else {
            // Ancien format sans taille, revenir au début
            rewind(file);
        }
    }
    
    // Lire le player start si présent
    if (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "PLAYER_START", 12) == 0) {
            float px, py;
            if (sscanf(line, "PLAYER_START %f %f", &px, &py) == 2) {
                player_start_x = px;
                player_start_y = py;
                printf("Player start: %.1f,%.1f\n", player_start_x, player_start_y);
            }
        } else {
            // Pas de player start, revenir en arrière
            fseek(file, -strlen(line), SEEK_CUR);
        }
    }
    
    for (int y = 0; y < current_map_height; y++) {
        for (int x = 0; x < current_map_width; x++) {
            int floor_type, floor_tex, ceiling_type, ceiling_tex, wall_type, wall_tex;
            if (fscanf(file, "%d,%d %d,%d %d,%d", 
                      &floor_type, &floor_tex, &ceiling_type, &ceiling_tex, &wall_type, &wall_tex) == 6) {
                map[LAYER_FLOOR][y][x].type = floor_type;
                map[LAYER_FLOOR][y][x].texture_id = floor_tex;
                map[LAYER_CEILING][y][x].type = ceiling_type;
                map[LAYER_CEILING][y][x].texture_id = ceiling_tex;
                map[LAYER_WALL][y][x].type = wall_type;
                map[LAYER_WALL][y][x].texture_id = wall_tex;
            } else {
                for (int l = 0; l < 3; l++) {
                    map[l][y][x].type = BRUSH_EMPTY;
                    map[l][y][x].texture_id = 0;
                }
            }
        }
    }
    fclose(file);
    printf("Carte chargée depuis %s (taille: %dx%d, start: %.1f,%.1f)\n", 
           filename, current_map_width, current_map_height, player_start_x, player_start_y);
    
    // Charger aussi les lumières
    char light_filename[512];
    snprintf(light_filename, sizeof(light_filename), "%s.lights", filename);
    lighting_load_from_file(&light_manager, light_filename);
}

void draw_tile_layer(SDL_Renderer* renderer, int x, int y, int layer) {
    SDL_Rect dst = { x * TILE_SIZE, y * TILE_SIZE + TILE_SIZE, TILE_SIZE, TILE_SIZE };
    Tile tile = map[layer][y][x];
    
    if (tile.type != BRUSH_EMPTY) {
        if (tile.texture_id >= 0 && tile.texture_id < texture_count) {
            SDL_RenderCopy(renderer, textures[tile.texture_id], NULL, &dst);
        } else {
            switch (layer) {
                case LAYER_FLOOR: SDL_SetRenderDrawColor(renderer, 101, 67, 33, 255); break;
                case LAYER_CEILING: SDL_SetRenderDrawColor(renderer, 50, 50, 150, 255); break;
                case LAYER_WALL: SDL_SetRenderDrawColor(renderer, 150, 50, 50, 255); break;
            }
            SDL_RenderFillRect(renderer, &dst);
        }
    } else {
        if (layer == current_layer) {
            SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        }
        SDL_RenderFillRect(renderer, &dst);
    }
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &dst);
}

void draw_player_start(SDL_Renderer* renderer) {
    // Dessiner le player start
    int screen_x = (int)(player_start_x * TILE_SIZE);
    int screen_y = (int)(player_start_y * TILE_SIZE) + TILE_SIZE;
    
    // Dessiner un cercle vert pour le player start
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int angle = 0; angle < 360; angle += 10) {
        float rad = angle * M_PI / 180.0f;
        int x1 = screen_x + (int)(8 * cos(rad));
        int y1 = screen_y + (int)(8 * sin(rad));
        SDL_RenderDrawPoint(renderer, x1, y1);
    }
    
    // Croix au centre
    SDL_RenderDrawLine(renderer, screen_x - 4, screen_y, screen_x + 4, screen_y);
    SDL_RenderDrawLine(renderer, screen_x, screen_y - 4, screen_x, screen_y + 4);
}

void draw_lights(SDL_Renderer* renderer) {
    for (int i = 0; i < light_manager.count; i++) {
        Light* light = &light_manager.lights[i];
        if (!light->active) continue;
        
        // Convertir coordonnées monde vers écran
        int screen_x = (int)(light->x * TILE_SIZE);
        int screen_y = (int)(light->y * TILE_SIZE) + TILE_SIZE;
        int radius_pixels = (int)(light->radius * TILE_SIZE);
        
        // Dessiner le rayon d'influence (cercle)
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 50);
        for (int angle = 0; angle < 360; angle += 5) {
            float rad = angle * M_PI / 180.0f;
            int x1 = screen_x + (int)(radius_pixels * cos(rad));
            int y1 = screen_y + (int)(radius_pixels * sin(rad));
            SDL_RenderDrawPoint(renderer, x1, y1);
        }
        
        // Dessiner la lumière elle-même
        SDL_Rect light_rect = { screen_x - 4, screen_y - 4, 8, 8 };
        
        // Couleur de la lumière
        Uint8 r = (Uint8)(light->r * 255);
        Uint8 g = (Uint8)(light->g * 255);
        Uint8 b = (Uint8)(light->b * 255);
        
        if (i == selected_light) {
            // Lumière sélectionnée - contour blanc
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &light_rect);
        }
        
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderFillRect(renderer, &light_rect);
    }
}

void draw_map(SDL_Renderer* renderer) {
    for (int y = 0; y < current_map_height; y++) {
        for (int x = 0; x < current_map_width; x++) {
            draw_tile_layer(renderer, x, y, current_layer);
        }
    }
    
    // Dessiner les lumières en mode lumière
    if (current_mode == MODE_LIGHTS) {
        draw_lights(renderer);
    }
    
    // Dessiner le player start en mode player start ou toujours visible
    if (current_mode == MODE_PLAYER_START || current_mode == MODE_TILES) {
        draw_player_start(renderer);
    }
}

void draw_info(SDL_Renderer* renderer) {
    const char* layer_names[] = {"Sol (Floor)", "Plafond (Ceiling)", "Mur (Wall)"};
    const char* brush_names[] = {"Vide (Empty)", "Solide"};
    const char* mode_names[] = {"🧱 Tiles", "💡 Lumières", "🎮 Player Start"};
    
    printf("\r"); // Retour en début de ligne
    if (current_mode == MODE_TILES) {
        printf("Mode: %s | Layer: %s | Brush: %s | Texture: %d | Start: %.1f,%.1f                    ", 
               mode_names[current_mode], layer_names[current_layer], 
               brush_names[current_brush], current_texture, player_start_x, player_start_y);
    } else if (current_mode == MODE_LIGHTS) {
        if (selected_light >= 0) {
            printf("Mode: %s | 💡:%d | Sélectionnée:%d | RGB(%.1f,%.1f,%.1f) I:%.1f R:%.1f        ", 
                   mode_names[current_mode], light_manager.count, selected_light,
                   light_color_r, light_color_g, light_color_b, light_intensity, light_radius);
        } else {
            printf("Mode: %s | 💡:%d | RGB(%.1f,%.1f,%.1f) I:%.1f R:%.1f [Clic G pour ajouter]     ", 
                   mode_names[current_mode], light_manager.count,
                   light_color_r, light_color_g, light_color_b, light_intensity, light_radius);
        }
    } else if (current_mode == MODE_PLAYER_START) {
        printf("Mode: %s | Position actuelle: %.1f,%.1f [Clic pour déplacer]                         ", 
               mode_names[current_mode], player_start_x, player_start_y);
    }
    fflush(stdout);
}

void handle_mouse(SDL_Event* event) {
    int x = event->button.x / TILE_SIZE;
    int y = event->button.y / TILE_SIZE;

    if (y == 0) {
        // Barre d'outils
        if (x >= 0 && x < texture_count) {
            if (current_texture != x) {
                current_texture = x;
                printf("\nTexture sélectionnée: %d\n", current_texture);
            }
        }
    } else if (y > 0 && y <= current_map_height) {
        int tile_y = y - 1;
        float world_x = (float)x;
        float world_y = (float)tile_y;
        
        if (x >= 0 && x < current_map_width) {
            if (current_mode == MODE_TILES) {
                // Mode édition de tiles
                if (event->type == SDL_MOUSEBUTTONDOWN || (event->type == SDL_MOUSEMOTION && mouse_down)) {
                    if (event->button.button == SDL_BUTTON_LEFT) {
                        map[current_layer][tile_y][x].type = current_brush;
                        map[current_layer][tile_y][x].texture_id = current_texture;
                    } else if (event->button.button == SDL_BUTTON_RIGHT) {
                        map[current_layer][tile_y][x].type = BRUSH_EMPTY;
                        map[current_layer][tile_y][x].texture_id = 0;
                    }
                }
            } else if (current_mode == MODE_LIGHTS) {
                // Mode édition de lumières
                if (event->type == SDL_MOUSEBUTTONDOWN) {
                    if (event->button.button == SDL_BUTTON_LEFT) {
                        // Ajouter une nouvelle lumière
                        int light_id = lighting_add_light(&light_manager, world_x + 0.5f, world_y + 0.5f,
                                                         light_color_r, light_color_g, light_color_b,
                                                         light_intensity, light_radius);
                        selected_light = light_id;
                        lighting_update_cache(&light_manager); // Mettre à jour le cache
                    } else if (event->button.button == SDL_BUTTON_RIGHT) {
                        // Sélectionner ou supprimer une lumière existante
                        int closest_light = -1;
                        float closest_dist = 1.0f; // Distance max pour sélection
                        
                        for (int i = 0; i < light_manager.count; i++) {
                            Light* light = &light_manager.lights[i];
                            if (!light->active) continue;
                            
                            float dx = world_x + 0.5f - light->x;
                            float dy = world_y + 0.5f - light->y;
                            float dist = sqrtf(dx * dx + dy * dy);
                            
                            if (dist < closest_dist) {
                                closest_dist = dist;
                                closest_light = i;
                            }
                        }
                        
                        if (closest_light >= 0) {
                            if (selected_light == closest_light) {
                                // Double-clic droit = supprimer
                                lighting_remove_light(&light_manager, closest_light);
                                selected_light = -1;
                            } else {
                                // Sélectionner
                                selected_light = closest_light;
                                Light* light = &light_manager.lights[selected_light];
                                light_color_r = light->r;
                                light_color_g = light->g;
                                light_color_b = light->b;
                                light_intensity = light->intensity;
                                light_radius = light->radius;
                                printf("\nLumière %d sélectionnée\n", selected_light);
                            }
                        }
                    }
                }
            } else if (current_mode == MODE_PLAYER_START) {
                // Mode player start
                if (event->type == SDL_MOUSEBUTTONDOWN) {
                    if (event->button.button == SDL_BUTTON_LEFT) {
                        // Déplacer le player start
                        player_start_x = world_x + 0.5f;
                        player_start_y = world_y + 0.5f;
                        printf("\n🎮 Player start déplacé à %.1f,%.1f\n", player_start_x, player_start_y);
                    }
                }
            }
        }
    }
}

void handle_keyboard(SDL_Event* event) {
    switch(event->key.keysym.sym) {
        case SDLK_ESCAPE: 
            exit(0); 
            break;
            
        // Changement de mode
        case SDLK_TAB:
            current_mode = (current_mode + 1) % 3; // Cycle entre les 3 modes
            const char* mode_names[] = {"🧱 Tiles", "💡 Lumières", "🎮 Player Start"};
            printf("\nMode: %s\n", mode_names[current_mode]);
            break;
            
        // Mode Tiles
        case SDLK_f: 
            if (current_mode == MODE_TILES) {
                current_layer = LAYER_FLOOR;
                printf("\nLayer actuel: Sol\n");
            }
            break;
        case SDLK_c: 
            if (current_mode == MODE_TILES) {
                current_layer = LAYER_CEILING;
                printf("\nLayer actuel: Plafond\n");
            }
            break;
        case SDLK_w: 
            if (current_mode == MODE_TILES) {
                current_layer = LAYER_WALL; 
                printf("\nLayer actuel: Mur\n");
            }
            break;
        case SDLK_e: 
            if (current_mode == MODE_TILES) {
                current_brush = BRUSH_EMPTY; 
                printf("\nBrosse: Vide\n");
            }
            break;
        case SDLK_t: 
            if (current_mode == MODE_TILES) {
                current_brush = BRUSH_SOLID; 
                printf("\nBrosse: Solide\n");
            }
            break;
            
        // Mode Lumières - Ajustements
        case SDLK_r:
            if (current_mode == MODE_LIGHTS) {
                light_color_r = (light_color_r > 0.1f) ? light_color_r - 0.1f : 0.0f;
                printf("\n🔴 Rouge: %.1f\n", light_color_r);
            }
            break;
        case SDLK_g:
            if (current_mode == MODE_LIGHTS) {
                light_color_g = (light_color_g > 0.1f) ? light_color_g - 0.1f : 0.0f;
                printf("\n🟢 Vert: %.1f\n", light_color_g);
            }
            break;
        case SDLK_b:
            if (current_mode == MODE_LIGHTS) {
                light_color_b = (light_color_b > 0.1f) ? light_color_b - 0.1f : 0.0f;
                printf("\n🔵 Bleu: %.1f\n", light_color_b);
            }
            break;
        case SDLK_1:
            if (current_mode == MODE_LIGHTS) {
                light_color_r = (light_color_r < 1.0f) ? light_color_r + 0.1f : 1.0f;
                printf("\n🔴 Rouge: %.1f\n", light_color_r);
            }
            break;
        case SDLK_2:
            if (current_mode == MODE_LIGHTS) {
                light_color_g = (light_color_g < 1.0f) ? light_color_g + 0.1f : 1.0f;
                printf("\n🟢 Vert: %.1f\n", light_color_g);
            }
            break;
        case SDLK_3:
            if (current_mode == MODE_LIGHTS) {
                light_color_b = (light_color_b < 1.0f) ? light_color_b + 0.1f : 1.0f;
                printf("\n🔵 Bleu: %.1f\n", light_color_b);
            }
            break;
        case SDLK_i:
            if (current_mode == MODE_LIGHTS) {
                light_intensity = (light_intensity < 10.0f) ? light_intensity + 0.5f : 10.0f;
                printf("\n⚡ Intensité: %.1f\n", light_intensity);
            }
            break;
        case SDLK_u:
            if (current_mode == MODE_LIGHTS) {
                light_intensity = (light_intensity > 0.5f) ? light_intensity - 0.5f : 0.5f;
                printf("\n⚡ Intensité: %.1f\n", light_intensity);
            }
            break;
        case SDLK_o:
            if (current_mode == MODE_LIGHTS) {
                light_radius = (light_radius < 15.0f) ? light_radius + 0.5f : 15.0f;
                printf("\n📏 Rayon: %.1f\n", light_radius);
            }
            break;
        case SDLK_p:
            if (current_mode == MODE_LIGHTS) {
                light_radius = (light_radius > 1.0f) ? light_radius - 0.5f : 1.0f;
                printf("\n📏 Rayon: %.1f\n", light_radius);
            }
            break;

        // Nouvelles commandes pour taille de map
        case SDLK_n:
            if (current_mode == MODE_TILES) {
                // Nouvelle map avec choix de taille
                printf("\n=== NOUVELLE MAP ===\n");
                printf("Taille actuelle: %dx%d\n", current_map_width, current_map_height);
                printf("Limites: %dx%d à %dx%d\n", MAP_WIDTH_MIN, MAP_HEIGHT_MIN, MAP_WIDTH_MAX, MAP_HEIGHT_MAX);
                printf("Largeur (10-40): ");
                
                int new_width, new_height;
                if (scanf("%d", &new_width) == 1 && new_width >= MAP_WIDTH_MIN && new_width <= MAP_WIDTH_MAX) {
                    printf("Hauteur (8-30): ");
                    if (scanf("%d", &new_height) == 1 && new_height >= MAP_HEIGHT_MIN && new_height <= MAP_HEIGHT_MAX) {
                        current_map_width = new_width;
                        current_map_height = new_height;
                        
                        // Vider la nouvelle map
                        for (int l = 0; l < 3; l++) {
                            for (int y = 0; y < current_map_height; y++) {
                                for (int x = 0; x < current_map_width; x++) {
                                    map[l][y][x].type = BRUSH_EMPTY;
                                    map[l][y][x].texture_id = 0;
                                }
                            }
                        }
                        
                        // Vider les lumières et remettre player start au centre
                        lighting_clear_all(&light_manager);
                        selected_light = -1;
                        player_start_x = current_map_width / 2.0f;
                        player_start_y = current_map_height / 2.0f;
                        
                        printf("✓ Nouvelle map créée: %dx%d\n", current_map_width, current_map_height);
                    } else {
                        printf("Hauteur invalide\n");
                    }
                } else {
                    printf("Largeur invalide\n");
                }
                printf("====================\n");
            }
            break;

        case SDLK_DELETE:
        case SDLK_BACKSPACE:
            if (current_mode == MODE_LIGHTS && selected_light >= 0) {
                lighting_remove_light(&light_manager, selected_light);
                selected_light = -1;
            }
            break;
            
        // Sauvegarde avec choix du nom
        case SDLK_s: {
            char filename[256];
            printf("\n=== SAUVEGARDE ===\n");
            printf("Nom du fichier (sans extension): ");
            if (fgets(filename, sizeof(filename), stdin)) {
                filename[strcspn(filename, "\n")] = '\0'; // Retirer le \n
                if (strlen(filename) > 0) {
                    char fullpath[512];
                    snprintf(fullpath, sizeof(fullpath), "maps/%s.txt", filename);
                    save_map(fullpath);
                } else {
                    printf("Nom invalide\n");
                }
            }
            printf("==================\n");
            break;
        }

        case SDLK_l:
            printf("\n=== CHARGEMENT ===\n");
            printf("Nom du fichier à charger (sans extension): ");
            char load_filename[256];
            if (fgets(load_filename, sizeof(load_filename), stdin)) {
                load_filename[strcspn(load_filename, "\n")] = '\0';
                if (strlen(load_filename) > 0) {
                    char fullpath[512];
                    snprintf(fullpath, sizeof(fullpath), "maps/%s.txt", load_filename);
                    load_map(fullpath);
                } else {
                    printf("Nom invalide\n");
                }
            }
            printf("==================\n");
            break;
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL_Init : %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow("Éditeur de Map avec Lumières - byNahos", 
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          current_map_width * TILE_SIZE, (current_map_height + TOOLBAR_HEIGHT) * TILE_SIZE, SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf("Erreur SDL_CreateWindow : %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erreur SDL_CreateRenderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    load_textures(renderer);
    lighting_init(&light_manager);

    // Initialiser la map à vide
    for (int l = 0; l < 3; l++) {
        for (int y = 0; y < current_map_height; y++) {
            for (int x = 0; x < current_map_width; x++) {
                map[l][y][x].type = BRUSH_EMPTY;
                map[l][y][x].texture_id = 0;
            }
        }
    }

    // Initialiser le player start au centre par défaut
    player_start_x = current_map_width / 2.0f;
    player_start_y = current_map_height / 2.0f;

    bool quit = false;
    SDL_Event event;

    printf("Éditeur avec système de lumières démarré\n");
    printf("=== CONTRÔLES RAPIDES ===\n");
    printf("🔄 TAB - Basculer Tiles ↔ Lumières ↔ Player Start\n");
    printf("💾 S - Sauvegarder | 📁 L - Charger | 🆕 N - Nouvelle map | ❌ ESC - Quitter\n");
    printf("\n🧱 Mode Tiles: F/C/W (layers) | E/T (vide/solide) | Clic = peindre\n");
    printf("💡 Mode Lumières: Clic G=ajouter | Clic D=sélectionner/suppr | DEL=supprimer\n");
    printf("🎮 Mode Player Start: Clic G=déplacer le point de spawn\n");
    printf("=========================\n\n");
    printf("📐 Taille actuelle: %dx%d | 🎮 Start: %.1f,%.1f\n", 
           current_map_width, current_map_height, player_start_x, player_start_y);
    printf("=========================\n");

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    mouse_down = 1;
                    handle_mouse(&event);
                    break;
                case SDL_MOUSEBUTTONUP:
                    mouse_down = 0;
                    break;
                case SDL_MOUSEMOTION:
                    handle_mouse(&event);
                    break;
                case SDL_KEYDOWN:
                    handle_keyboard(&event);
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        // Dessiner la barre d'outils (textures)
        for (int i = 0; i < texture_count; i++) {
            SDL_Rect rect = { i * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE };
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            SDL_RenderFillRect(renderer, &rect);
            SDL_RenderCopy(renderer, textures[i], NULL, &rect);
            if (i == current_texture) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                SDL_RenderDrawRect(renderer, &rect);
            }
        }

        draw_map(renderer);
        draw_info(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    // Libération
    for (int i = 0; i < texture_count; i++) {
        SDL_DestroyTexture(textures[i]);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}