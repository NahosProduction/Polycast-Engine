# 📦 Guide de Build & Export

## 🎯 **Prérequis**

### **Icônes (Optionnel)**
Placez vos icônes dans le répertoire racine :
- `icon.ico` - Pour Windows (format ICO)
- `icon.png` - Pour Linux (256x256 recommandé)

Si aucune icône n'est présente, la compilation se fera sans icône.

## 📦 **Builds d'Export**

Dans le PowerShell copier coller le script :
```
build.ps1
```

## 🗂️ **Structure de Sortie**

```
build/
├── windows/
│   ├── raycaster.exe          # Jeu Windows
│   ├── map_editor.exe         # Éditeur Windows  
│   ├── launch.bat             # Lanceur simple
│   ├── SDL2.dll               # Dépendances
│   ├── textures/              # Assets
│   ├── maps/                  # Niveaux
│   └── README.txt             # Instructions
├── Raycaster-x86_64.AppImage  # Linux portable
└── ...
```

## ⚠️ **Notes Importantes**

- **Windows** : Icône automatique si `icon.ico` présent
- **Linux** : AppImage nécessite les droits d'exécution
- **Assets** : Dossiers `textures/` et `maps/` automatiquement copiés

## 🎯 **Workflow Complet**

1. **Développement** : `make && make run`
2. **Ajouter icônes** : Copier `icon.ico` et `icon.png`
3. **Export final** : `make build-all`
4. **Distribution** : Partager les dossiers/fichiers de `build/`

C'est tout ! 🚀
