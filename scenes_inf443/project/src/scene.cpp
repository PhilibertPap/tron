#include "scene.hpp"

#include "trail.hpp"

using namespace cgp;

void deform_terrain(mesh& m)
{
	// Set the terrain to have a gaussian shape
	for (int k = 0; k < m.position.size(); ++k)
	{
		vec3& p = m.position[k];
		float d2 = p.x*p.x + p.y * p.y;
		float z = exp(-d2 / 4)-1;

		z = z + 0.05f*noise_perlin({ p.x,p.y });

		p = { p.x, p.y, z };
	}

	m.normal_update();
}

// This function is called only once at the beginning of the program
// This function can contain any complex operation that can be pre-computed once
void scene_structure::initialize()
{
	std::cout << "Start function scene_structure::initialize()" << std::endl;

	// Set the behavior of the camera and its initial position
	// ********************************************** //
	camera_control.initialize(inputs, window); 
	camera_control.set_rotation_axis_z(); // camera rotates around z-axis
	//   look_at(camera_position, targeted_point, up_direction)
	camera_control.look_at(
		{ 5.0f, -4.0f, 3.5f } /* position of the camera in the 3D scene */,
		{0,0,0} /* targeted point in 3D scene */,
		{0,0,1} /* direction of the "up" vector */);


	// Create the global (x,y,z) frame
	global_frame.initialize_data_on_gpu(mesh_primitive_frame());


	// Create the shapes seen in the 3D scene
	// ********************************************** //

	// Terrain sous forme de grille
	float L = 50.0f; // Longueur du Terrain
	int N = 100; // nombre de subdivisions
	mesh grille_mesh = mesh_primitive_grid({ -L / 2,-L / 2,0 }, { L / 2,-L / 2,0 }, { L / 2,L / 2,0 }, { -L / 2,L / 2,0 }, N, N);
	//deform_terrain(grille_mesh);

	float repeat = 20.0f;
	grille_mesh.uv.clear();
	for (vec3 const& p : grille_mesh.position) {
		grille_mesh.uv.push_back({ repeat * (p.x + L / 2) / L, repeat * (p.y + L / 2) / L });
	}

	// Moto
	mesh body_mesh = mesh_primitive_cylinder(0.4f, { -1,0,0 }, { 1,0,0 }); // corps long
	mesh wheel_mesh = mesh_primitive_torus(0.5f, 0.3f);

	// Trail
	mesh trail_mesh = mesh();

	// Initialize data
	// ********************************************** //

	grille.initialize_data_on_gpu(grille_mesh);
	grille.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/grille.png", GL_REPEAT, GL_REPEAT);

	body.initialize_data_on_gpu(body_mesh);
	wheel.initialize_data_on_gpu(wheel_mesh);
	moto.add(body, "Body", "Root");
	moto.add(wheel, "Wheel1", "Body", { 1.5f, 0.0f, 0.0f });
	moto.add(wheel, "Wheel2", "Body", { -1.5f, 0.0f, 0.0f });

	trail.initialize_data_on_gpu(trail_mesh);

}


// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.
void scene_structure::display_frame()
{

	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();
	

	// conditional display of the global frame (set via the GUI)
	if (gui.display_frame)
		draw(global_frame, environment);
	
	// Update time
	timer.update();

	// Draw all the shapes
	draw(grille, environment);
	
	// Moto

	float radius = 5.0f;
	moto["Body"].transform_local.translation = 
	{
		radius * cos(2.0f * timer.t),
		radius * sin(2.0f * timer.t),
		0.8f 
	};
	moto["Body"].transform_local.rotation = rotation_transform::from_axis_angle({ 0, 0, 1 }, 2.0f * timer.t + Pi / 2.0f);

	float wheel_angle = 5.0f * timer.t;
	rotation_transform wheel_orientation = rotation_transform::from_axis_angle({ 1,0,0 }, Pi / 2.0f);
	rotation_transform wheel_spin = rotation_transform::from_axis_angle({ 0,0,1 }, wheel_angle);
	moto["Wheel1"].transform_local.rotation = wheel_orientation * wheel_spin;
	moto["Wheel2"].transform_local.rotation = wheel_orientation * wheel_spin;

	moto.update_local_to_global_coordinates();

	draw(moto, environment);


	// Trail positions


	vec3 current_pos = moto["Body"].transform_local.translation;
	trail_positions.push_back(current_pos);

	if (trail_positions.size() > 100) // Taille max du trail
	{
		trail_positions.erase(trail_positions.begin());
	}

	trail.vbo_position.clear();

	trail_mesh = create_trail(trail_positions);

	trail.initialize_data_on_gpu(trail_mesh);

	draw(trail, environment);

	// Display Wireframe

	if (gui.display_wireframe) {
		draw_wireframe(grille, environment);
		draw_wireframe(moto, environment);
		draw_wireframe(trail, environment);
	}
	
}

void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);

}

void scene_structure::mouse_move_event()
{
	if (!inputs.keyboard.shift)
		camera_control.action_mouse_move(environment.camera_view);
}
void scene_structure::mouse_click_event()
{
	camera_control.action_mouse_click(environment.camera_view);
}
void scene_structure::keyboard_event()
{
	camera_control.action_keyboard(environment.camera_view);
}
void scene_structure::idle_frame()
{
	camera_control.idle_frame(environment.camera_view);
}

