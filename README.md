# INF 433

Auteurs: Philibert PAPPENS & Wandrille BOIVIN

🎮 TRON – Le jeu

Le jeu TRON est inspiré de l’univers du film Tron (1982, puis Tron: Legacy en 2010), qui met en scène des personnages plongés dans un monde numérique.

🎲 Concept de jeu :
Le jeu le plus emblématique issu de TRON est appelé "Light Cycle" :
- Chaque joueur contrôle une moto futuriste qui laisse derrière elle une traînée lumineuse.
- Cette traînée agit comme un mur : si un joueur entre en collision avec une traînée, il perd.
- Le but est de piéger les autres joueurs en traçant des lignes et en évitant les collisions.

🎨 Style visuel :
- Grille au sol noire avec des lignes lumineuses bleues, rouges, vertes, etc.
- Esthétique très futuriste, minimaliste, néon.
- Ambiance de cyber-espace numérique.

## A faire

- Texture pour créer la grille: doit-on rajouter de la lumière?
- Voiture qui éclaire le sol avec sa couleur 
- Collisions
- Faire des étages + passerelles + gestion des chutes
- Moto qui se penche quand on tourne

Pour le muret:
- Mesh unité = 2 triangles
- Discrétiser pour les courbes
- version force brute: clear() + réinitialisation du mesh
- belle version: Créer un Mesh de taille fixe + update, pré-allouer une taille fixe de sommets + connectivités (modulo la taille)

Lumières sur le sol:
- Initialiser l'effet qu'elles ont sur le sol à côté
- Utiliser les N plus proches pour l'effet sur la moto