#include "map_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

int map_loader_file_exists(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

void map_loader_list_available_maps(void) {
    printf("\n=== MAPS DISPONIBLES ===\n");
    
    DIR* dir = opendir("maps");
    if (dir) {
        struct dirent* entry;
        int map_count = 0;
        
        while ((entry = readdir(dir)) != NULL) {
            char* ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".txt") == 0) {
                // Retirer l'extension pour l'affichage
                char map_name[256];
                strncpy(map_name, entry->d_name, sizeof(map_name));
                map_name[ext - entry->d_name] = '\0';
                
                printf("  %d. %s\n", ++map_count, map_name);
            }
        }
        closedir(dir);
        
        if (map_count == 0) {
            printf("  Aucune map trouvée dans le dossier maps/\n");
        }
    } else {
        printf("  Impossible d'ouvrir le dossier maps/\n");
        printf("  Assurez-vous que le dossier existe.\n");
    }
    printf("========================\n");
}

int map_loader_load_interactive(Map* map, char* loaded_map_name, size_t name_size) {
    map_loader_list_available_maps();
    
    printf("Entrez le nom de la map (sans .txt) ou 'cancel' pour annuler: ");
    fflush(stdout);
    
    char input[256];
    if (fgets(input, sizeof(input), stdin)) {
        // Retirer le \n
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0 || strcmp(input, "cancel") == 0) {
            printf("Chargement annulé.\n");
            return 0;
        }
        
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "maps/%s.txt", input);
        
        if (!map_loader_file_exists(full_path)) {
            printf("Erreur: Le fichier %s n'existe pas.\n", full_path);
            return 0;
        }
        
        if (map_load(map, full_path)) {
            snprintf(loaded_map_name, name_size, "%s", full_path);
            printf("✓ Map '%s' chargée avec succès!\n", input);
            return 1;
        } else {
            printf("✗ Erreur lors du chargement de %s\n", full_path);
            return 0;
        }
    }
    
    return 0;
}