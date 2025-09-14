#ifndef MAP_LOADER_H
#define MAP_LOADER_H

#include "map.h"

// Fonctions pour le chargement dynamique de maps
void map_loader_list_available_maps(void);
int map_loader_load_interactive(Map* map, char* loaded_map_name, size_t name_size);
int map_loader_file_exists(const char* filename);

#endif