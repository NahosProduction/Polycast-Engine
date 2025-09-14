#include "lighting.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

void lighting_init(LightManager* lm) {
    lm->count = 0;
    lm->active_count = 0;
    
    // Initialiser toutes les lumières comme inactives
    for (int i = 0; i < MAX_LIGHTS; i++) {
        lm->lights[i].active = 0;
        lm->lights[i].x = 0.0f;
        lm->lights[i].y = 0.0f;
        lm->lights[i].r = 1.0f;
        lm->lights[i].g = 1.0f;
        lm->lights[i].b = 1.0f;
        lm->lights[i].intensity = 1.0f;
        lm->lights[i].radius = 5.0f;
        lm->lights[i].radius_squared = 25.0f;
        lm->active_lights[i] = -1;
    }
    
    // Lumière ambiante par défaut (faible et blanche)
    lighting_set_ambient(lm, 0.3f, 0.3f, 0.3f, 0.2f);
}

int lighting_add_light(LightManager* lm, float x, float y, float r, float g, float b, float intensity, float radius) {
    if (lm->count >= MAX_LIGHTS) {
        printf("Erreur: Nombre maximum de lumières atteint (%d)\n", MAX_LIGHTS);
        return -1;
    }
    
    int index = lm->count;
    Light* light = &lm->lights[index];
    
    light->x = x;
    light->y = y;
    light->r = r;
    light->g = g;
    light->b = b;
    light->intensity = intensity;
    light->radius = radius;
    light->radius_squared = radius * radius;  // Précalculer le carré
    light->active = 1;
    
    lm->count++;
    lighting_update_cache(lm);  // Mettre à jour le cache
    
    printf("Lumière ajoutée à (%.1f, %.1f) - RGB(%.2f,%.2f,%.2f) I:%.1f R:%.1f\n", 
           x, y, r, g, b, intensity, radius);
    
    return index;
}

void lighting_remove_light(LightManager* lm, int index) {
    if (index < 0 || index >= lm->count) return;
    
    // Désactiver la lumière
    lm->lights[index].active = 0;
    
    // Compacter le tableau
    for (int i = index; i < lm->count - 1; i++) {
        lm->lights[i] = lm->lights[i + 1];
    }
    
    lm->count--;
    lighting_update_cache(lm);  // Mettre à jour le cache
    printf("Lumière supprimée (index %d)\n", index);
}

void lighting_clear_all(LightManager* lm) {
    for (int i = 0; i < MAX_LIGHTS; i++) {
        lm->lights[i].active = 0;
    }
    lm->count = 0;
    lm->active_count = 0;
    printf("Toutes les lumières supprimées\n");
}

void lighting_update_cache(LightManager* lm) {
    lm->active_count = 0;
    for (int i = 0; i < lm->count; i++) {
        if (lm->lights[i].active) {
            lm->active_lights[lm->active_count] = i;
            lm->active_count++;
        }
    }
}

// Version ultra-optimisée de l'atténuation
float lighting_calculate_distance_attenuation_fast(float distance_squared, float radius_squared) {
    if (distance_squared > radius_squared) return 0.0f;
    
    // Atténuation linéaire simple (plus rapide que quadratique)
    float distance = sqrtf(distance_squared);
    float radius = sqrtf(radius_squared);
    float attenuation = 1.0f - (distance / radius);
    
    // Courbe simple pour adoucir
    return attenuation * attenuation;
}

// Version optimisée de l'application de lumière
Uint32 lighting_apply_light_to_color_fast(Uint32 base_color, float light_r, float light_g, float light_b, float intensity) {
    // Extraire les composantes (format RGBA8888)
    Uint8 r = (base_color >> 24) & 0xFF;
    Uint8 g = (base_color >> 16) & 0xFF;
    Uint8 b = (base_color >> 8) & 0xFF;
    Uint8 a = base_color & 0xFF;
    
    // Calculs en entiers pour plus de vitesse
    int fr = (int)((r * light_r * intensity) + 0.5f);
    int fg = (int)((g * light_g * intensity) + 0.5f);
    int fb = (int)((b * light_b * intensity) + 0.5f);
    
    // Clamping rapide
    if (fr > 255) fr = 255;
    if (fg > 255) fg = 255;
    if (fb > 255) fb = 255;
    
    return ((Uint8)fr << 24) | ((Uint8)fg << 16) | ((Uint8)fb << 8) | a;
}

// Version ultra-optimisée du calcul d'éclairage
void lighting_calculate_pixel_color_fast(LightManager* lm, float world_x, float world_y, 
                                        Uint32 base_color, Uint32* output_color) {
    // Commencer avec la lumière ambiante
    float total_r = lm->ambient_r * lm->ambient_intensity;
    float total_g = lm->ambient_g * lm->ambient_intensity;
    float total_b = lm->ambient_b * lm->ambient_intensity;
    
    // Traiter seulement les lumières actives (cache optimisé)
    for (int i = 0; i < lm->active_count; i++) {
        int light_idx = lm->active_lights[i];
        Light* light = &lm->lights[light_idx];
        
        // Calculer la distance au carré (éviter sqrt)
        float dx = world_x - light->x;
        float dy = world_y - light->y;
        float distance_squared = dx * dx + dy * dy;
        
        // Test rapide de distance
        if (distance_squared > light->radius_squared) continue;
        
        // Calculer l'atténuation optimisée
        float attenuation = lighting_calculate_distance_attenuation_fast(distance_squared, light->radius_squared);
        
        // Ignorer les contributions négligeables
        if (attenuation < MIN_LIGHT_CONTRIBUTION) continue;
        
        // Ajouter la contribution de cette lumière
        float contribution = light->intensity * attenuation;
        total_r += light->r * contribution;
        total_g += light->g * contribution;
        total_b += light->b * contribution;
    }
    
    // Appliquer l'éclairage final
    *output_color = lighting_apply_light_to_color_fast(base_color, total_r, total_g, total_b, 1.0f);
}

void lighting_set_ambient(LightManager* lm, float r, float g, float b, float intensity) {
    lm->ambient_r = r;
    lm->ambient_g = g;
    lm->ambient_b = b;
    lm->ambient_intensity = intensity;
}

Light* lighting_get_light(LightManager* lm, int index) {
    if (index < 0 || index >= lm->count) return NULL;
    return &lm->lights[index];
}

int lighting_save_to_file(LightManager* lm, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Erreur: impossible d'ouvrir %s pour écriture\n", filename);
        return 0;
    }
    
    // Sauvegarder la lumière ambiante
    fprintf(file, "AMBIENT %.2f %.2f %.2f %.2f\n", 
            lm->ambient_r, lm->ambient_g, lm->ambient_b, lm->ambient_intensity);
    
    // Sauvegarder le nombre de lumières
    fprintf(file, "LIGHTS %d\n", lm->count);
    
    // Sauvegarder chaque lumière
    for (int i = 0; i < lm->count; i++) {
        Light* light = &lm->lights[i];
        if (light->active) {
            fprintf(file, "LIGHT %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n",
                    light->x, light->y, light->r, light->g, light->b, 
                    light->intensity, light->radius);
        }
    }
    
    fclose(file);
    printf("Données d'éclairage sauvegardées dans %s\n", filename);
    return 1;
}

int lighting_load_from_file(LightManager* lm, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Aucun fichier d'éclairage trouvé: %s\n", filename);
        return 0;
    }
    
    lighting_init(lm); // Réinitialiser
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "AMBIENT", 7) == 0) {
            float r, g, b, intensity;
            if (sscanf(line, "AMBIENT %f %f %f %f", &r, &g, &b, &intensity) == 4) {
                lighting_set_ambient(lm, r, g, b, intensity);
            }
        } else if (strncmp(line, "LIGHTS", 6) == 0) {
            int count;
            sscanf(line, "LIGHTS %d", &count);
        } else if (strncmp(line, "LIGHT", 5) == 0) {
            float x, y, r, g, b, intensity, radius;
            if (sscanf(line, "LIGHT %f %f %f %f %f %f %f", 
                      &x, &y, &r, &g, &b, &intensity, &radius) == 7) {
                lighting_add_light(lm, x, y, r, g, b, intensity, radius);
            }
        }
    }
    
    fclose(file);
    printf("Données d'éclairage chargées depuis %s (%d lumières)\n", filename, lm->count);
    return 1;
}