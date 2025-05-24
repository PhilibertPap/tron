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

	vec3 light_color = { 1,1,1 };
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

	mesh_drawable grille;
	mesh_drawable body;
	mesh_drawable wheel;
	cgp::mesh trail_mesh;
	mesh_drawable trail;
	mesh_drawable ground_light;

	cgp::skybox_drawable skybox;

	std::vector<vec3> trail_positions;
	int max_trail_length = 100;

	vec3 moto_position = {0, 0, 0.8f};
	float moto_speed = 0.0f;           // vitesse actuelle
	float moto_speed_max = 0.3f;       // vitesse maximale
	float moto_acceleration = 0.01f;   // accélération par frame
	float moto_friction = 0.005f;      // ralentissement naturel
	float moto_angle = 0.0f;           // direction (en radians)
	float moto_turn_speed = 0.03f;     // vitesse de rotation (pour la courbe)

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