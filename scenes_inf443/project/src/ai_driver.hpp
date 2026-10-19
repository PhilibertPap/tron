#pragma once

#include "cgp/cgp.hpp"
#include "boosters.hpp"
#include "level.hpp"
#include "player.hpp"

#include <vector>

enum class ai_difficulty { easy, normal, hard };

// Pilote automatique d'une moto.
// A intervalles réguliers, l'IA essaie plusieurs braquages. Pour chacun, elle simule la trajectoire
// de la moto sur quelques dizaines de mètres et mesure la distance parcourue avant le premier obstacle
// (mur de lumière, flanc de rampe, vide, moto adverse). Elle garde le braquage le plus sûr, avec un
// bonus pour aller chercher les boosts et, plus elle est forte, pour couper la route de l'adversaire.
struct ai_driver
{
	ai_difficulty difficulty = ai_difficulty::normal;

	void reset();
	player_input decide(player_structure const& self, std::vector<player_structure> const& players,
	                    level_structure const& level, booster_field const& boosters, float dt);

private:
	float decision_timer = 0;
	player_input current;

	// Distance (m) que la moto peut parcourir en gardant ce braquage avant de heurter quelque chose
	float free_distance(player_structure const& self, float steer, float look_ahead,
	                    std::vector<player_structure> const& players, level_structure const& level,
	                    cgp::vec2& end_position) const;
};
