#pragma once

#include "cgp/cgp.hpp"

#include <memory>
#include <string>

enum class sound_effect { explosion, boost, fall, countdown, go };

// Sons du jeu, avec la bibliothèque miniaudio (src/third_party), qui fonctionne sur PC et dans le navigateur.
// Les moteurs des motos sont des sons en boucle placés dans l'espace : volume et stéréo dépendent de la
// position de la caméra (l'auditeur), la hauteur du son dépend de la vitesse, et l'effet Doppler
// fait le reste quand une moto vous croise.
// Si aucune sortie audio n'est disponible, le jeu tourne simplement sans son.
struct audio_system
{
	audio_system();
	~audio_system();

	void initialize(std::string const& sound_directory, int engine_count);

	void set_muted(bool muted);
	bool is_muted() const { return muted; }

	void set_listener(cgp::vec3 const& position, cgp::vec3 const& forward, cgp::vec3 const& velocity);

	// running = false coupe le moteur (moto détruite, écran titre)
	void update_engine(int index, cgp::vec3 const& position, cgp::vec3 const& velocity, bool running);

	void play(sound_effect effect, cgp::vec3 const& position); // placé dans l'espace
	void play(sound_effect effect);                             // son d'interface

private:
	struct implementation; // cache miniaudio.h, un en-tête très lourd
	std::unique_ptr<implementation> impl;
	bool muted = false;
};
