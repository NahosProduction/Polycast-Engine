#ifndef PLAYER_H
#define PLAYER_H

#include "map.h"
#include <SDL2/SDL.h>

#define PLAYER_SPEED 3.0f
#define ROTATION_SPEED 2.0f

typedef struct {
    float x, y;           // Position
    float dir_x, dir_y;   // Direction (vecteur unitaire)
    float plane_x, plane_y; // Plan de la caméra (perpendiculaire à la direction)
    float move_speed;
    float rot_speed;
} Player;

// Fonctions publiques
void player_init(Player* player, float x, float y, float dir_x, float dir_y);
void player_update(Player* player, Map* map, const Uint8* keys, float deltaTime);
void player_rotate(Player* player, float angle);
void player_move(Player* player, Map* map, float dx, float dy);

#endif
