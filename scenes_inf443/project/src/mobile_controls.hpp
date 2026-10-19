#pragma once

#include "player.hpp"

// Commandes sur téléphone ou tablette (version web uniquement).
// Les capteurs et l'écran tactile sont lus en JavaScript (scripts/emscripten_template.html),
// qui range le résultat dans l'objet window.tronMobile ; on le relit ici à chaque frame.
// En natif (PC), rien n'est actif.
struct mobile_controls
{
	bool is_active() const;

	// Inclinaison du téléphone (façon volant) pour tourner, toucher à droite / à gauche pour accélérer / freiner
	player_input read() const;

	// Evénements, renvoyés une seule fois chacun
	bool take_tap();           // l'écran a été touché
	int take_start_request();  // difficulté choisie dans le menu du téléphone (-1 si aucune)
	bool take_menu_request();  // bouton "menu" touché pendant la partie
};
