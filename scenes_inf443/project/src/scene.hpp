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

	float L = 100.0f; // Longueur du Terrain
	int N = 200; // nombre de subdivisions
	cgp::mesh grille_mesh;
	mesh_drawable grille;

	mesh_drawable body;
	mesh_drawable wheel;
	vec3 moto_position = { 0.0f, 0.0f, 0.0f };
	float moto_orientation = 0.0f; // angle en radians
	float moto_speed = 0.0f;
	const float max_speed = 10.0f;
	const float acceleration = 10.0f;
	const float deceleration = 8.0f;
	const float min_speed_threshold = 0.1f;
	const float turn_speed = 2.0f;

	cgp::mesh trail_mesh;
	mesh_drawable trail;
	
	mesh_drawable luminaire;

	cgp::skybox_drawable skybox;

	std::vector<vec3> trail_positions;
	int max_trail_length = 150;

	std::vector<vec3> luminaire_positions;

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