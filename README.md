# LidSleepTray

[![Build (Windows)](https://github.com/francknos/LidSleepTray/actions/workflows/build.yml/badge.svg)](https://github.com/francknos/LidSleepTray/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/github/license/francknos/LidSleepTray)](LICENSE)
![Platform: Windows](https://img.shields.io/badge/platform-Windows%2010%20%7C%2011-0078D6?logo=windows&logoColor=white)
![Qt 6](https://img.shields.io/badge/Qt-6-41CD52?logo=qt&logoColor=white)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
![Last commit](https://img.shields.io/github/last-commit/francknos/LidSleepTray)
[![Release](https://img.shields.io/github/v/release/francknos/LidSleepTray?include_prereleases)](https://github.com/francknos/LidSleepTray/releases)

Petite icône dans la zone de notification Windows (Qt 6) pour :

- **Clic gauche** : bloquer / autoriser la mise en veille automatique (☀️ / 🌙).
- **Clic droit** : ouvrir un petit panneau pour choisir l'action à la **fermeture du capot**
  (*Ne rien faire* ou *Veille*), séparément **sur secteur** et **sur batterie**.
  Les choix sont mémorisés et l'application bascule d'une configuration à l'autre
  quand l'ordinateur passe du secteur à la batterie (et inversement).
- Option **Lancer au démarrage de Windows**.
- Bouton pour **rétablir les réglages Windows d'origine** (sauvegardés au premier lancement).

Pas de droits administrateur nécessaires : l'app modifie le plan d'alimentation de l'utilisateur
via l'API `PowerWriteACValueIndex` / `PowerWriteDCValueIndex` (réglage *Lid close action*).

## Installation
Télécharger `LidSleepTray-win64.zip` dans les *Releases* (ou l'artefact de la dernière CI), décompresser, lancer `LidSleepTray.exe`.

## Compiler
```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
Prérequis : Qt 6 (Widgets), CMake ≥ 3.21, MSVC.

## Notes
- La bascule "veille bloquée" utilise `SetThreadExecutionState` (`ES_SYSTEM_REQUIRED` + `ES_DISPLAY_REQUIRED`) : elle empêche la veille automatique **et garde l'écran allumé**, elle ne change pas l'action du capot (c'est le rôle du panneau).
- Certains PC gérés par stratégie de groupe interdisent la modification du plan d'alimentation.

Licence MIT.
