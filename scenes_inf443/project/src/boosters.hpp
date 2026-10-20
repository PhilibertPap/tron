#pragma once

#include "cgp/cgp.hpp"
#include "environment.hpp"
#include "level.hpp"
#include "player.hpp"

#include <vector>

// Bonus de vitesse qui apparaissent au hasard sur le plateau
struct booster_field
{
	std::vector<cgp::vec3> positions;

	void initialize();
	void clear();

	// Fait apparaître de nouveaux bonus et donne le boost aux motos qui passent dessus.
	// Renvoie les indices des joueurs qui viennent de prendre un boost.
	std::vector<size_t> update(float dt, level_structure const& level, std::vector<player_structure>& players);
	void draw(environment_structure const& environment, float time);

private:
	float spawn_timer = 0;
	cgp::mesh_drawable ring;
	cgp::mesh_drawable core;

	bool is_free_spot(cgp::vec3 const& p, std::vector<player_structure> const& players) const;
};
