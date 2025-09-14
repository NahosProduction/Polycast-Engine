#include "player.h"
#include <math.h>

void player_init(Player* player, float x, float y, float dir_x, float dir_y) {
    player->x = x;
    player->y = y;
    player->dir_x = dir_x;
    player->dir_y = dir_y;
    
    // Le plan de la caméra est perpendiculaire à la direction
    // et sa longueur détermine le FOV (field of view)
    player->plane_x = -dir_y * 0.66f; // FOV d'environ 66 degrés
    player->plane_y = dir_x * 0.66f;
    
    player->move_speed = PLAYER_SPEED;
    player->rot_speed = ROTATION_SPEED;
}

void player_rotate(Player* player, float angle) {
    float cos_a = cos(angle);
    float sin_a = sin(angle);
    
    // Rotation du vecteur direction
    float old_dir_x = player->dir_x;
    player->dir_x = player->dir_x * cos_a - player->dir_y * sin_a;
    player->dir_y = old_dir_x * sin_a + player->dir_y * cos_a;
    
    // Rotation du plan de la caméra
    float old_plane_x = player->plane_x;
    player->plane_x = player->plane_x * cos_a - player->plane_y * sin_a;
    player->plane_y = old_plane_x * sin_a + player->plane_y * cos_a;
}

void player_move(Player* player, Map* map, float dx, float dy) {
    float new_x = player->x + dx;
    float new_y = player->y + dy;
    
    // Vérifier les collisions avec les murs
    if (!map_is_wall(map, (int)new_x, (int)player->y)) {
        player->x = new_x;
    }
    if (!map_is_wall(map, (int)player->x, (int)new_y)) {
        player->y = new_y;
    }
}

void player_update(Player* player, Map* map, const Uint8* keys, float deltaTime) {
    float move_speed = player->move_speed * deltaTime;
    float rot_speed = player->rot_speed * deltaTime;
    
    // Rotation
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
        player_rotate(player, -rot_speed);
    }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
        player_rotate(player, rot_speed);
    }
    
    // Mouvement avant/arrière
    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
        player_move(player, map, player->dir_x * move_speed, player->dir_y * move_speed);
    }
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
        player_move(player, map, -player->dir_x * move_speed, -player->dir_y * move_speed);
    }
    
    // Mouvement latéral (strafing)
    if (keys[SDL_SCANCODE_Q]) {
        player_move(player, map, -player->plane_x * move_speed, -player->plane_y * move_speed);
    }
    if (keys[SDL_SCANCODE_E]) {
        player_move(player, map, player->plane_x * move_speed, player->plane_y * move_speed);
    }
}