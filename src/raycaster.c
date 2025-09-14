#include "raycaster.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int raycaster_init(RaycastRenderer* rc, SDL_Renderer* renderer, int width, int height) {
    rc->renderer = renderer;
    rc->screen_width = width;
    rc->screen_height = height;
    rc->light_manager = NULL;
    
    // Créer la texture pour le buffer d'écran
    rc->screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
                                         SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!rc->screen_texture) {
        printf("Erreur création texture écran: %s\n", SDL_GetError());
        return 0;
    }
    
    // Allouer le buffer d'écran
    rc->screen_buffer = malloc(width * height * sizeof(Uint32));
    if (!rc->screen_buffer) {
        printf("Erreur allocation buffer écran\n");
        SDL_DestroyTexture(rc->screen_texture);
        return 0;
    }
    
    return 1;
}

void raycaster_set_lighting(RaycastRenderer* rc, LightManager* lm) {
    rc->light_manager = lm;
}

void raycaster_destroy(RaycastRenderer* rc) {
    if (rc->screen_buffer) {
        free(rc->screen_buffer);
        rc->screen_buffer = NULL;
    }
    if (rc->screen_texture) {
        SDL_DestroyTexture(rc->screen_texture);
        rc->screen_texture = NULL;
    }
}

void raycaster_clear_screen(RaycastRenderer* rc, Uint32 color) {
    for (int i = 0; i < rc->screen_width * rc->screen_height; i++) {
        rc->screen_buffer[i] = color;
    }
}

Uint32 raycaster_get_pixel_from_texture(Uint32* texture_pixels, int tex_x, int tex_y) {
    if (!texture_pixels) return 0x808080FF; // Gris par défaut
    
    // S'assurer que les coordonnées sont dans les limites
    tex_x = tex_x % TEXTURE_SIZE;
    tex_y = tex_y % TEXTURE_SIZE;
    if (tex_x < 0) tex_x += TEXTURE_SIZE;
    if (tex_y < 0) tex_y += TEXTURE_SIZE;
    
    return texture_pixels[tex_y * TEXTURE_SIZE + tex_x];
}

Uint32 raycaster_darken_color(Uint32 color, float factor) {
    Uint8 r = (color >> 24) & 0xFF;
    Uint8 g = (color >> 16) & 0xFF;
    Uint8 b = (color >> 8) & 0xFF;
    Uint8 a = color & 0xFF;
    
    r = (Uint8)(r * factor);
    g = (Uint8)(g * factor);
    b = (Uint8)(b * factor);
    
    return (r << 24) | (g << 16) | (b << 8) | a;
}

void raycaster_render(RaycastRenderer* rc, Player* player, Map* map, TextureManager* tm) {
    int w = rc->screen_width;
    int h = rc->screen_height;
    
    // Rendu du sol et du plafond avec textures (échantillonnage optimisé)
    int sample_step = 2; // Échantillonner 1 pixel sur 2 pour l'éclairage
    
    for (int y = 0; y < h; y += sample_step) {
        // Calculer la distance au sol/plafond pour cette ligne
        float ray_dir_x0 = player->dir_x - player->plane_x;
        float ray_dir_y0 = player->dir_y - player->plane_y;
        float ray_dir_x1 = player->dir_x + player->plane_x;
        float ray_dir_y1 = player->dir_y + player->plane_y;
        
        int p = y - h / 2;
        if (p == 0) {
            // Ligne de l'horizon - remplir avec une couleur neutre
            for (int x = 0; x < w; x++) {
                for (int sy = 0; sy < sample_step && y + sy < h; sy++) {
                    rc->screen_buffer[(y + sy) * w + x] = 0x808080FF;
                }
            }
            continue;
        }
        
        float pos_z = 0.5 * h;
        float row_distance = pos_z / abs(p);
        
        float floor_step_x = row_distance * (ray_dir_x1 - ray_dir_x0) / w;
        float floor_step_y = row_distance * (ray_dir_y1 - ray_dir_y0) / w;
        
        float floor_x = player->x + row_distance * ray_dir_x0;
        float floor_y = player->y + row_distance * ray_dir_y0;
        
        for (int x = 0; x < w; x += sample_step) {
            int cell_x = (int)floor_x;
            int cell_y = (int)floor_y;
            
            int tex_x = (int)(TEXTURE_SIZE * (floor_x - cell_x)) & (TEXTURE_SIZE - 1);
            int tex_y = (int)(TEXTURE_SIZE * (floor_y - cell_y)) & (TEXTURE_SIZE - 1);
            
            Uint32 ceiling_color, floor_color;
            
            if (y < h / 2) {
                // Rendu du plafond
                int ceiling_tex = map_get_ceiling_texture(map, cell_x, cell_y);
                Uint32* ceiling_pixels = textures_get_pixels(tm, ceiling_tex);
                ceiling_color = raycaster_get_pixel_from_texture(ceiling_pixels, tex_x, tex_y);
                
                // Appliquer l'éclairage optimisé si disponible
                if (rc->light_manager) {
                    lighting_calculate_pixel_color_fast(rc->light_manager, floor_x, floor_y, ceiling_color, &ceiling_color);
                }
                
                // Assombrir légèrement le plafond
                ceiling_color = raycaster_darken_color(ceiling_color, 0.8f);
            } else {
                // Rendu du sol
                int floor_tex = map_get_floor_texture(map, cell_x, cell_y);
                Uint32* floor_pixels = textures_get_pixels(tm, floor_tex);
                floor_color = raycaster_get_pixel_from_texture(floor_pixels, tex_x, tex_y);
                
                // Appliquer l'éclairage optimisé si disponible
                if (rc->light_manager) {
                    lighting_calculate_pixel_color_fast(rc->light_manager, floor_x, floor_y, floor_color, &floor_color);
                }
            }
            
            // Répliquer le pixel échantillonné sur la zone sample_step x sample_step
            for (int sx = 0; sx < sample_step && x + sx < w; sx++) {
                for (int sy = 0; sy < sample_step && y + sy < h; sy++) {
                    if (y < h / 2) {
                        rc->screen_buffer[(y + sy) * w + (x + sx)] = ceiling_color;
                    } else {
                        rc->screen_buffer[(y + sy) * w + (x + sx)] = floor_color;
                    }
                }
            }
            
            floor_x += floor_step_x * sample_step;
            floor_y += floor_step_y * sample_step;
        }
    }
    
    // Raycasting pour chaque colonne d'écran (rendu des murs)
    for (int x = 0; x < w; x++) {
        // Calculer la direction du rayon
        float camera_x = 2 * x / (float)w - 1; // Coordonnée x dans l'espace caméra (-1 à 1)
        float ray_dir_x = player->dir_x + player->plane_x * camera_x;
        float ray_dir_y = player->dir_y + player->plane_y * camera_x;
        
        // Position actuelle dans la grille
        int map_x = (int)player->x;
        int map_y = (int)player->y;
        
        // Distance du rayon au prochain côté x ou y
        float delta_dist_x = (ray_dir_x == 0) ? 1e30 : fabs(1 / ray_dir_x);
        float delta_dist_y = (ray_dir_y == 0) ? 1e30 : fabs(1 / ray_dir_y);
        
        // Calcul du pas et de la distance initiale
        float side_dist_x, side_dist_y;
        int step_x, step_y;
        
        if (ray_dir_x < 0) {
            step_x = -1;
            side_dist_x = (player->x - map_x) * delta_dist_x;
        } else {
            step_x = 1;
            side_dist_x = (map_x + 1.0 - player->x) * delta_dist_x;
        }
        
        if (ray_dir_y < 0) {
            step_y = -1;
            side_dist_y = (player->y - map_y) * delta_dist_y;
        } else {
            step_y = 1;
            side_dist_y = (map_y + 1.0 - player->y) * delta_dist_y;
        }
        
        // DDA (Digital Differential Analyzer)
        int hit = 0;
        int side; // 0 pour côté NS, 1 pour côté EW
        
        while (hit == 0) {
            if (side_dist_x < side_dist_y) {
                side_dist_x += delta_dist_x;
                map_x += step_x;
                side = 0;
            } else {
                side_dist_y += delta_dist_y;
                map_y += step_y;
                side = 1;
            }
            
            if (map_is_wall(map, map_x, map_y)) {
                hit = 1;
            }
        }
        
        // Calcul de la distance
        float perp_wall_dist;
        if (side == 0) {
            perp_wall_dist = (map_x - player->x + (1 - step_x) / 2) / ray_dir_x;
        } else {
            perp_wall_dist = (map_y - player->y + (1 - step_y) / 2) / ray_dir_y;
        }
        
        // Calcul de la hauteur du mur à dessiner
        int line_height = (int)(h / perp_wall_dist);
        
        // Calcul des pixels de début et fin
        int draw_start = -line_height / 2 + h / 2;
        if (draw_start < 0) draw_start = 0;
        int draw_end = line_height / 2 + h / 2;
        if (draw_end >= h) draw_end = h - 1;
        
        // Récupérer la texture du mur
        int texture_id = map_get_wall_texture(map, map_x, map_y);
        Uint32* texture_pixels = textures_get_pixels(tm, texture_id);
        
        // Calcul de la coordonnée x sur la texture
        float wall_x;
        if (side == 0) {
            wall_x = player->y + perp_wall_dist * ray_dir_y;
        } else {
            wall_x = player->x + perp_wall_dist * ray_dir_x;
        }
        wall_x -= floor(wall_x);
        
        int tex_x = (int)(wall_x * TEXTURE_SIZE);
        if ((side == 0 && ray_dir_x > 0) || (side == 1 && ray_dir_y < 0)) {
            tex_x = TEXTURE_SIZE - tex_x - 1;
        }
        
        // Dessiner la colonne du mur (optimisé pour l'éclairage)
        float step = 1.0 * TEXTURE_SIZE / line_height;
        float tex_pos = (draw_start - h / 2 + line_height / 2) * step;
        
        // Calculer la position mondiale du mur une seule fois
        float wall_world_x, wall_world_y;
        if (side == 0) {
            wall_world_x = (float)map_x + (step_x > 0 ? 1.0f : 0.0f);
            wall_world_y = player->y + perp_wall_dist * ray_dir_y;
        } else {
            wall_world_x = player->x + perp_wall_dist * ray_dir_x;
            wall_world_y = (float)map_y + (step_y > 0 ? 1.0f : 0.0f);
        }
        
        // Calculer l'éclairage une seule fois pour toute la colonne
        float light_factor_r = 1.0f, light_factor_g = 1.0f, light_factor_b = 1.0f;
        
        if (rc->light_manager) {
            // Calculer les facteurs d'éclairage une seule fois
            float total_r = rc->light_manager->ambient_r * rc->light_manager->ambient_intensity;
            float total_g = rc->light_manager->ambient_g * rc->light_manager->ambient_intensity;
            float total_b = rc->light_manager->ambient_b * rc->light_manager->ambient_intensity;
            
            // Traiter seulement les lumières actives proches
            for (int i = 0; i < rc->light_manager->active_count; i++) {
                int light_idx = rc->light_manager->active_lights[i];
                Light* light = &rc->light_manager->lights[light_idx];
                
                float dx = wall_world_x - light->x;
                float dy = wall_world_y - light->y;
                float distance_squared = dx * dx + dy * dy;
                
                if (distance_squared > light->radius_squared) continue;
                
                float attenuation = lighting_calculate_distance_attenuation_fast(distance_squared, light->radius_squared);
                if (attenuation < MIN_LIGHT_CONTRIBUTION) continue;
                
                float contribution = light->intensity * attenuation;
                total_r += light->r * contribution;
                total_g += light->g * contribution;
                total_b += light->b * contribution;
            }
            
            light_factor_r = total_r;
            light_factor_g = total_g;
            light_factor_b = total_b;
        }
        
        // Assombrir les côtés EW pour un effet 3D
        if (side == 1) {
            light_factor_r *= 0.7f;
            light_factor_g *= 0.7f;
            light_factor_b *= 0.7f;
        }
        
        for (int y = draw_start; y < draw_end; y++) {
            int tex_y = (int)tex_pos & (TEXTURE_SIZE - 1);
            tex_pos += step;
            
            // Récupérer la couleur de texture pour cette ligne
            Uint32 color = raycaster_get_pixel_from_texture(texture_pixels, tex_x, tex_y);
            
            // Appliquer l'éclairage précalculé rapidement
            if (rc->light_manager) {
                Uint8 r = (color >> 24) & 0xFF;
                Uint8 g = (color >> 16) & 0xFF;
                Uint8 b = (color >> 8) & 0xFF;
                Uint8 a = color & 0xFF;
                
                int fr = (int)(r * light_factor_r);
                int fg = (int)(g * light_factor_g);
                int fb = (int)(b * light_factor_b);
                
                if (fr > 255) fr = 255;
                if (fg > 255) fg = 255;
                if (fb > 255) fb = 255;
                
                color = ((Uint8)fr << 24) | ((Uint8)fg << 16) | ((Uint8)fb << 8) | a;
            }
            
            rc->screen_buffer[y * w + x] = color;
        }
    }
}

void raycaster_present(RaycastRenderer* rc) {
    // Mettre à jour la texture avec le buffer
    SDL_UpdateTexture(rc->screen_texture, NULL, rc->screen_buffer, 
                     rc->screen_width * sizeof(Uint32));
    
    // Copier la texture vers le renderer
    SDL_RenderCopy(rc->renderer, rc->screen_texture, NULL, NULL);
    SDL_RenderPresent(rc->renderer);
}

void raycaster_list_maps(void) {
    printf("Maps disponibles:\n");
    printf("(Listez vos fichiers .txt dans le dossier maps/)\n");
}

int raycaster_resize(RaycastRenderer* rc, SDL_Renderer* renderer, int width, int height) {
    // Libérer les anciennes ressources
    if (rc->screen_buffer) {
        free(rc->screen_buffer);
    }
    if (rc->screen_texture) {
        SDL_DestroyTexture(rc->screen_texture);
    }
    
    // Réinitialiser avec la nouvelle taille
    return raycaster_init(rc, renderer, width, height);
}