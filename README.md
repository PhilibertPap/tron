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
- Faire des étages + passerelles + gestion des chutes
- Moto qui se penche quand on tourne

Trail:
- Mesh unité = 2 triangles
- Discrétiser pour les courbes
- version force brute: clear() + réinitialisation du mesh -> fait
- belle version: Créer un Mesh de taille fixe + update, pré-allouer une taille fixe de sommets + connectivités (modulo la taille)
- Améliorer le mesh (surtout la couleur, lumière etc)

Lumières sur le sol:
- Disposées à intervalles réguliers (à chaque coin de quadrangle)
- Initialiser l'effet qu'elles ont sur le sol à côté (toujours pas fait)
- Utiliser les N plus proches pour l'effet sur la moto 
-> Problème avec la moto 1 : on ne ovit pas les lumières s'allumer quand elle passe

Gestion des collisions:
- Avec n'importe quel muret
- Dès que la moto touche le muret, elle disparait ou la simulation s'arrête

Boosters:
- Disparaissent lorsqu'on les touche
- Apparaissent aléatoirement sur la grille (pas plus de 5 boosters)
- Pourquoi pas ajouter des particules lorsuq'ils disparaissent

Gestion des sorties de terrain:
- Chute libre lorsque la moto sort du terrain
- Si elle tombe en dehors de la grille: reset et l'adversaire gagne un point

Moto:
- Changer le style de la moto
- Faire avancer sur un chemin prédéfini / au hasard / nous-mêmes
- Nous avons fait en sorte d'utiliser deux motos qui avancent avec les touches du clavier

Skybox:
- Utilisation d'une image de galaxies
- Problème d'orientation pour d'autres images, mais celle-ci n'en a pas vraiment donc ok

Collisions et fin du jeu:
- GUI
- Prise en compte des collisions : ok
- Appuyer sur espace pour reset complètement