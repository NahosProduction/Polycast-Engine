#ifndef LIGHTING_H
#define LIGHTING_H

#include <SDL2/SDL.h>

#define MAX_LIGHTS 32
#define LIGHT_SAMPLES 4  // Réduire l'échantillonnage pour performance
#define MIN_LIGHT_CONTRIBUTION 0.05f  // Seuil plus élevé pour ignorer les faibles lumières

typedef struct {
    float x, y;           // Position mondiale
    float r, g, b;        // Couleur RGB (0.0 - 1.0)
    float intensity;      // Intensité (0.0 - 10.0+)
    float radius;         // Rayon d'influence
    float radius_squared; // Rayon au carré (optimisation)
    int active;           // 1 si active, 0 sinon
} Light;

typedef struct {
    Light lights[MAX_LIGHTS];
    int count;
    float ambient_r, ambient_g, ambient_b;  // Lumière ambiante
    float ambient_intensity;
    
    // Cache d'optimisation
    int active_lights[MAX_LIGHTS];  // Indices des lumières actives seulement
    int active_count;               // Nombre de lumières actives
} LightManager;

// Fonctions principales
void lighting_init(LightManager* lm);
int lighting_add_light(LightManager* lm, float x, float y, float r, float g, float b, float intensity, float radius);
void lighting_remove_light(LightManager* lm, int index);
void lighting_clear_all(LightManager* lm);
void lighting_update_cache(LightManager* lm);

// Calculs d'éclairage optimisés
void lighting_calculate_pixel_color_fast(LightManager* lm, float world_x, float world_y, 
                                         Uint32 base_color, Uint32* output_color);
float lighting_calculate_distance_attenuation_fast(float distance_squared, float radius_squared);
Uint32 lighting_apply_light_to_color_fast(Uint32 base_color, float light_r, float light_g, float light_b, float intensity);

// Sauvegarde/Chargement
int lighting_save_to_file(LightManager* lm, const char* filename);
int lighting_load_from_file(LightManager* lm, const char* filename);

// Utilitaires
void lighting_set_ambient(LightManager* lm, float r, float g, float b, float intensity);
Light* lighting_get_light(LightManager* lm, int index);

#endif