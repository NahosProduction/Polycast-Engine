# 🎨 Guide Complet de l'Éditeur de Map avec Lumières

## 🚀 Lancement
```bash
map_editor.exe
```

## 🎮 Contrôles Généraux

### **TAB** - Basculer entre les modes
- **Mode Tiles** : Édition de la géométrie (sol, murs, plafond)
- **Mode Lumières** : Placement et édition des lumières

### **S** - Sauvegarder
- Demande le nom du fichier
- Sauvegarde la map ET les lumières automatiquement

### **L** - Charger
- Charge une map existante
- Charge aussi les lumières associées

### **ESC** - Quitter

---

## 🧱 Mode TILES (Géométrie)

### Sélection du Layer
- **F** - Layer Sol (Floor)
- **C** - Layer Plafond (Ceiling) 
- **W** - Layer Mur (Wall)

### Type de Brosse
- **E** - Vide (Empty) - Efface
- **T** - Solide - Peint avec la texture sélectionnée

### Utilisation
1. **Clic gauche** dans la barre du haut = Sélectionner texture
2. **Clic gauche** sur la map = Peindre avec texture actuelle
3. **Clic droit** sur la map = Effacer (mettre en vide)
4. **Maintenir + glisser** = Peindre en continu

---

## 💡 Mode LUMIÈRES (Éclairage)

### Placement des Lumières
- **Clic gauche** = Ajouter une nouvelle lumière
- **Clic droit** = Sélectionner lumière existante
- **Clic droit** (sur lumière sélectionnée) = Supprimer
- **DELETE/BACKSPACE** = Supprimer lumière sélectionnée

### 🎨 Ajustement des Couleurs

#### **Rouge (R)**
- **R** = Diminuer rouge (-0.1)
- **1** = Augmenter rouge (+0.1)

#### **Vert (G)**
- **G** = Diminuer vert (-0.1)
- **2** = Augmenter vert (+0.1)

#### **Bleu (B)**
- **B** = Diminuer bleu (-0.1)
- **3** = Augmenter bleu (+0.1)

### ⚡ Ajustement de l'Intensité
- **U** = Diminuer intensité (-0.5)
- **I** = Augmenter intensité (+0.5)
- *Range: 0.5 à 10.0*

### 📏 Ajustement du Rayon
- **P** = Diminuer rayon (-0.5)
- **O** = Augmenter rayon (+0.5)
- *Range: 1.0 à 15.0*

---

## 🎯 Exemples Pratiques

### Créer une Lumière Blanche Standard
1. **TAB** pour passer en mode Lumières
2. Valeurs par défaut : RGB(1.0, 1.0, 1.0), I:1.5, R:5.0
3. **Clic gauche** sur la position désirée

### Créer une Lumière Rouge Intense
1. Mode Lumières actif
2. **R R R R** pour diminuer rouge à ~0.6
3. **G G G G G** pour diminuer vert à ~0.5
4. **B B B B B** pour diminuer bleu à ~0.5
5. **I I I** pour intensité ~3.0
6. **Clic gauche** pour placer

### Créer une Lumière Bleue Froide
1. Mode Lumières
2. **R R R R R** (rouge à ~0.5)
3. **G G G** (vert à ~0.7)
4. **3 3 3** (bleu à ~1.3, max 1.0)
5. **I** (intensité ~1.0 pour effet subtil)
6. Placer avec clic gauche

### Ajuster une Lumière Existante
1. **Clic droit** sur la lumière à modifier
2. Elle devient sélectionnée (contour blanc)
3. Utilisez R/1, G/2, B/3, U/I, P/O pour ajuster
4. Les changements s'appliquent à la lumière sélectionnée

---

## 🎨 Palette de Couleurs Courantes

### Lumières Chaudes
- **Bougie** : R:1.0, G:0.8, B:0.4, I:1.2
- **Feu de cheminée** : R:1.0, G:0.6, B:0.2, I:2.0
- **Ampoule** : R:1.0, G:0.9, B:0.7, I:1.8

### Lumières Froides
- **Néon** : R:0.8, G:0.9, B:1.0, I:2.2
- **Lune** : R:0.7, G:0.7, B:1.0, I:1.0
- **LED blanche** : R:1.0, G:1.0, B:1.0, I:1.5

### Lumières Colorées
- **Rouge danger** : R:1.0, G:0.1, B:0.1, I:2.5
- **Vert mystique** : R:0.2, G:1.0, B:0.3, I:1.8
- **Violet magique** : R:0.8, G:0.2, B:1.0, I:2.0

---

## 💾 Gestion des Fichiers

### Structure de Sauvegarde
```
maps/
  ├── ma_map.txt        # Géométrie (tiles)
  └── ma_map.txt.lights # Éclairage
```

### Format du Fichier Lumières
```
AMBIENT 0.3 0.3 0.3 0.2          # Lumière ambiante
LIGHTS 3                         # Nombre de lumières
LIGHT 10.5 7.5 1.0 1.0 1.0 1.5 5.0  # x y r g b intensité rayon
LIGHT 3.0 3.0 1.0 0.6 0.2 2.0 4.0
LIGHT 16.0 12.0 0.2 1.0 0.3 1.8 6.0
```

---

## 🔧 Conseils d'Utilisation

### Performance
- **Max 32 lumières** par map pour de bonnes performances
- **Rayon optimal** : 3-8 unités selon la taille de la zone
- **Intensité modérée** : 1.0-3.0 pour la plupart des cas

### Esthétique
- **Lumière ambiante faible** pour plus de contraste
- **Mélangez les couleurs** chaudes et froides
- **Variez les intensités** pour créer de la profondeur
- **Placez près des murs** pour un meilleur effet

### Workflow Recommandé
1. **Créer la géométrie** d'abord (mode Tiles)
2. **Placer les lumières principales** (éclairage général)
3. **Ajouter les lumières d'ambiance** (détails)
4. **Ajuster l'intensité globale** selon l'atmosphère
5. **Tester dans le moteur** régulièrement

---

## 🎮 Interface Visuelle

### Indicateurs
- **Cercle jaune** = Rayon d'influence de la lumière
- **Carré coloré** = Position exacte de la lumière
- **Contour blanc** = Lumière actuellement sélectionnée
- **Barre du haut** = Textures disponibles (mode Tiles)

### Informations en Temps Réel
La console affiche en continu :
- Mode actuel (Tiles/Lumières)
- Paramètres actuels (couleur, intensité, rayon)
- Nombre total de lumières
- Layer actuel (en mode Tiles)

---

