#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <SDL2/SDL.h>
#include "map.h"
#include "player.h"
#include "textures.h"
#include "../editor/lighting.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

typedef struct {
    SDL_Renderer* renderer;
    SDL_Texture* screen_texture;
    Uint32* screen_buffer;
    int screen_width;
    int screen_height;
    LightManager* light_manager;  // Gestionnaire d'éclairage
} RaycastRenderer;

// Fonctions publiques
int raycaster_init(RaycastRenderer* rc, SDL_Renderer* renderer, int width, int height);
void raycaster_destroy(RaycastRenderer* rc);
void raycaster_set_lighting(RaycastRenderer* rc, LightManager* lm);
void raycaster_render(RaycastRenderer* rc, Player* player, Map* map, TextureManager* tm);
void raycaster_clear_screen(RaycastRenderer* rc, Uint32 color);
void raycaster_present(RaycastRenderer* rc);

// Fonctions utilitaires
Uint32 raycaster_get_pixel_from_texture(Uint32* texture_pixels, int tex_x, int tex_y);
Uint32 raycaster_darken_color(Uint32 color, float factor);
void raycaster_list_maps(void);
int raycaster_resize(RaycastRenderer* rc, SDL_Renderer* renderer, int width, int height);

#endif