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
