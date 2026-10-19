#include "ai_driver.hpp"
#include "geometry.hpp"

using namespace cgp;

namespace
{
	struct ai_settings
	{
		float reaction_time; // temps entre deux décisions (s)
		float look_ahead;    // distance d'anticipation (m)
		float aggression;    // envie de couper la route de l'adversaire
		bool likes_boosts;
		bool brakes;         // sait freiner pour tourner plus court
	};

	ai_settings settings_for(ai_difficulty difficulty)
	{
		switch (difficulty) {
		case ai_difficulty::easy: return { 0.35f, 16.0f, 0.0f, false, false };
		case ai_difficulty::hard: return { 0.05f, 50.0f, 1.0f, true, true };
		default:                  return { 0.15f, 30.0f, 0.3f, true, true };
		}
	}

	constexpr float simulation_step = 1.0f; // m
	constexpr float candidate_steers[] = { 0.0f, 0.35f, -0.35f, 0.7f, -0.7f, 1.0f, -1.0f };
}

void ai_driver::reset()
{
	decision_timer = 0;
	current = player_input();
}

player_input ai_driver::decide(player_structure const& self, std::vector<player_structure> const& players,
                               level_structure const& level, booster_field const& boosters, float dt)
{
	// En plein saut le braquage ne sert presque à rien : on garde la décision précédente
	decision_timer -= dt;
	if (!self.is_riding() || !self.on_ground || decision_timer > 0)
		return current;

	ai_settings const settings = settings_for(difficulty);
	decision_timer = settings.reaction_time;

	player_structure const* opponent = nullptr;
	for (player_structure const& p : players)
		if (&p != &self && p.is_riding())
			opponent = &p;

	float best_score = -1e9f;
	float best_steer = 0;
	float best_free = 0;

	for (float steer : candidate_steers) {
		vec2 end;
		float const free = free_distance(self, steer, settings.look_ahead, players, level, end);
		float score = free;

		// Trajectoire sûre : on peut penser à autre chose qu'à survivre
		if (free >= settings.look_ahead) {
			if (opponent != nullptr) {
				// Se placer là où l'adversaire sera quand on arrivera au bout de la trajectoire
				float const time_to_end = settings.look_ahead / std::max(self.speed, 1.0f);
				vec2 const opponent_later = opponent->position + opponent->speed * time_to_end * opponent->direction();
				score += 15.0f * settings.aggression * std::max(0.0f, 1 - norm(end - opponent_later) / 40.0f);
			}
			if (settings.likes_boosts) {
				for (vec3 const& b : boosters.positions)
					if (std::abs(b.z - self.altitude) < 2.0f)
						score += 10.0f * std::max(0.0f, 1 - norm(end - xy(b)) / 50.0f);
			}
		}

		// A score égal, on préfère aller tout droit et ne pas changer d'avis (sinon la moto zigzague)
		score -= 1.5f * std::abs(steer);
		if (steer == current.steer)
			score += 0.5f;

		if (score > best_score) {
			best_score = score;
			best_steer = steer;
			best_free = free;
		}
	}

	current.steer = best_steer;
	if (settings.brakes && best_free < 0.35f * settings.look_ahead)
		current.throttle = -1; // freiner pour tourner plus court
	else if (settings.aggression > 0.5f)
		current.throttle = 1;
	else
		current.throttle = 0;

	return current;
}

float ai_driver::free_distance(player_structure const& self, float steer, float look_ahead,
                               std::vector<player_structure> const& players, level_structure const& level,
                               vec2& end_position) const
{
	float const turn_per_meter = steer * moto_turn_rate / std::max(self.speed, 1.0f);

	vec2 position = self.position;
	float heading = self.heading;
	float altitude = self.altitude;
	vec3 front = self.front();
	end_position = position;

	for (float d = simulation_step; d <= look_ahead; d += simulation_step) {
		heading += turn_per_meter * simulation_step;
		vec2 const direction = { std::cos(heading), std::sin(heading) };
		position += simulation_step * direction;

		// Le vide, ou le flanc d'une rampe / d'un étage
		float ground;
		if (!level.ground_height(position, altitude, ground))
			return d;
		vec2 const front_xy = position + moto_half_length * direction;
		if (level.blocks(front_xy, altitude, moto_height))
			return d;
		altitude = ground;

		// Les murs de lumière existants
		vec3 const next_front = { front_xy.x, front_xy.y, altitude };
		for (player_structure const& p : players) {
			int const ignored_segments = (&p == &self) ? 2 : 0;
			if (p.trail.hits(front, next_front, moto_height, ignored_segments))
				return d;
		}

		// La moto adverse et le chemin qu'elle va parcourir dans la seconde qui vient
		for (player_structure const& p : players) {
			if (&p == &self || !p.is_riding() || std::abs(p.altitude - altitude) > moto_height)
				continue;
			vec2 const a = xy(p.front());
			vec2 const b = a + p.speed * p.direction();
			float t, u;
			if (segments_intersect(xy(front), front_xy, a, b, t, u))
				return d;
		}

		front = next_front;
		end_position = position;
	}
	return look_ahead;
}
