#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL.h>

#define MAX_TEXTURES 8
#define TEXTURE_SIZE 64

typedef struct {
    SDL_Texture* textures[MAX_TEXTURES];
    int count;
} TextureManager;

// Fonctions publiques
int textures_init(TextureManager* tm, SDL_Renderer* renderer);
void textures_destroy(TextureManager* tm);
SDL_Texture* textures_get(TextureManager* tm, int id);
Uint32* textures_get_pixels(TextureManager* tm, int id);

// Variables globales pour les pixels des textures (pour le raycasting)
extern Uint32* texture_pixels[MAX_TEXTURES];
extern int texture_loaded[MAX_TEXTURES];

#endif