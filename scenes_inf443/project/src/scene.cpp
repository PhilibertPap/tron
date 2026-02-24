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
    int grid_size = N + 1;
    int idx00 = ix     + iy     * grid_size;
    int idx10 = (ix+1) + iy     * grid_size;
    int idx01 = ix     + (iy+1) * grid_size;
    int idx11 = (ix+1) + (iy+1) * grid_size;

    // Vérification des bornes
    int max_index = static_cast<int>(grille_mesh.position.size()) - 1;
    if (idx00 > max_index || idx10 > max_index || idx01 > max_index || idx11 > max_index ||
        idx00 < 0 || idx10 < 0 || idx01 < 0 || idx11 < 0) {
        // Retourne une valeur par défaut ou lève une exception selon le besoin
        return 0.0f;
    }

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
		{ -50.0f, 0.0f, 100.0f }, // position caméra (plus haut et derrière)
		{ 0.0f, 0.0f, 0.0f },  // cible (la moto au centre)
		{ 0.0f, 0.0f, 1.0f }   // up
	);


	// Create the global (x,y,z) frame
	global_frame.initialize_data_on_gpu(mesh_primitive_frame());


	// Create the shapes seen in the 3D scene
	// ********************************************** //

	// Terrain sous forme de grille
	
	grille_mesh = mesh_primitive_grid({ -L / 2,-L / 2,0 }, { L / 2,-L / 2,0 }, { L / 2,L / 2,0 }, { -L / 2,L / 2,0 }, N, N);
	//deform_terrain(grille_mesh);

	float repeat = 20.0f;
	grille_mesh.uv.clear();
	for (vec3 const& p : grille_mesh.position) {
		grille_mesh.uv.push_back({ repeat * (p.x + L / 2) / L, repeat * (p.y + L / 2) / L });
	}

	// Placement des luminaires
	int light_step = 10; // espace les lumières tous les 10 points de la grille
	for (int ix = 0; ix <= N; ix += light_step) {
	    for (int iy = 0; iy <= N; iy += light_step) {
	        float x = -L/2 + ix * (L / N);
	        float y = -L/2 + iy * (L / N);
	        float z = get_grid_height(grille_mesh, L, N, x, y) + 0.5f;
	        luminaire_positions.push_back({x, y, z});
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

	wheel.initialize_data_on_gpu(wheel_mesh);
	
	body.initialize_data_on_gpu(body_mesh);
	moto.add(body, "Body", "Root");
	moto.add(wheel, "Wheel1", "Body", { 1.5f, 0.0f, 0.0f });
	moto.add(wheel, "Wheel2", "Body", { -1.5f, 0.0f, 0.0f });

	trail.initialize_data_on_gpu(trail_mesh);

	body2.initialize_data_on_gpu(body_mesh);
	moto2.add(body2, "Body", "Root");
	moto2.add(wheel, "Wheel1", "Body", { 1.5f, 0.0f, 0.0f });
	moto2.add(wheel, "Wheel2", "Body", { -1.5f, 0.0f, 0.0f });

	trail2.initialize_data_on_gpu(mesh());

	luminaire.initialize_data_on_gpu(luminaire_mesh);

	booster.initialize_data_on_gpu(mesh_primitive_sphere(booster_radius));
	booster.material.color = { 1.0f, 0.0f, 1.0f };
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

	// Grille
	// ********************************* //

	const int MAX_LIGHTS = 10;

	// --- Pour la grille : tous les premiers luminaires ---
	int grille_light_count = std::min(MAX_LIGHTS, int(luminaire_positions.size()));
	draw(grille, environment);

	// Moto
	// ********************************* //

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
	float moto_max = (moto_boost_timer > 0.0f) ? boost_max_speed : max_speed;
	if (moto_speed > moto_max) moto_speed = moto_max;
	if (moto_speed < 0.0f) moto_speed = 0.0f; // Empêche la marche arrière

	// Seuil minimal pour arrêter la moto
	if (std::abs(moto_speed) < min_speed_threshold) moto_speed = 0.0f;

	// Calcul de la nouvelle position x/y
	vec2 direction = { std::cos(moto_orientation), std::sin(moto_orientation) };
	float new_x = moto_position.x + direction.x * moto_speed * dt;
	float new_y = moto_position.y + direction.y * moto_speed * dt;

	// Met à jour la position de la moto
	moto_position.x = new_x;
	moto_position.y = new_y;
	moto_position.z = get_grid_height(grille_mesh, L, N, new_x, new_y) + 0.8f;

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

	// --- Pour la moto 1 ---
	get_k_closest_lights(luminaire_positions, moto_position, MAX_LIGHTS, moto_lights);
	environment.uniform_generic.uniform_int["light_count"] = int(moto_lights.size());
	for (int i = 0; i < moto_lights.size(); ++i)
	    environment.uniform_generic.uniform_vec3["local_lights[" + std::to_string(i) + "]"] = moto_lights[i];
	draw(moto, environment);

	// --- Contrôle de la deuxième moto (ZQSD) ---
	if (inputs.keyboard.is_pressed(GLFW_KEY_W)) { // avancer
		moto2_speed += acceleration * dt;
	}
	if (inputs.keyboard.is_pressed(GLFW_KEY_S)) { // reculer
		moto2_speed -= deceleration * dt;
	}
	if (inputs.keyboard.is_pressed(GLFW_KEY_A)) { // gauche
		moto2_orientation += turn_speed * dt;
	}
	if (inputs.keyboard.is_pressed(GLFW_KEY_D)) { // droite
		moto2_orientation -= turn_speed * dt;
	}

	float moto2_max = (moto2_boost_timer > 0.0f) ? boost_max_speed : max_speed;
	if (moto2_speed > moto2_max) moto2_speed = moto2_max;
	if (moto2_speed < 0.0f) moto2_speed = 0.0f;
	if (std::abs(moto2_speed) < min_speed_threshold) moto2_speed = 0.0f;

	vec2 direction2 = { std::cos(moto2_orientation), std::sin(moto2_orientation) };
	float new_x2 = moto2_position.x + direction2.x * moto2_speed * dt;
	float new_y2 = moto2_position.y + direction2.y * moto2_speed * dt;

	moto2_position.x = new_x2;
	moto2_position.y = new_y2;
	moto2_position.z = get_grid_height(grille_mesh, L, N, new_x2, new_y2) + 0.8f;

	moto2["Body"].transform_local.translation = moto2_position;
	moto2["Body"].transform_local.rotation = rotation_transform::from_axis_angle({ 0, 0, 1 }, moto2_orientation);

	float wheel_angle2 = 5.0f * timer.t;
	rotation_transform wheel_orientation2 = rotation_transform::from_axis_angle({ 1,0,0 }, Pi / 2.0f);
	rotation_transform wheel_spin2 = rotation_transform::from_axis_angle({ 0,0,1 }, wheel_angle2);
	moto2["Wheel1"].transform_local.rotation = wheel_orientation2 * wheel_spin2;
	moto2["Wheel2"].transform_local.rotation = wheel_orientation2 * wheel_spin2;

	moto2.update_local_to_global_coordinates();

	// --- Pour la moto 2 ---
	get_k_closest_lights(luminaire_positions, moto2_position, MAX_LIGHTS, moto2_lights);
	environment.uniform_generic.uniform_int["light_count"] = int(moto2_lights.size());
	for (int i = 0; i < moto2_lights.size(); ++i)
		environment.uniform_generic.uniform_vec3["local_lights[" + std::to_string(i) + "]"] = moto2_lights[i];
	draw(moto2, environment);

	// Trail
	// ********************************* //

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
		trail_mesh = create_trail(filtered_positions,trail_color);
	} else {
		trail_mesh = mesh();
	}
 
	trail.initialize_data_on_gpu(trail_mesh);

	draw(trail, environment);

	vec3 rear_pos2 = moto2["Body"].transform_local.translation;
	trail_positions2.push_back(rear_pos2);

	if (trail_positions2.size() > max_trail_length)
		trail_positions2.erase(trail_positions2.begin());

	trail2.vbo_position.clear();

	if (trail_positions2.size() >= 2) {
		std::vector<vec3> filtered_positions2;
		filtered_positions2.push_back(trail_positions2[0]);
		for (size_t i = 1; i < trail_positions2.size(); ++i) {
			if (norm(trail_positions2[i] - trail_positions2[i-1]) > 1e-5f) {
				filtered_positions2.push_back(trail_positions2[i]);
			}
		}
		trail_mesh = create_trail(filtered_positions2, trail2_color);
	} else {
		trail_mesh = mesh();
	}
	trail2.initialize_data_on_gpu(trail_mesh);

	/*trail2.material.color = {1.0f, 1.0f, 0.0f};*/
	draw(trail2, environment);

	// Boosters
	// ********************************* //

	// Apparition automatique de boosters
	booster_spawn_timer += dt;
	if (booster_spawn_timer > booster_spawn_interval) {
	    booster_spawn_timer = 0.0f;
	    if (booster_positions.size() < MAX_BOOSTERS) {
	        // Position aléatoire sur le terrain
	        float bx = -L/2 + static_cast<float>(rand())/RAND_MAX * L;
	        float by = -L/2 + static_cast<float>(rand())/RAND_MAX * L;
	        float bz = get_grid_height(grille_mesh, L, N, bx, by) + 1.0f;
	        booster_positions.push_back({bx, by, bz});
	    }
	}

	// Pour la moto 1
	for (auto it = booster_positions.begin(); it != booster_positions.end(); ) {
	    if (norm(moto_position - *it) < booster_radius) {
	        moto_boost_timer = boost_duration;
	        it = booster_positions.erase(it);
	    } else {
	        ++it;
	    }
	}
	// Pour la moto 2
	for (auto it = booster_positions.begin(); it != booster_positions.end(); ) {
	    if (norm(moto2_position - *it) < booster_radius) {
	        moto2_boost_timer = boost_duration;
	        it = booster_positions.erase(it);
	    } else {
	        ++it;
	    }
	}

	// Dessiner les boosters
	for(const vec3& pos : booster_positions) {
	    booster.model.translation = pos;
	    draw(booster, environment);
	}

	// Luminaires
	// ********************************* //

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

	// --- Reset général si ESPACE ---
	if (inputs.keyboard.is_pressed(GLFW_KEY_SPACE)) {
	    // Reset complet des deux motos et des scores
	    moto_position = { 0.0f,  -L / 2 + 3.0f, get_grid_height(grille_mesh, L, N, -L / 4, 0.0f) + 0.8f };
	    moto_orientation = cgp::Pi / 2; 
	    moto2_position = { 0.0f,  L / 2 - 3.0f, get_grid_height(grille_mesh, L, N, L / 4, 0.0f) + 0.8f };
	    moto2_orientation = - cgp::Pi / 2; 
	    moto_speed = 0.0f;
	    moto2_speed = 0.0f;
	    trail_positions.clear();
	    trail_positions2.clear();
	    score_moto1 = 0;
	    score_moto2 = 0;
	    loser = 0;
	    booster_positions.clear(); // Ajout : réinitialise les boosters
	}

	// --- Collision detection ---
	const float collision_radius = 0.6f;
	bool collision_moto1_trail2 = false;
	for(const vec3& p : trail_positions2) {
	    if(norm(moto_position - p) < collision_radius) {
	        collision_moto1_trail2 = true;
	        break;
	    }
	}
	bool collision_moto2_trail1 = false;
	for(const vec3& p : trail_positions) {
	    if(norm(moto2_position - p) < collision_radius) {
	        collision_moto2_trail1 = true;
	        break;
	    }
	}

	// --- Reset automatique de la moto perdante ---
	if (collision_moto1_trail2) {
	    score_moto2++;
	    loser = 1;
	    moto_position = { 0.0f,  -L / 2 + 3.0f, get_grid_height(grille_mesh, L, N, -L/4, 0.0f) + 0.8f };
	    moto_speed = 0.0f;
	    moto_orientation = cgp::Pi / 2; 
	    trail_positions.clear();
	}
	if (collision_moto2_trail1) {
	    score_moto1++;
	    loser = 2;
	    moto2_position = { 0.0f,  L / 2 - 3.0f, get_grid_height(grille_mesh, L, N, L/4, 0.0f) + 0.8f };
	    moto2_speed = 0.0f;
	    moto2_orientation = - cgp::Pi / 2;
	    trail_positions2.clear();
	}

	// --- Affichage du message de défaite ---
	if (loser != 0) {
	    ImGui::Begin("Score");
	    if (loser == 1)
	        ImGui::Text("La moto bleue a perdu un point ! Score : %d - %d", score_moto1, score_moto2);
	    if (loser == 2)
	        ImGui::Text("La moto verte a perdu un point ! Score : %d - %d", score_moto1, score_moto2);
	    ImGui::Text("Appuyez sur une touche pour continuer.");
	    ImGui::End();
	    return;
	}

	// --- Boost Timer Update ---
	if (moto_boost_timer > 0.0f) moto_boost_timer -= dt;
	if (moto2_boost_timer > 0.0f) moto2_boost_timer -= dt;

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