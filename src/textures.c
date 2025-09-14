#include "textures.h"
#include <stdio.h>
#include <stdlib.h>

// Variables globales pour stocker les pixels des textures
Uint32* texture_pixels[MAX_TEXTURES];
int texture_loaded[MAX_TEXTURES];

int textures_init(TextureManager* tm, SDL_Renderer* renderer) {
    tm->count = 0;
    
    // Initialiser les tableaux
    for (int i = 0; i < MAX_TEXTURES; i++) {
        tm->textures[i] = NULL;
        texture_pixels[i] = NULL;
        texture_loaded[i] = 0;
    }
    
    char path[64];
    for (int i = 0; i < MAX_TEXTURES; i++) {
        snprintf(path, sizeof(path), "textures/texture%d.bmp", i + 1);
        
        SDL_Surface* surface = SDL_LoadBMP(path);
        if (!surface) {
            printf("Impossible de charger %s: %s\n", path, SDL_GetError());
            break;
        }
        
        // Convertir en format RGBA32 pour un accès facile aux pixels
        SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
        SDL_FreeSurface(surface);
        
        if (!converted) {
            printf("Erreur de conversion de surface: %s\n", SDL_GetError());
            break;
        }
        
        // Créer la texture SDL
        tm->textures[i] = SDL_CreateTextureFromSurface(renderer, converted);
        if (!tm->textures[i]) {
            printf("Erreur création texture: %s\n", SDL_GetError());
            SDL_FreeSurface(converted);
            break;
        }
        
        // Allouer et copier les pixels pour le raycasting
        texture_pixels[i] = malloc(TEXTURE_SIZE * TEXTURE_SIZE * sizeof(Uint32));
        if (texture_pixels[i]) {
            // Redimensionner si nécessaire
            if (converted->w != TEXTURE_SIZE || converted->h != TEXTURE_SIZE) {
                SDL_Surface* resized = SDL_CreateRGBSurface(0, TEXTURE_SIZE, TEXTURE_SIZE, 32, 
                                                          0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
                if (resized) {
                    SDL_BlitScaled(converted, NULL, resized, NULL);
                    memcpy(texture_pixels[i], resized->pixels, TEXTURE_SIZE * TEXTURE_SIZE * sizeof(Uint32));
                    SDL_FreeSurface(resized);
                } else {
                    memcpy(texture_pixels[i], converted->pixels, converted->w * converted->h * sizeof(Uint32));
                }
            } else {
                memcpy(texture_pixels[i], converted->pixels, TEXTURE_SIZE * TEXTURE_SIZE * sizeof(Uint32));
            }
            texture_loaded[i] = 1;
        }
        
        SDL_FreeSurface(converted);
        tm->count++;
    }
    
    printf("%d textures chargées\n", tm->count);
    return tm->count;
}

void textures_destroy(TextureManager* tm) {
    for (int i = 0; i < tm->count; i++) {
        if (tm->textures[i]) {
            SDL_DestroyTexture(tm->textures[i]);
            tm->textures[i] = NULL;
        }
        if (texture_pixels[i]) {
            free(texture_pixels[i]);
            texture_pixels[i] = NULL;
        }
        texture_loaded[i] = 0;
    }
    tm->count = 0;
}

SDL_Texture* textures_get(TextureManager* tm, int id) {
    if (id >= 0 && id < tm->count) {
        return tm->textures[id];
    }
    return NULL;
}

Uint32* textures_get_pixels(TextureManager* tm, int id) {
    if (id >= 0 && id < MAX_TEXTURES && texture_loaded[id]) {
        return texture_pixels[id];
    }
    return NULL;
}
