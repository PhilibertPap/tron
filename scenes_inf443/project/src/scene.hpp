#pragma once

#include "cgp/cgp.hpp"
#include "environment.hpp"

#include "bloom.hpp"
#include "boosters.hpp"
#include "explosions.hpp"
#include "level.hpp"
#include "player.hpp"

#include <string>
#include <vector>

enum class game_state { title, countdown, playing, round_over, match_over };

enum class camera_mode { overview, third_person, first_person };

// Options réglables depuis l'écran titre
struct game_options
{
	camera_mode camera = camera_mode::third_person;
	int points_to_win = 5;
	float wall_length = 120.0f;
	bool bloom = true;
};

// Caméra qui suit une moto, avec un léger retard pour adoucir les virages
struct follow_camera
{
	cgp::vec3 eye;
	cgp::vec3 target;

	void snap(player_structure const& player, camera_mode mode);
	void update(player_structure const& player, camera_mode mode, float dt);
	cgp::mat4 view_matrix() const;
};

struct scene_structure : cgp::scene_inputs_generic
{
	window_structure window;
	input_devices inputs;
	environment_structure environment;
	camera_projection_perspective camera_projection;
	camera_controller_orbit_euler camera_control; // vue globale, orientable à la souris

	game_options options;
	game_state state = game_state::title;
	float state_time = 0; // temps passé dans l'état courant
	std::string round_message;

	level_structure level;
	std::vector<player_structure> players;
	std::vector<follow_camera> player_cameras;
	booster_field boosters;
	explosion_effects explosions;
	bloom_effect bloom;

	cgp::opengl_shader_structure trail_shader;
	cgp::opengl_shader_structure ground_shader;

	cgp::timer_basic timer;

	ImFont* text_font = nullptr;
	ImFont* title_font = nullptr;

	void initialize();
	void display_frame();
	void display_gui();

	void mouse_move_event();
	void keyboard_event();

private:
	void set_state(game_state new_state);
	void start_match();
	void start_round();

	void update(float dt);
	void update_riders(float dt);
	void detect_collisions();
	void check_end_of_round();

	void render();
	void render_view(int x, int y, int width, int height, cgp::mat4 const& view);
	void send_moto_lights();

	void draw_title_screen();
	void draw_hud();
	void draw_centered_text(ImFont* font, float size, float y, ImU32 color, std::string const& text, float x_center = -1);
};
