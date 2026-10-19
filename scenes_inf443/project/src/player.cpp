#include "player.hpp"

using namespace cgp;

namespace
{
	constexpr float cruise_speed = 18.0f;
	constexpr float max_speed = 28.0f;
	constexpr float min_speed = 9.0f;
	constexpr float boost_speed = 42.0f;
	constexpr float boost_duration = 2.0f;
	constexpr float speed_response = 1.5f; // vitesse à laquelle on rejoint la vitesse visée (1/s)

	constexpr float air_control = 0.3f;    // fraction du braquage disponible en plein saut
	constexpr float max_lean = 0.45f;      // rad
	constexpr float gravity = 25.0f;
	constexpr float snap_distance = 0.5f;  // en dessous, la moto reste collée au sol (descente de rampe)

	// Rapproche value de target, sans dépasser, avec un taux exponentiel
	float approach(float value, float target, float rate, float dt)
	{
		return value + (target - value) * std::min(1.0f, rate * dt);
	}
}

player_input read_keyboard(inputs_keyboard_parameters const& keyboard, keyboard_layout const& keys)
{
	auto pressed = [&keyboard](int const (&key)[2]) {
		return keyboard.is_pressed(key[0]) || keyboard.is_pressed(key[1]);
	};

	player_input input;
	if (pressed(keys.forward)) input.throttle += 1;
	if (pressed(keys.brake))   input.throttle -= 1;
	if (pressed(keys.left))    input.steer += 1;
	if (pressed(keys.right))   input.steer -= 1;
	return input;
}

void player_structure::initialize(std::string const& name_arg, vec3 const& color_arg, keyboard_layout const& keys_arg, opengl_shader_structure const& trail_shader)
{
	name = name_arg;
	color = color_arg;
	keys = keys_arg;
	trail.initialize(color, trail_shader);
	model.initialize(color);
}

void player_structure::spawn(vec2 const& position_arg, float heading_arg)
{
	status = player_status::riding;
	position = position_arg;
	altitude = 0;
	heading = heading_arg;
	speed = cruise_speed;
	vertical_speed = 0;
	on_ground = true;
	lean = 0;
	pitch = 0;
	boost_timer = 0;
	previous_front = front();
	trail.reset(rear());
	model.set_pose(position_3d(), heading, 0, 0);
}

void player_structure::update(player_input const& input, float dt, level_structure const& level)
{
	if (status == player_status::crashed)
		return;

	if (status == player_status::riding) {
		float target_speed = cruise_speed;
		if (input.throttle > 0) target_speed = max_speed;
		if (input.throttle < 0) target_speed = min_speed;
		if (boost_timer > 0) {
			target_speed = boost_speed;
			boost_timer -= dt;
		}
		speed = approach(speed, target_speed, speed_response, dt);

		float const grip = on_ground ? 1.0f : air_control;
		heading += grip * input.steer * moto_turn_rate * dt;
		lean = approach(lean, input.steer * max_lean, 6.0f, dt);
	}

	previous_front = front();
	position += speed * direction() * dt;
	update_altitude(dt, level);

	if (status == player_status::riding) {
		if (altitude < level.void_altitude)
			status = player_status::falling;
		else
			trail.extend(rear());
	}

	// Le nez suit la trajectoire : montée de rampe, saut, chute
	pitch = approach(pitch, std::atan2(-vertical_speed, std::max(speed, 1.0f)), 12.0f, dt);
	model.set_pose(position_3d(), heading, lean, pitch);
}

void player_structure::update_altitude(float dt, level_structure const& level)
{
	float ground = 0;
	bool const has_ground = level.ground_height(position, altitude, ground);

	// Au sol, la moto suit le relief tant qu'il ne se dérobe pas sous elle
	if (on_ground && has_ground && ground > altitude - snap_distance) {
		vertical_speed = (ground - altitude) / dt;
		altitude = ground;
		return;
	}

	// Sinon elle vole (bout de tremplin, bord d'un étage) jusqu'à retrouver un sol
	on_ground = false;
	vertical_speed -= gravity * dt;
	altitude += vertical_speed * dt;
	if (has_ground && altitude <= ground) {
		altitude = ground;
		vertical_speed = 0;
		on_ground = true;
	}
}

void player_structure::draw(environment_structure const& environment)
{
	if (status != player_status::crashed)
		model.draw(environment);
}

void player_structure::crash()
{
	status = player_status::crashed;
	speed = 0;
}

void player_structure::start_boost()
{
	boost_timer = boost_duration;
	speed = std::max(speed, max_speed);
}

vec2 player_structure::direction() const
{
	return { std::cos(heading), std::sin(heading) };
}

vec3 player_structure::position_3d() const
{
	return { position.x, position.y, altitude };
}

vec3 player_structure::front() const
{
	vec2 const p = position + moto_half_length * direction();
	return { p.x, p.y, altitude };
}

vec3 player_structure::rear() const
{
	vec2 const p = position - moto_half_length * direction();
	return { p.x, p.y, altitude };
}
