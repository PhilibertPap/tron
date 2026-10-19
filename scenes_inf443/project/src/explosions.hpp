#pragma once

#include "cgp/cgp.hpp"
#include "environment.hpp"

#include <vector>

// Gerbes d'éclats lumineux quand une moto s'écrase
struct explosion_effects
{
	void initialize();
	void spawn(cgp::vec3 const& center, cgp::vec3 const& color);
	void update(float dt);
	void draw(environment_structure const& environment);
	void clear();

private:
	struct particle {
		cgp::vec3 position;
		cgp::vec3 velocity;
		cgp::vec3 color;
		float life; // durée restante (s)
	};
	std::vector<particle> particles;
	cgp::mesh_drawable shard;
};
