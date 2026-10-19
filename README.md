# TRON 3D — Light Cycles

Projet INF443 — Philibert Pappens & Wandrille Boivin

Jeu de motos de lumière inspiré de *Tron* (1982) et *Tron: Legacy* (2010), en C++ avec la bibliothèque CGP.
Chaque moto laisse derrière elle un mur de lumière : toucher un mur, le décor ou tomber dans le vide fait perdre la manche.
Tron (bleu) affronte Clu (orange) : en solo, Clu est piloté par l'ordinateur ; à deux, chacun prend une moto sur le même clavier.
Contrairement au jeu d'origine en 2D, l'arène est en 3D : étages, rampes, passerelles et tremplins.

Version en ligne : https://philibertpap.github.io/tron/

## Commandes

| | Tron (bleu) | Clu (orange, à deux joueurs) |
|---|---|---|
| Accélérer / freiner | Z / S | ↑ / ↓ |
| Tourner | Q / D | ← / → |

En QWERTY, Tron utilise W A S D (mêmes positions sur le clavier). En solo, ZQSD et les flèches pilotent toutes les deux Tron.

**Sur téléphone** (version web) : choisir la difficulté, puis tenir le téléphone à l'horizontale comme un volant. On le penche pour tourner, doigt à droite de l'écran pour accélérer, à gauche pour freiner.

- `Entrée` : lancer la partie
- `C` : changer de vue (globale, 3e personne, 1re personne)
- `F11` : plein écran
- `Échap` : retour au menu, puis quitter

## Règles

- Solo : trois niveaux de difficulté pour Clu (Facile, Normal, Difficile).
- Le premier à `N` points gagne (réglable sur l'écran titre, ainsi que la longueur des murs).
- On peut passer sous un étage, sous un mur posé à l'étage, et sauter par-dessus un mur avec un tremplin.
- Les anneaux roses donnent un boost de vitesse pendant 2 s.

## Compiler et lancer

**Windows (Visual Studio)** : lancer `scenes_inf443/project/scripts/windows_cmake_visual.bat`, ouvrir `build/project.sln`, puis F5.

**Version autonome** : `scripts/windows_package.bat` crée un dossier `TRON/` (exécutable sans console + shaders + assets), à zipper et lancer par double-clic sur `TRON.exe`.

**Linux** : `python3 scripts/linux_compile_run_cmake.py` (nécessite `libglfw3-dev`).

**Web** : chaque push sur `main` compile le jeu en WebAssembly et le publie sur GitHub Pages (`.github/workflows/web.yml`).

## Organisation du code (`scenes_inf443/project/src`)

| Fichier | Rôle |
|---|---|
| `scene` | Boucle de jeu : états (menu, compte à rebours, manche, fin de partie), collisions, caméras, rendu, interface |
| `player` | Moto d'un joueur : commandes, vitesse, boost, physique (rampes, sauts, chute) |
| `ai_driver` | Pilote automatique : simule plusieurs trajectoires et garde la plus sûre |
| `mobile_controls` | Commandes sur téléphone (inclinaison et tactile, lues en JavaScript) |
| `geometry` | Intersection de segments |
| `trail` | Mur de lumière : tracé, mesh pré-alloué mis à jour à chaque frame, collisions 3D |
| `level` | Arène : sol, étages, rampes, tremplins, et requêtes « hauteur du sol » / « obstacle » |
| `moto_model` | Modèle 3D de la moto (hiérarchie CGP), inclinaison dans les virages |
| `boosters` | Bonus de vitesse |
| `explosions` | Particules quand une moto s'écrase |
| `bloom` | Effet de lueur en post-traitement (rendu hors écran + flou gaussien) |

Shaders (`shaders/`) : `mesh` (Phong éclairé par les motos), `ground` (grille procédurale), `trail` (murs lumineux), `post` (bloom).

## Idées pour la suite

- Multijoueur en réseau local
- Deux joueurs sur une même tablette (une moitié d'écran chacun)
- Comptes joueurs pour garder sa progression

## Crédits

Polices Orbitron et Rajdhani (SIL Open Font License, voir `assets/fonts/`).
