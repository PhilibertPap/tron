#include "explosions.hpp"

#include <algorithm>

using namespace cgp;

namespace
{
	constexpr int particles_per_explosion = 160;
	constexpr float particle_life = 1.6f;
	constexpr float gravity = 12.0f;
}

void explosion_effects::initialize()
{
	shard.initialize_data_on_gpu(mesh_primitive_cube({ 0, 0, 0 }, 0.18f));
	shard.material.phong = { 1, 0, 0, 1 }; // éclats lumineux, non éclairés
}

void explosion_effects::spawn(vec3 const& center, vec3 const& color)
{
	for (int k = 0; k < particles_per_explosion; ++k) {
		// Direction aléatoire sur la demi-sphère supérieure
		float const theta = rand_interval(0, 2 * Pi);
		float const z = rand_interval(0.1f, 1.0f);
		float const r = std::sqrt(1 - z * z);
		vec3 const dir = { r * std::cos(theta), r * std::sin(theta), z };

		particle p;
		p.position = center + vec3{ 0, 0, 0.6f };
		p.velocity = rand_interval(4.0f, 16.0f) * dir;
		p.color = (k % 4 == 0) ? vec3{ 1, 1, 1 } : color;
		p.life = rand_interval(0.5f, 1.0f) * particle_life;
		particles.push_back(p);
	}
}

void explosion_effects::update(float dt)
{
	for (particle& p : particles) {
		p.velocity.z -= gravity * dt;
		p.velocity *= (1 - 1.5f * dt); // frottement de l'air
		p.position += p.velocity * dt;
		p.life -= dt;
	}

	particles.erase(std::remove_if(particles.begin(), particles.end(),
		[](particle const& p) { return p.life <= 0; }), particles.end());
}

void explosion_effects::draw(environment_structure const& environment)
{
	for (particle const& p : particles) {
		shard.model.translation = p.position;
		shard.model.scaling = std::min(1.0f, p.life / 0.4f); // rétrécit avant de disparaître
		shard.material.color = p.color;
		cgp::draw(shard, environment);
	}
}

void explosion_effects::clear()
{
	particles.clear();
}
