Write-Host "🚀 POLYCAST ENGINE BUILD SYSTEM COMPLET" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan

# Configuration adaptée à votre structure
$compiler = "mingw64\bin\gcc.exe"
$windres = "mingw64\bin\windres.exe"
$buildDir = "build"
$srcDir = "src"
$editorDir = "editor"
$assetsDirs = @("build/textures", "build/maps")


# Vérifications initiales
function Test-Environment {
    Write-Host "🔍 Vérification de l'environnement..." -ForegroundColor Yellow
    
    $issues = @()
    
    if (-not (Test-Path $compiler)) {
        $issues += "❌ Compilateur non trouvé: $compiler"
    } else {
        Write-Host "✅ Compilateur trouvé" -ForegroundColor Green
    }
    
    if (-not (Test-Path "libs\SDL2\lib\libSDL2.a")) {
        $issues += "❌ Bibliothèques SDL2 non trouvées"
    } else {
        Write-Host "✅ Bibliothèques SDL2 trouvées" -ForegroundColor Green
    }
    
    if (-not (Test-Path "$srcDir\main.c")) {
        $issues += "❌ Fichiers source non trouvés"
    } else {
        Write-Host "✅ Fichiers source trouvés" -ForegroundColor Green
    }
    
    if ($issues.Count -gt 0) {
        Write-Host "`nProblèmes détectés:" -ForegroundColor Red
        $issues | ForEach-Object { Write-Host $_ -ForegroundColor Red }
        return $false
    }
    
    Write-Host "✅ Environnement OK" -ForegroundColor Green
    return $true
}

# Création des dossiers
function New-BuildDirectories {
    if (-not (Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir | Out-Null
    }
}

# Compilation du moteur
function Build-Engine {
    Write-Host "🔨 Compilation du moteur..." -ForegroundColor Yellow
    
    New-BuildDirectories
    
    $args = @(
        "-mconsole", "-fdiagnostics-color=always", "-g",
        "-I$PWD\libs\SDL2\include",
        "-I$PWD\libs\SDL2\include\SDL2", 
        "-I$PWD\libs\SDL2_image\include",
        "-I$PWD\libs\SDL2_image\include\SDL2",
        "-L$PWD\libs\SDL2\lib",
        "-L$PWD\libs\SDL2_image\lib",
        "$srcDir\main.c",
        "$srcDir\engine.c", 
        "$srcDir\input.c",
        "$srcDir\raycaster.c",
        "$srcDir\player.c",
        "$srcDir\textures.c", 
        "$srcDir\map.c",
        "$srcDir\map_loader.c",
        "$editorDir\lighting.c",
        "-o", "$buildDir\engine.exe",
        "-lSDL2main", "-lSDL2", "-lSDL2_image",
        "-lgdi32", "-lshell32", "-lshlwapi", "-lcomctl32",
        "-lsetupapi", "-limm32", "-lole32", "-loleaut32", "-lversion", "-luuid", "-lwinmm", "-ldxguid"

    )

    
    & $compiler $args
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ Compilation réussie!" -ForegroundColor Green
        return $true
    } else {
        Write-Host "❌ Erreur de compilation!" -ForegroundColor Red
        return $false
    }
}

# Compilation de l'éditeur
function Build-Editor {
    Write-Host "🎨 Compilation de l'éditeur..." -ForegroundColor Yellow
    
    if (-not (Test-Path "$editorDir\map_editor.c")) {
        Write-Host "❌ Éditeur source non trouvé" -ForegroundColor Red
        return $false
    }
    
    $args = @(
        "-mconsole", "-fdiagnostics-color=always", "-g",
        "-I$PWD\libs\SDL2\include",
        "-I$PWD\libs\SDL2\include\SDL2", 
        "-I$PWD\libs\SDL2_image\include",
        "-I$PWD\libs\SDL2_image\include\SDL2",
        "-L$PWD\libs\SDL2\lib",
        "-L$PWD\libs\SDL2_image\lib",
        "$editorDir\map_editor.c",
        "$editorDir\lighting.c",
        "-o", "$buildDir\map_editor.exe",
        "-lSDL2main", "-lSDL2", "-lSDL2_image",
        "-lgdi32", "-lshell32", "-lshlwapi", "-lcomctl32",
        "-lsetupapi", "-limm32", "-lole32", "-loleaut32", "-lversion", "-luuid", "-lwinmm", "-ldxguid"
    )
    
    & $compiler $args
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ Éditeur compilé!" -ForegroundColor Green
        return $true
    } else {
        Write-Host "❌ Erreur compilation éditeur!" -ForegroundColor Red
        return $false
    }
}

# Build Windows avec icône
function Build-WindowsPackage {
    Write-Host "📦 Création du package Windows..." -ForegroundColor Yellow
    
    $windowsDir = "$buildDir\windows"
    if (Test-Path $windowsDir) {
        Remove-Item $windowsDir -Recurse -Force
    }
    New-Item -ItemType Directory -Path $windowsDir | Out-Null
    
    # Créer dossier resources si nécessaire
    if (-not (Test-Path "resources")) {
        New-Item -ItemType Directory -Path "resources" | Out-Null
    }
    
    # Gestion de l'icône
    $useIcon = $false
    $resourceFile = ""
    
    if (Test-Path "icon.ico") {
        Write-Host "🎨 Création des ressources avec icône..." -ForegroundColor Yellow
        
        # Copier l'icône dans resources
        Copy-Item "icon.ico" "resources\"
        
        # Créer le fichier RC
        $rcContent = @"
#include <windows.h>

// Icône principale
IDI_MAIN_ICON ICON "icon.ico"

// Informations de version
VS_VERSION_INFO VERSIONINFO
FILEVERSION 1,0,0,0
PRODUCTVERSION 1,0,0,0
FILEFLAGSMASK 0x3fL
FILEFLAGS 0x0L
FILEOS 0x40004L
FILETYPE 0x1L
FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "CompanyName", "Nahos Production"
            VALUE "FileDescription", "Polycast Engine"
            VALUE "FileVersion", "1.0.0.0"
            VALUE "InternalName", "engine"
            VALUE "LegalCopyright", "© 2025 Nahos Production"
            VALUE "OriginalFilename", "engine.exe"
            VALUE "ProductName", "Polycast Engine"
            VALUE "ProductVersion", "1.0.0.0"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END
"@
        
        $rcContent | Out-File "resources\game.rc" -Encoding ASCII
        
        # Compiler les ressources
        if (Test-Path $windres) {
            & $windres "resources\game.rc" "-O" "coff" "-o" "$windowsDir\game.res"
            if ($LASTEXITCODE -eq 0) {
                $useIcon = $true
                $resourceFile = "$windowsDir\game.res"
                Write-Host "✅ Ressources avec icône créées" -ForegroundColor Green
            }
        }
    }
    
    # Compiler le moteur avec icône
    Write-Host "🔨 Compilation moteur Windows..." -ForegroundColor Yellow
    $args = @(
        "-mconsole", "-g", "-O2", "-static",
        "-I$PWD\libs\SDL2\include",
        "-I$PWD\libs\SDL2\include\SDL2", 
        "-I$PWD\libs\SDL2_image\include",
        "-I$PWD\libs\SDL2_image\include\SDL2",
        "-L$PWD\libs\SDL2\lib",
        "-L$PWD\libs\SDL2_image\lib",
        "$srcDir\main.c",
        "$srcDir\engine.c", 
        "$srcDir\input.c",
        "$srcDir\raycaster.c",
        "$srcDir\player.c",
        "$srcDir\textures.c", 
        "$srcDir\map.c",
        "$srcDir\map_loader.c",
        "$editorDir\lighting.c"
    )
    
    if ($useIcon) {
        $args += $resourceFile
    }
    
    $args += @(
        "-o", "$windowsDir\engine.exe",
        "-lSDL2main", "-lSDL2", "-lSDL2_image",
        "-lgdi32", "-lshell32", "-lshlwapi", "-lcomctl32",
        "-lsetupapi", "-limm32", "-lole32", "-loleaut32", "-lversion", "-luuid", "-lwinmm", "-ldxguid"
    )
    
    & $compiler $args
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Erreur compilation moteur Windows" -ForegroundColor Red
        return $false
    }
    
    # Compiler l'éditeur avec icône
    if (Test-Path "$editorDir\map_editor_with_lights.c") {
        Write-Host "🎨 Compilation éditeur Windows..." -ForegroundColor Yellow
        $editorArgs = @(
            "-mconsole", "-g", "-O2", "-static",
            "-I$PWD\libs\SDL2\include",
            "-I$PWD\libs\SDL2\include\SDL2", 
            "-I$PWD\libs\SDL2_image\include",
            "-I$PWD\libs\SDL2_image\include\SDL2",
            "-L$PWD\libs\SDL2\lib",
            "-L$PWD\libs\SDL2_image\lib",
            "$editorDir\map_editor_with_lights.c",
            "$editorDir\lighting.c"
        )
        
        if ($useIcon) {
            $editorArgs += $resourceFile
        }
        
        $editorArgs += @(
            "-o", "$windowsDir\map_editor.exe",
            "-lSDL2main", "-lSDL2", "-lSDL2_image",
            "-lgdi32", "-lshell32", "-lshlwapi", "-lcomctl32"
        )
        
        & $compiler $editorArgs
    }
    
    # Copier les DLL SDL
    Write-Host "📚 Copie des bibliothèques..." -ForegroundColor Yellow
    if (Test-Path "libs\SDL2\lib\SDL2.dll") {
        Copy-Item "libs\SDL2\lib\SDL2.dll" $windowsDir
    }
    if (Test-Path "libs\SDL2_image\lib\SDL2_image.dll") {
        Copy-Item "libs\SDL2_image\lib\SDL2_image.dll" $windowsDir
    }
        
    # Copier les assets
    foreach ($dir in $assetsDirs) {
        if (Test-Path $dir) {
            Copy-Item $dir $windowsDir -Recurse -Force
            Write-Host "🎨 Assets copiés depuis $dir" -ForegroundColor Green
        } else {
            Write-Host "⚠️ Dossier $dir introuvable" -ForegroundColor Yellow
        }
    }

    # Créer le lanceur
    $launcherContent = @"
@echo off
echo POLYCAST ENGINE
echo ========================
echo.
echo Choisissez:
echo 1. Lancer le jeu
echo 2. Lancer l'editeur de map
echo 3. Quitter
echo.
set /p choice="Votre choix (1-3): "

if "%choice%"=="1" (
    start engine.exe
) else if "%choice%"=="2" (
    start map_editor.exe
) else if "%choice%"=="3" (
    exit
) else (
    echo Choix invalide!
    pause
    goto :eof
)
"@
    
    $launcherContent | Out-File "$windowsDir\launch.bat" -Encoding ASCII
    
    # Créer le README
    $readmeContent = @"
POLYCAST ENGINE v1.0
=============================

CONTENU DU PACKAGE:
- engine.exe        : Le moteur de jeu
- map_editor.exe    : L'éditeur de niveau
- textures/         : Dossier des textures
- maps/             : Dossier des maps
- launch.bat        : Lanceur simplifié

UTILISATION:
1. Double-cliquez sur launch.bat
2. Ou lancez directement engine.exe

CONTRÔLES JEU:
- WASD : Mouvement
- Q/E : Mouvement latéral
- O : Toggle éclairage
- L : Charger niveau
- ESC : Quitter

ÉDITEUR:
- TAB : Changer de mode
- S : Sauvegarder
- N : Nouvelle map

Support: nahosproduction@gmail.com
"@
    
    $readmeContent | Out-File "$windowsDir\README.txt" -Encoding UTF8
    
    Write-Host "✅ Package Windows créé dans $windowsDir" -ForegroundColor Green
    if ($useIcon) {
        Write-Host "🎨 Avec icône intégrée!" -ForegroundColor Green
    }
    
    return $true
}


# Menu principal
do {
    Write-Host ""
    Write-Host "Choisissez une option:" -ForegroundColor White
    Write-Host "1. Test environnement" -ForegroundColor White
    Write-Host "2. Compiler le moteur" -ForegroundColor White
    Write-Host "3. Compiler et lancer" -ForegroundColor White
    Write-Host "4. Compiler éditeur" -ForegroundColor White  
    Write-Host "5. Build package Windows (avec icône)" -ForegroundColor White
    Write-Host "6. Nettoyer" -ForegroundColor White
    Write-Host "7. Quitter" -ForegroundColor White
    
    $choice = Read-Host "Votre choix (1-7)"
    
    switch ($choice) {
        "1" { 
            Test-Environment | Out-Null
        }
        "2" { 
            Build-Engine | Out-Null
        }
        "3" { 
            if (Build-Engine) {
                Write-Host "🎮 Lancement du jeu..." -ForegroundColor Cyan
                Set-Location $buildDir
                & ".\engine.exe"
                Set-Location ..
            }
        }
        "4" {
            Build-Editor | Out-Null
        }
        "5" {
            Build-WindowsPackage | Out-Null
        }
        "6" {
            Write-Host "🧹 Nettoyage..." -ForegroundColor Yellow
            if (Test-Path "$buildDir\engine.exe") { Remove-Item "$buildDir\engine.exe" }
            if (Test-Path "$buildDir\map_editor.exe") { Remove-Item "$buildDir\map_editor.exe" }
            if (Test-Path "$buildDir\windows") { Remove-Item "$buildDir\windows" -Recurse -Force }
            if (Test-Path "$buildDir\web") { Remove-Item "$buildDir\web" -Recurse -Force }
            if (Test-Path "resources") { Remove-Item "resources" -Recurse -Force }
            Write-Host "✅ Nettoyage terminé!" -ForegroundColor Green
        }
        "7" { 
            Write-Host "Au revoir!" -ForegroundColor Cyan
            exit 
        }
        default { 
            Write-Host "Choix invalide!" -ForegroundColor Red 
        }
    }
} while ($choice -ne "7")