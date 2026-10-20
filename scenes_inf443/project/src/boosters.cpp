#include "boosters.hpp"

using namespace cgp;

namespace
{
	constexpr int max_boosters = 5;
	constexpr float spawn_interval = 4.0f; // s
	constexpr float pickup_radius = 2.0f;
	constexpr float hover_height = 1.2f;
	vec3 const booster_color = { 1.0f, 0.15f, 0.9f };
}

void booster_field::initialize()
{
	ring.initialize_data_on_gpu(mesh_primitive_torus(0.9f, 0.08f, { 0, 0, 0 }, { 1, 0, 0 }));
	ring.material.color = booster_color;
	ring.material.phong = { 1, 0, 0, 1 };

	core.initialize_data_on_gpu(mesh_primitive_sphere(0.35f));
	core.material.color = { 1, 0.8f, 1 };
	core.material.phong = { 1, 0, 0, 1 };
}

void booster_field::clear()
{
	positions.clear();
	spawn_timer = 0;
}

std::vector<size_t> booster_field::update(float dt, level_structure const& level, std::vector<player_structure>& players)
{
	spawn_timer += dt;
	if (spawn_timer > spawn_interval && positions.size() < max_boosters) {
		spawn_timer = 0;

		// On tire quelques positions au hasard et on garde la première qui n'est pas sur un mur
		for (int attempt = 0; attempt < 10; ++attempt) {
			vec3 const p = level.random_point_on_top();
			if (is_free_spot(p, players)) {
				positions.push_back(p);
				break;
			}
		}
	}

	std::vector<size_t> boosted;
	for (size_t i = 0; i < players.size(); ++i) {
		if (!players[i].is_riding())
			continue;
		for (auto it = positions.begin(); it != positions.end(); ) {
			if (norm(players[i].position_3d() - *it) < pickup_radius) {
				players[i].start_boost();
				boosted.push_back(i);
				it = positions.erase(it);
			}
			else
				++it;
		}
	}
	return boosted;
}

bool booster_field::is_free_spot(vec3 const& p, std::vector<player_structure> const& players) const
{
	for (player_structure const& player : players) {
		if (norm(player.position_3d() - p) < 10.0f)
			return false;
		for (vec3 const& q : player.trail.points)
			if (norm(q - p) < 3.0f)
				return false;
	}
	return true;
}

void booster_field::draw(environment_structure const& environment, float time)
{
	for (vec3 const& p : positions) {
		float const bob = 0.15f * std::sin(3.0f * time + p.x);
		vec3 const center = p + vec3{ 0, 0, hover_height + bob };

		ring.model.translation = center;
		ring.model.rotation = rotation_transform::from_axis_angle({ 0, 0, 1 }, 2.0f * time);
		cgp::draw(ring, environment);

		core.model.translation = center;
		cgp::draw(core, environment);
	}
}
