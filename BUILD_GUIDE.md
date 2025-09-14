# 📦 Guide de Build & Export

## 🎯 **Prérequis**

### **Icônes (Optionnel)**
Placez vos icônes dans le répertoire racine :
- `icon.ico` - Pour Windows (format ICO)
- `icon.png` - Pour Linux (256x256 recommandé)

Si aucune icône n'est présente, la compilation se fera sans icône.

### **Dépendances par Plateforme**

#### **Linux (développement)**
```bash
# Ubuntu/Debian
make install-deps-ubuntu

# Fedora/RHEL  
make install-deps-fedora
```

#### **Windows (cross-compilation)**
```bash
# Installer mingw-w64
sudo apt-get install mingw-w64
```

#### **Web (Emscripten)**
```bash
# Installer Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

## 🚀 **Compilation Standard**

```bash
# Compilation normale
make

# Compiler l'éditeur
make editor

# Lancer le jeu
make run

# Lancer l'éditeur
make run-editor
```

## 📦 **Builds d'Export**

### **Windows (.exe)**
```bash
make build-windows
```
**Résultat :** `build/windows/`
- `raycaster.exe` (avec icône si `icon.ico` présent)
- `map_editor.exe`
- `launch.bat` (lanceur)
- DLLs SDL incluses
- README.txt

### **Linux (AppImage)**
```bash  
make build-linux
```
**Résultat :** `build/Raycaster-x86_64.AppImage`
- Portable, fonctionne sur toutes les distributions
- Icône intégrée si `icon.png` présent

### **Web (HTML5)**
```bash
make build-web
```
**Résultat :** `build/web/`
- `raycaster.html` (jeu web)
- `raycaster.js` + `raycaster.wasm`
- Interface web stylée

### **Build Complet**
```bash
make build-all
```
Compile pour toutes les plateformes d'un coup !

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
└── web/
    ├── raycaster.html         # Version web
    ├── raycaster.js           # Code JS généré
    ├── raycaster.wasm         # WebAssembly
    └── raycaster.data         # Assets packés
```

## 🌐 **Déploiement Web**

1. **Build** : `make build-web`
2. **Test** : Ouvrir `build/web/raycaster.html` dans le navigateur
3. **Héberger** : Upload du dossier `build/web/` sur votre serveur
4. **Partager** : `https://monsite.com/raycaster.html`

## 🧹 **Nettoyage**

```bash
# Nettoyer compilation normale
make clean

# Nettoyer tous les builds
make clean-builds
```

## ⚠️ **Notes Importantes**

- **Windows** : Icône automatique si `icon.ico` présent
- **Linux** : AppImage nécessite les droits d'exécution
- **Web** : Fonctionne sur serveurs HTTP/HTTPS (pas file://)
- **Assets** : Dossiers `textures/` et `maps/` automatiquement copiés

## 🎯 **Workflow Complet**

1. **Développement** : `make && make run`
2. **Ajouter icônes** : Copier `icon.ico` et `icon.png`
3. **Export final** : `make build-all`
4. **Distribution** : Partager les dossiers/fichiers de `build/`

C'est tout ! 🚀