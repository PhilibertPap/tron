#pragma once


#include "cgp/cgp.hpp"
#include "environment.hpp"


// This definitions allow to use the structures: mesh, mesh_drawable, etc. without mentionning explicitly cgp::
using cgp::mesh;
using cgp::mesh_drawable;
using cgp::vec3;
using cgp::numarray;
using cgp::timer_basic;

// Variables associated to the GUI (buttons, etc)
struct gui_parameters {
	bool display_frame = true;
	bool display_wireframe = false;

	vec3 light_color = { 0,1,1 };
};

// The structure of the custom scene
struct scene_structure : cgp::scene_inputs_generic {
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //
	camera_controller_orbit_euler camera_control;
	camera_projection_perspective camera_projection;
	window_structure window;

	mesh_drawable global_frame;          // The standard global frame
	environment_structure environment;   // Standard environment controler
	input_devices inputs;                // Storage for inputs status (mouse, keyboard, window dimension)
	gui_parameters gui;                  // Standard GUI element storage
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //

	timer_basic timer;

	cgp::hierarchy_mesh_drawable moto;
	hierarchy_mesh_drawable moto2;

	mesh_drawable body;
	mesh_drawable body2;
	mesh_drawable wheel;
	mesh_drawable wheel2;

	float L = 100.0f; // Longueur du Terrain
	int N = 200; // nombre de subdivisions
	cgp::mesh grille_mesh;
	mesh_drawable grille;


	cgp::mesh trail_mesh;
	int max_trail_length = 100;
	mesh_drawable luminaire;
	std::vector<vec3> luminaire_positions;
	cgp::skybox_drawable skybox;

	vec3 moto_position = { 0.0f, 0.0f, 0.0f };
	vec3 moto2_position = { 0, 5, 0 }; // Position initiale différente
	float moto_orientation = 0.0f; // angle en radians
	float moto2_orientation = 0.0f;
	float moto_speed = 0.0f;
	float moto2_speed = 0.0f;

	const float max_speed = 10.0f;
	const float acceleration = 10.0f;
	const float deceleration = 8.0f;
	const float min_speed_threshold = 0.1f;
	const float turn_speed = 2.0f;

	mesh_drawable trail;
	mesh_drawable trail2;
	std::vector<vec3> trail_positions;
	std::vector<vec3> trail_positions2;

	bool game_over = false;

	int score_moto1 = 0;
	int score_moto2 = 0;
	int loser = 0; // 0 = personne, 1 = moto1 a perdu, 2 = moto2 a perdu
	

	// ****************************** //
	// Functions
	// ****************************** //

	void initialize();    // Standard initialization to be called before the animation loop
	void display_frame(); // The frame display to be called within the animation loop
	void display_gui();   // The display of the GUI, also called within the animation loop


	void mouse_move_event();
	void mouse_click_event();
	void keyboard_event();
	void idle_frame();

};