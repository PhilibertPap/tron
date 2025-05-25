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
		float z = exp(- d2 / 4)-1;

		z = z + 0.05f*noise_perlin({ p.x,p.y });

		p = { p.x, p.y, z };
	}

	m.normal_update();
}

float get_grid_height(const mesh& grille_mesh, float L, int N, float x, float y)
{
    // Convertit (x, y) en indices de la grille
    float fx = (x + L/2) / L * N;
    float fy = (y + L/2) / L * N;
    int ix = int(fx);
    int iy = int(fy);

    // Clamp pour rester dans la grille
    ix = std::max(0, std::min(N-1, ix));
    iy = std::max(0, std::min(N-1, iy));

    // Indices des 4 sommets du carré local
    int idx00 = ix     + iy     * (N+1);
    int idx10 = (ix+1) + iy     * (N+1);
    int idx01 = ix     + (iy+1) * (N+1);
    int idx11 = (ix+1) + (iy+1) * (N+1);

    // Coordonnées locales dans la cellule
    float tx = fx - ix;
    float ty = fy - iy;

    // Interpolation bilinéaire des hauteurs
    float z00 = grille_mesh.position[idx00].z;
    float z10 = grille_mesh.position[idx10].z;
    float z01 = grille_mesh.position[idx01].z;
    float z11 = grille_mesh.position[idx11].z;

    float z0 = (1-tx)*z00 + tx*z10;
    float z1 = (1-tx)*z01 + tx*z11;
    float z = (1-ty)*z0 + ty*z1;

    return z;
}

void get_k_closest_lights(const std::vector<vec3>& luminaire_positions, const vec3& pos, int k, std::vector<vec3>& out_lights)
{
    out_lights.clear();
    std::vector<float> max_dists(k, std::numeric_limits<float>::max());
    std::vector<vec3> closest(k, {0,0,0});

    for (const vec3& l : luminaire_positions) {
        float d = norm(l - pos);
        // Trouver la case la plus éloignée dans closest[]
        int imax = 0;
        for (int i = 1; i < k; ++i)
            if (max_dists[i] > max_dists[imax]) imax = i;
        // Si ce luminaire est plus proche que le plus éloigné du tableau, on le remplace
        if (d < max_dists[imax]) {
            max_dists[imax] = d;
            closest[imax] = l;
        }
    }
    // Remplir le résultat
    for (int i = 0; i < k; ++i)
        out_lights.push_back(closest[i]);
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
	
	grille_mesh = mesh_primitive_grid({ -L / 2,-L / 2,0 }, { L / 2,-L / 2,0 }, { L / 2,L / 2,0 }, { -L / 2,L / 2,0 }, N, N);
	deform_terrain(grille_mesh);

	float repeat = 20.0f;
	grille_mesh.uv.clear();
	for (vec3 const& p : grille_mesh.position) {
		grille_mesh.uv.push_back({ repeat * (p.x + L / 2) / L, repeat * (p.y + L / 2) / L });
	}

	// Placement des luminaires
	float interval = 10.0f;
	for (float x = -L/2; x <= L/2; x += interval) {
		for (float y = -L/2; y <= L/2; y += interval) {
			luminaire_positions.push_back({x, y, 1.0f}); // 1.0f = hauteur du luminaire
		}
	}

	// Moto
	mesh body_mesh = mesh_primitive_cylinder(0.4f, { -1,0,0 }, { 1,0,0 }); // corps long
	mesh wheel_mesh = mesh_primitive_torus(0.5f, 0.3f);

	// Trail
	mesh trail_mesh = mesh();

	// Skybox

	image_structure img_right = image_load_file(project::path + "assets/skybox/px.png");
	image_structure img_left = image_load_file(project::path + "assets/skybox/nx.png");
	image_structure img_top = image_load_file(project::path + "assets/skybox/py.png");
	image_structure img_bottom = image_load_file(project::path + "assets/skybox/ny.png");
	image_structure img_front = image_load_file(project::path + "assets/skybox/pz.png");
	image_structure img_back = image_load_file(project::path + "assets/skybox/nz.png");

	skybox.initialize_data_on_gpu();
	skybox.texture.initialize_cubemap_on_gpu(
		img_left,
		img_right,
		img_bottom,
		img_top,
		img_back,
		img_front
	);

	// Luminaire

	mesh luminaire_mesh = mesh_primitive_sphere(0.3f);

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

	luminaire.initialize_data_on_gpu(luminaire_mesh);
}


// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.
void scene_structure::display_frame()
{

	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();

	// Set additional uniform parameters to the shader
	environment.uniform_generic.uniform_vec3["light_color"] = gui.light_color;

	//  Skybox
	glDepthMask(GL_FALSE);
	draw(skybox, environment);
	glDepthMask(GL_TRUE);  

	// conditional display of the global frame (set via the GUI)
	if (gui.display_frame)
		draw(global_frame, environment);
	
	// Update time
	float dt = timer.update() * timer.scale;

	// Draw all the shapes

	const int MAX_LIGHTS = 9;
	std::vector<vec3> sol_lights;
	get_k_closest_lights(luminaire_positions, {0,0,0}, MAX_LIGHTS, sol_lights);
	environment.uniform_generic.uniform_int["light_count"] = int(sol_lights.size());
	for (int i = 0; i < sol_lights.size(); ++i)
		environment.uniform_generic.uniform_vec3["local_lights[" + std::to_string(i) + "]"] = sol_lights[i];
	draw(grille, environment);

	// Moto
	// ********************************* //

	std::cout << "dt: " << dt << std::endl;

	// Gestion du clavier
	if (inputs.keyboard.is_pressed(GLFW_KEY_UP)) {
		moto_speed += acceleration * dt;
	}
	if (inputs.keyboard.is_pressed(GLFW_KEY_DOWN)) {
		moto_speed -= deceleration * dt;
	}
	if (inputs.keyboard.is_pressed(GLFW_KEY_LEFT)) {
		moto_orientation += turn_speed * dt;
	}
	if (inputs.keyboard.is_pressed(GLFW_KEY_RIGHT)) {
		moto_orientation -= turn_speed * dt;
	}

	// Clamp la vitesse
	if (moto_speed > max_speed) moto_speed = max_speed;
	if (moto_speed < 0.0f) moto_speed = 0.0f; // Empêche la marche arrière

	// Seuil minimal pour arrêter la moto
	if (std::abs(moto_speed) < min_speed_threshold) moto_speed = 0.0f;

	// Mise à jour de la position
	vec2 direction = { std::cos(moto_orientation), std::sin(moto_orientation) };
	moto_position.x += direction.x * moto_speed * dt;
	moto_position.y += direction.y * moto_speed * dt;
	moto_position.z = get_grid_height(grille_mesh, L, N, moto_position.x, moto_position.y) + 0.8f;

	std::cout << "moto_speed: " << moto_speed << std::endl;
	std::cout << "moto_position: " << moto_position << std::endl;

	// Application à la moto
	moto["Body"].transform_local.translation = moto_position;
	moto["Body"].transform_local.rotation = rotation_transform::from_axis_angle({ 0, 0, 1 }, moto_orientation);

	// Roues
	float wheel_angle = 5.0f * timer.t;
	rotation_transform wheel_orientation = rotation_transform::from_axis_angle({ 1,0,0 }, Pi / 2.0f);
	rotation_transform wheel_spin = rotation_transform::from_axis_angle({ 0,0,1 }, wheel_angle);
	moto["Wheel1"].transform_local.rotation = wheel_orientation * wheel_spin;
	moto["Wheel2"].transform_local.rotation = wheel_orientation * wheel_spin;

	moto.update_local_to_global_coordinates();

	// Effet des luminaires sur la moto

	std::vector<vec3> moto_lights;
	vec3 moto_pos = moto["Body"].transform_local.translation;
	get_k_closest_lights(luminaire_positions, moto_pos, 3, moto_lights);
	environment.uniform_generic.uniform_int["light_count"] = int(moto_lights.size());
	for (int i = 0; i < moto_lights.size(); ++i)
		environment.uniform_generic.uniform_vec3["local_lights[" + std::to_string(i) + "]"] = moto_lights[i];
	draw(moto, environment);


	// Trail positions

	vec3 rear_pos = moto["Body"].transform_local.translation;
	trail_positions.push_back(rear_pos);

	if (trail_positions.size() > max_trail_length) // Taille max du trail
	{
		trail_positions.erase(trail_positions.begin());
	}

	trail.vbo_position.clear();

	if (trail_positions.size() >= 2) {
		// Supprimer les points consécutifs trop proches
		std::vector<vec3> filtered_positions;
		filtered_positions.push_back(trail_positions[0]);
		for (size_t i = 1; i < trail_positions.size(); ++i) {
			if (norm(trail_positions[i] - trail_positions[i-1]) > 1e-5f) {
				filtered_positions.push_back(trail_positions[i]);
			}
		}
		trail_mesh = create_trail(filtered_positions);
	} else {
		trail_mesh = mesh();
	}

	trail.initialize_data_on_gpu(trail_mesh);

	draw(trail, environment);

	// Trouver le luminaire le plus proche de la moto
	vec3 current_pos = moto["Wheel2"].transform_local.translation;
	float min_dist = 1e9f;
	vec3 closest_light = {0,0,0};
	for (const vec3& pos : luminaire_positions) {
		float d = norm(current_pos - pos);
		if (d < min_dist) {
			min_dist = d;
			closest_light = pos;
		}
	}
	// Passer la position au shader
	environment.uniform_generic.uniform_vec3["local_light"] = closest_light;

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
	ImGui::ColorEdit3("Light color", &gui.light_color[0]);
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
	//camera_control.action_keyboard(environment.camera_view);
}

void scene_structure::idle_frame()
{
    // Sauvegarde l'état des flèches
    bool left = inputs.keyboard.left;
    bool right = inputs.keyboard.right;
    bool up = inputs.keyboard.up;
    bool down = inputs.keyboard.down;

    // Désactive les flèches pour la caméra
    inputs.keyboard.left = false;
    inputs.keyboard.right = false;
    inputs.keyboard.up = false;
    inputs.keyboard.down = false;

    // Appelle le comportement caméra sans les flèches
    camera_control.idle_frame(environment.camera_view);

    // Restaure l'état des flèches
    inputs.keyboard.left = left;
    inputs.keyboard.right = right;
    inputs.keyboard.up = up;
    inputs.keyboard.down = down;
}