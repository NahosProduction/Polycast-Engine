#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "player.h"
#include "textures.h"
#include "raycaster.h"
#include "map_loader.h"
#include "../editor/lighting.h"

int main(int argc, char* argv[]) {
    // Initialisation SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    
    // Créer la fenêtre redimensionnable
    SDL_Window* window = SDL_CreateWindow("Moteur 2.5D - Raycaster",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, 
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf("Erreur SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Créer le renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erreur SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Initialiser les modules
    Map game_map;
    Player player;
    TextureManager texture_manager;
    RaycastRenderer raycaster;
    LightManager light_manager;
    
    // Charger les textures
    if (textures_init(&texture_manager, renderer) == 0) {
        printf("Aucune texture chargée, continuation avec des couleurs par défaut\n");
    }
    
    // Initialiser le système d'éclairage
    lighting_init(&light_manager);
    
    // Initialiser le raycaster avec la taille initiale
    int current_width = SCREEN_WIDTH;
    int current_height = SCREEN_HEIGHT;
    if (!raycaster_init(&raycaster, renderer, current_width, current_height)) {
        printf("Erreur initialisation raycaster\n");
        textures_destroy(&texture_manager);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Connecter le système d'éclairage au raycaster
    raycaster_set_lighting(&raycaster, &light_manager);
    
    // Système de chargement de maps
    char current_map[256] = "maps/map.txt";
    
    // Charger la map par défaut ou depuis les arguments
    if (argc > 1) {
        snprintf(current_map, sizeof(current_map), "maps/%s", argv[1]);
        if (strstr(argv[1], ".txt") == NULL) {
            strcat(current_map, ".txt");
        }
    }
    
    // Charger la map
    printf("Tentative de chargement: %s\n", current_map);
    if (!map_load(&game_map, current_map)) {
        printf("Erreur chargement %s, création d'une map par défaut\n", current_map);
        map_init(&game_map);
        
        // Créer une map de test simple
        for (int y = 0; y < game_map.height; y++) {
            for (int x = 0; x < game_map.width; x++) {
                if (x == 0 || x == game_map.width-1 || y == 0 || y == game_map.height-1) {
                    // Murs extérieurs
                    game_map.layers[LAYER_WALL][y][x].type = TILE_SOLID;
                    game_map.layers[LAYER_WALL][y][x].texture_id = 1;
                } else if (x == 5 && y > 3 && y < game_map.height-3) {
                    // Mur intérieur vertical
                    game_map.layers[LAYER_WALL][y][x].type = TILE_SOLID;
                    game_map.layers[LAYER_WALL][y][x].texture_id = 2;
                }
            }
        }
    }
    
    // Charger les données d'éclairage
    char light_filename[512];
    snprintf(light_filename, sizeof(light_filename), "%s.lights", current_map);
    if (!lighting_load_from_file(&light_manager, light_filename)) {
        // Créer quelques lumières blanches faibles par défaut si aucun fichier trouvé
        printf("Création de lumières par défaut (blanches)\n");
        lighting_add_light(&light_manager, game_map.width/2.0f, game_map.height/2.0f, 1.0f, 1.0f, 1.0f, 1.5f, 6.0f); // Lumière blanche centrale
        lighting_add_light(&light_manager, 3.0f, 3.0f, 1.0f, 1.0f, 1.0f, 1.2f, 4.0f);   // Lumière blanche coin
        if (game_map.width > 15 && game_map.height > 10) {
            lighting_add_light(&light_manager, game_map.width-3.0f, game_map.height-3.0f, 1.0f, 1.0f, 1.0f, 1.3f, 5.0f); // Lumière blanche autre coin
        }
    }
    
    // Initialiser le joueur à la position de spawn de la map
    player_init(&player, game_map.player_start_x, game_map.player_start_y, -1.0f, 0.0f);
    
    // Variables pour le timing
    Uint32 last_time = SDL_GetTicks();
    bool quit = false;
    SDL_Event event;
    
    printf("Moteur 2.5D démarré!\n");
    printf("Map chargée: %s (%dx%d)\n", current_map, game_map.width, game_map.height);
    printf("Player start: %.1f,%.1f\n", game_map.player_start_x, game_map.player_start_y);
    printf("Système d'éclairage: %d lumières actives\n", light_manager.count);
    printf("Contrôles:\n");
    printf("  WASD ou Flèches - Mouvement\n");
    printf("  Q/E - Mouvement latéral\n");
    printf("  L - Charger une nouvelle map\n");
    printf("  O - Toggle éclairage (test performance)\n");
    printf("  ESC - Quitter\n");
    printf("Usage: %s [nom_de_map] (sans extension .txt)\n", argv[0]);
    
    // Boucle principale
    while (!quit) {
        // Calcul du temps écoulé
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        
        // Gestion des événements
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        quit = true;
                    } else if (event.key.keysym.sym == SDLK_o) {
                        // Toggle éclairage on/off pour test de performance
                        if (raycaster.light_manager) {
                            raycaster_set_lighting(&raycaster, NULL);
                            printf("\n💡 Éclairage DÉSACTIVÉ (test performance)\n");
                        } else {
                            raycaster_set_lighting(&raycaster, &light_manager);
                            printf("\n💡 Éclairage ACTIVÉ (%d lumières)\n", light_manager.count);
                        }
                    } else if (event.key.keysym.sym == SDLK_l) {
                        // Charger une nouvelle map
                        printf("\n=== CHARGEMENT DE MAP ===\n");
                        
                        if (map_loader_load_interactive(&game_map, current_map, sizeof(current_map))) {
                            // Réinitialiser le joueur à la position de spawn de la nouvelle map
                            player_init(&player, game_map.player_start_x, game_map.player_start_y, -1.0f, 0.0f);
                            
                            // Charger les lumières correspondantes
                            char light_filename[512];
                            snprintf(light_filename, sizeof(light_filename), "%s.lights", current_map);
                            lighting_load_from_file(&light_manager, light_filename);
                        }
                        
                        printf("=========================\n\n");
                    }
                    break;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        // Fenêtre redimensionnée
                        current_width = event.window.data1;
                        current_height = event.window.data2;
                        printf("Fenêtre redimensionnée: %dx%d\n", current_width, current_height);
                        
                        // Recréer le raycaster avec la nouvelle taille
                        raycaster_destroy(&raycaster);
                        if (!raycaster_init(&raycaster, renderer, current_width, current_height)) {
                            printf("Erreur lors du redimensionnement\n");
                            quit = true;
                        } else {
                            // Reconnecter le système d'éclairage
                            raycaster_set_lighting(&raycaster, &light_manager);
                        }
                    }
                    break;
            }
        }
        
        // Mise à jour du joueur
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        player_update(&player, &game_map, keys, delta_time);
        
        // Rendu
        raycaster_render(&raycaster, &player, &game_map, &texture_manager);
        raycaster_present(&raycaster);
        
        // Limiter les FPS
        SDL_Delay(16); // ~60 FPS
    }
    
    // Nettoyage
    raycaster_destroy(&raycaster);
    textures_destroy(&texture_manager);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    printf("Moteur fermé proprement\n");
    return 0;
}