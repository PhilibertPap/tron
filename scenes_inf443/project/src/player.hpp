#pragma once

#include "cgp/cgp.hpp"
#include "environment.hpp"
#include "level.hpp"
#include "moto_model.hpp"
#include "trail.hpp"

#include <string>

// Commandes d'un joueur pour une frame.
// La moto ne sait pas d'où elles viennent : clavier aujourd'hui, réseau ou gyroscope demain.
struct player_input
{
	float throttle = 0; // +1 : accélérer, -1 : freiner
	float steer = 0;    // +1 : tourner à gauche, -1 : à droite
};

// Deux touches possibles par action (codes GLFW_KEY_...). Sur PC, GLFW donne la position physique
// de la touche (W = Z en AZERTY) ; dans le navigateur, c'est la lettre : on accepte donc les deux.
struct keyboard_layout
{
	int forward[2], brake[2], left[2], right[2];
};

player_input read_keyboard(cgp::inputs_keyboard_parameters const& keyboard, keyboard_layout const& keys);

// riding : en course (au sol ou en plein saut) ; falling : tombée dans le vide ; crashed : détruite
enum class player_status { riding, falling, crashed };

constexpr float moto_height = 1.2f;

struct player_structure
{
	std::string name;
	cgp::vec3 color;
	keyboard_layout keys;
	int score = 0;

	// Etat de la moto
	player_status status = player_status::riding;
	cgp::vec2 position;   // position du centre de la moto, vue du dessus
	float altitude = 0;
	float heading = 0;    // cap dans le plan (rad)
	float speed = 0;      // vitesse horizontale
	float vertical_speed = 0;
	bool on_ground = true;
	float lean = 0;       // inclinaison dans les virages
	float pitch = 0;      // nez vers le bas > 0 (descente, chute)
	float boost_timer = 0;
	cgp::vec3 previous_front; // position de l'avant à la frame précédente (collisions)

	trail_structure trail;
	moto_model model;

	void initialize(std::string const& name, cgp::vec3 const& color, keyboard_layout const& keys, cgp::opengl_shader_structure const& trail_shader);
	void spawn(cgp::vec2 const& position, float heading);
	void update(player_input const& input, float dt, level_structure const& level);
	void draw(environment_structure const& environment);

	void crash();
	void start_boost();

	bool is_riding() const { return status == player_status::riding; }
	cgp::vec2 direction() const;
	cgp::vec3 position_3d() const;
	cgp::vec3 front() const;
	cgp::vec3 rear() const;

private:
	void update_altitude(float dt, level_structure const& level);
};
