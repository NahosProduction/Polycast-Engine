# Moteur de Jeu 2.5D avec Raycasting

Ce projet implémente un moteur de jeu 2.5D utilisant la technique du raycasting, compatible avec les maps créées par votre éditeur de map.

## Fonctionnalités

- **Raycasting en temps réel** : Rendu 3D d'un monde 2D
- **Support multi-layers** : Sol, plafond et murs avec textures séparées
- **Système d'éclairage dynamique** : Lumières colorées avec atténuation réaliste
- **Éditeur de lumières intégré** : Créez et modifiez l'éclairage directement
- **Système de textures** : Support des textures BMP avec éclairage appliqué
- **Contrôles fluides** : Mouvement et rotation du joueur
- **Chargement dynamique de maps** : Changez de niveau en cours de jeu
- **Fenêtre redimensionnable** : Ajustez la résolution à la volée
- **Interface en ligne de commande** : Lancez directement avec une map spécifique

## Structure des fichiers

```
├── main.c              # Point d'entrée principal du moteur
├── map.h/c             # Gestion des cartes et chargement
├── player.h/c          # Logique du joueur et contrôles
├── textures.h/c        # Gestionnaire de textures
├── raycaster.h/c       # Moteur de rendu raycasting
├── lighting.h/c        # Système d'éclairage dynamique
├── map_loader.h/c      # Chargement dynamique de maps
├── map_editor_with_lights.c # Éditeur de map avec support lumières
├── Makefile            # Script de compilation
└── README.md           # Documentation
```

## Prérequis

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-image-dev build-essential
```

### Fedora/RHEL
```bash
sudo dnf install SDL2-devel SDL2_image-devel gcc make
```

### Arch Linux
```bash
sudo pacman -S sdl2 sdl2_image gcc make
```

## Compilation

1. **Compilation du moteur** :
```bash
make
```

2. **Compilation de l'éditeur avec lumières** :
```bash
make editor
```

3. **Installation des dépendances** (Ubuntu) :
```bash
make install-deps-ubuntu
```

4. **Nettoyage** :
```bash
make clean
```

## Structure des répertoires

Le moteur attend cette structure de répertoires :

```
├── maps/              # Fichiers de cartes (.txt) et lumières (.txt.lights)
│   ├── map.txt       # Carte par défaut
│   ├── map.txt.lights # Éclairage pour map.txt
│   └── ...
├── textures/         # Fichiers de textures (.bmp)
│   ├── texture1.bmp  # Texture ID 0
│   ├── texture2.bmp  # Texture ID 1
│   └── ...
├── raycaster         # Exécutable du moteur
└── map_editor_lights # Éditeur de map avec lumières
```

## Format de map

### Fichier de map (.txt)
Le moteur charge les maps au format généré par votre éditeur :
```
floor_type,floor_texture ceiling_type,ceiling_texture wall_type,wall_texture
```

### Fichier d'éclairage (.txt.lights)
Les données d'éclairage sont stockées séparément :
```
AMBIENT r g b intensity        # Lumière ambiante
LIGHTS count                   # Nombre de lumières
LIGHT x y r g b intensity radius # Chaque lumière
```

Exemple :
```
AMBIENT 0.15 0.15 0.20 0.4
LIGHTS 2
LIGHT 10.5 7.5 1.0 0.9 0.7 3.5 8.0
LIGHT 3.5 3.5 0.7 0.8 1.0 2.0 5.5
```

## Contrôles

- **WASD** ou **Flèches** : Mouvement avant/arrière/rotation
- **Q/E** : Mouvement latéral (strafe)
- **L** : Charger une nouvelle map (mode interactif)
- **O** : Toggle éclairage (test de performance)
- **ESC** : Quitter le jeu

## Utilisation

## Utilisation

### 1. Création de contenu avec l'éditeur
```bash
./map_editor_lights
```

**Contrôles de l'éditeur :**
- **TAB** : Basculer entre mode Tiles et Lumières
- **Mode Tiles** : F/C/W (layers), E/T (vide/solide), clic pour peindre
- **Mode Lumières** :
  - **Clic gauche** : Ajouter lumière
  - **Clic droit** : Sélectionner/supprimer lumière
  - **RGB-/+** : R/1, G/2, B/3 pour ajuster couleurs
  - **Intensité** : I/U pour augmenter/diminuer
  - **Rayon** : O/P pour ajuster la portée
  - **DELETE** : Supprimer lumière sélectionnée
- **S** : Sauvegarder, **L** : Charger

### 2. Lancement du moteur
```bash
# Avec map par défaut
./raycaster

# Avec map spécifique  
./raycaster ma_map
```

### 3. Chargement en cours de jeu
- Appuyez sur **L** pendant le jeu
- Choisissez une map dans la liste
- L'éclairage correspondant se charge automatiquement

## Système d'éclairage

### Caractéristiques
- **Lumières ponctuelles** : Jusqu'à 32 lumières simultanées
- **Performance optimisée** : Support de 20+ lumières à 45+ FPS
- **Couleurs RGB** : Chaque lumière a sa propre couleur
- **Atténuation efficace** : Distance et intensité optimisées
- **Lumière ambiante** : Éclairage de base configurable
- **Cache intelligent** : Traitement uniquement des lumières actives

### Types d'éclairage
- **Lumière ambiante** : Éclairage global uniforme
- **Lumières ponctuelles** : Sources de lumière localisées
- **Atténuation par distance** : Plus loin = plus sombre
- **Mélange de couleurs** : Superposition réaliste des lumières

### Paramètres des lumières
- **Position** : Coordonnées monde (x, y)
- **Couleur** : RGB (0.0 - 1.0)
- **Intensité** : Puissance (0.0 - 10.0+)
- **Rayon** : Distance d'influence maximale

## Dépannage

### Problème : "Aucune texture chargée"
- Vérifiez que le dossier `textures/` existe
- Assurez-vous que les fichiers sont nommés `texture1.bmp`, `texture2.bmp`, etc.
- Les textures doivent être au format BMP 24-bit

### Problème : "Erreur chargement map"
- Vérifiez que le fichier `maps/map.txt` existe
- Assurez-vous que le format correspond à celui de votre éditeur
- Le moteur créera une map par défaut si le chargement échoue

### Problème : Performances lentes
- Réduisez la résolution d'écran
- Utilisez des textures plus petites (32x32 ou 64x64)
- Compilez avec l'optimisation `-O2` (déjà inclus dans le Makefile)

## Améliorations possibles

1. **Éclairage avancé** : Ombres portées, éclairage volumétrique
2. **Support de sprites** : Objets et ennemis dans le monde
3. **Effets visuels** : Brouillard, particules, reflets
4. **Audio** : Sons 3D positionnels avec écho
5. **Physique** : Collisions plus précises, objets dynamiques
6. **Interface** : HUD, menu, inventaire
7. **Scripting** : Événements, triggers, animations
8. **Multijoueur** : Support réseau

## Licence

Ce projet est fourni à des fins éducatives. Vous êtes libre de le modifier et de l'utiliser selon vos besoins.