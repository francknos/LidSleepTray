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
  Cet état est mémorisé séparément **sur secteur** et **sur batterie**, puis rétabli
  automatiquement lors d'un changement d'alimentation et au prochain lancement.
  Par défaut, la veille reste autorisée pour chaque alimentation.
- **Clic droit** : ouvrir un petit panneau pour choisir l'action à la **fermeture du capot**
  (*Ne rien faire* ou *Veille*), séparément **sur secteur** et **sur batterie**.
  Les choix sont mémorisés et l'application bascule d'une configuration à l'autre
  quand l'ordinateur passe du secteur à la batterie (et inversement).
- Option **Lancer au démarrage de Windows**.
- Icône soleil / lune dans le panneau pour choisir un thème clair ou sombre. Le choix
  est mémorisé pour les prochains lancements ; le thème sombre est utilisé par défaut.
- Bouton pour **rétablir les réglages Windows d'origine** (sauvegardés au premier lancement).

## Aperçu

| Thème sombre | Thème clair |
|:--:|:--:|
| <img src="docs/screenshots/popup-dark.png" alt="Panneau LidSleepTray en thème sombre" width="340"> | <img src="docs/screenshots/popup-light.png" alt="Panneau LidSleepTray en thème clair" width="340"> |

Pas de droits administrateur nécessaires : l'app modifie le plan d'alimentation de l'utilisateur
via l'API `PowerWriteACValueIndex` / `PowerWriteDCValueIndex` (réglage *Lid close action*).

## Installation
Télécharger `LidSleepTray-win64.zip` dans les *Releases* (ou l'artefact de la dernière CI), décompresser, lancer `LidSleepTray.exe`.

## Mises à jour
Au démarrage, LidSleepTray vérifie en arrière-plan les versions stables publiées sur GitHub.
Si une version plus récente est disponible, tu peux choisir **Plus tard** ou **Télécharger et installer**.
Dans ce dernier cas, l'application vérifie l'empreinte SHA-256 du fichier avant de lancer
l'installateur Windows. Si tu utilises la version ZIP, l'installateur ne remplace pas
les fichiers du dossier portable : pour rester en mode portable, récupère plutôt le nouveau ZIP.

## Compiler
```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
Prérequis : Qt 6 (Widgets et Network), CMake ≥ 3.21, MSVC.

Pour compiler et exécuter les tests :
```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DLIDSLEEPTRAY_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
Pour régénérer les captures du README : `build\SettingsPopupTest.exe --capture-screenshots docs\screenshots`.
Pour régénérer l'icône Windows : `python assets\make_app_icon.py`.

## Notes
- La bascule "veille bloquée" utilise `SetThreadExecutionState` (`ES_SYSTEM_REQUIRED` + `ES_DISPLAY_REQUIRED`) : elle empêche la veille automatique **et garde l'écran allumé**, elle ne change pas l'action du capot (c'est le rôle du panneau).
- Certains PC gérés par stratégie de groupe interdisent la modification du plan d'alimentation.

Licence MIT.
