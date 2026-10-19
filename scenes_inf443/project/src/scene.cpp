#include "scene.hpp"

#include <cfloat>

using namespace cgp;

namespace
{
	constexpr float countdown_duration = 3.0f;
	constexpr float round_over_duration = 2.5f;
	constexpr float moto_collision_distance = 1.6f;

	ImU32 to_imgui_color(vec3 const& c, float alpha = 1.0f)
	{
		return ImGui::ColorConvertFloat4ToU32({ c.x, c.y, c.z, alpha });
	}

	mat4 look_at(vec3 const& eye, vec3 const& target, vec3 const& up = { 0, 0, 1 })
	{
		return inverse(camera_frame_look_at(eye, target, up)).matrix();
	}
}


// Caméras qui suivent les motos
// ***************************************************************** //

namespace
{
	void camera_placement(player_structure const& player, camera_mode mode, vec3& eye, vec3& target)
	{
		vec3 const forward = { player.direction().x, player.direction().y, 0 };
		vec3 const moto = player.position_3d();

		if (mode == camera_mode::first_person) {
			// Juste devant le phare, pour ne pas voir l'intérieur de sa propre moto
			eye = moto + 1.4f * forward + vec3{ 0, 0, 1.1f };
			target = eye + 10.0f * forward - vec3{ 0, 0, 0.7f };
		}
		else {
			eye = moto - 7.0f * forward + vec3{ 0, 0, 3.0f };
			target = moto + 5.0f * forward + vec3{ 0, 0, 0.6f };
		}
	}
}

void follow_camera::snap(player_structure const& player, camera_mode mode)
{
	camera_placement(player, mode, eye, target);
}

void follow_camera::update(player_structure const& player, camera_mode mode, float dt)
{
	vec3 eye_goal, target_goal;
	camera_placement(player, mode, eye_goal, target_goal);

	// En vue subjective la caméra est fixée à la moto ; sinon elle la rattrape en douceur
	float const k = (mode == camera_mode::first_person) ? 1.0f : std::min(1.0f, 6.0f * dt);
	eye += k * (eye_goal - eye);
	target += k * (target_goal - target);
}

mat4 follow_camera::view_matrix() const
{
	return look_at(eye, target);
}


// Initialisation
// ***************************************************************** //

void scene_structure::initialize()
{
	std::string const shaders = project::path + "shaders/";
	trail_shader.load(shaders + "mesh/mesh.vert.glsl", shaders + "trail/trail.frag.glsl");
	ground_shader.load(shaders + "mesh/mesh.vert.glsl", shaders + "ground/ground.frag.glsl");

	level.initialize(ground_shader);
	boosters.initialize();
	explosions.initialize();
	bloom.initialize(shaders + "post/");

	// Tron (bleu) à gauche du clavier (ZQSD en AZERTY, WASD en QWERTY), Clu (orange) sur les flèches.
	// En solo, Clu est piloté par l'IA.
	players.resize(2);
	keyboard_layout const zqsd = { { GLFW_KEY_W, GLFW_KEY_Z }, { GLFW_KEY_S, GLFW_KEY_S }, { GLFW_KEY_A, GLFW_KEY_Q }, { GLFW_KEY_D, GLFW_KEY_D } };
	keyboard_layout const arrows = { { GLFW_KEY_UP, GLFW_KEY_UP }, { GLFW_KEY_DOWN, GLFW_KEY_DOWN }, { GLFW_KEY_LEFT, GLFW_KEY_LEFT }, { GLFW_KEY_RIGHT, GLFW_KEY_RIGHT } };
	players[0].initialize("Tron", { 0.1f, 0.75f, 1.0f }, zqsd, trail_shader);
	players[1].initialize("Clu", { 1.0f, 0.45f, 0.08f }, arrows, trail_shader);
	player_cameras.resize(players.size());

	camera_control.initialize(inputs, window);
	camera_control.set_rotation_axis_z();
	camera_control.look_at({ 0, -130, 110 }, { 0, 0, 0 });

	ImGuiIO& io = ImGui::GetIO();
	text_font = io.Fonts->AddFontFromFileTTF((project::path + "assets/fonts/Rajdhani-SemiBold.ttf").c_str(), 30.0f);
	title_font = io.Fonts->AddFontFromFileTTF((project::path + "assets/fonts/Orbitron.ttf").c_str(), 128.0f);

	timer.start();
	start_match();
	set_state(game_state::title);
}


// Déroulement d'une partie
// ***************************************************************** //

void scene_structure::set_state(game_state new_state)
{
	state = new_state;
	state_time = 0;
}

void scene_structure::start_match()
{
	for (player_structure& player : players)
		player.score = 0;
	ai.difficulty = options.difficulty;
	start_round();
}

void scene_structure::start_round()
{
	// Au sol, côté sud, chacun face à l'autre sur deux lignes décalées
	players[0].spawn({ -60.0f, -64.0f }, 0.0f);
	players[1].spawn({ 60.0f, -56.0f }, Pi);

	for (size_t i = 0; i < players.size(); ++i) {
		players[i].trail.max_length = options.wall_length;
		player_cameras[i].snap(players[i], options.camera);
	}

	boosters.clear();
	explosions.clear();
	ai.reset();
	set_state(game_state::countdown);
}

void scene_structure::update(float dt)
{
	state_time += dt;
	explosions.update(dt);

	switch (state)
	{
	case game_state::title:
	case game_state::match_over:
		break;

	case game_state::countdown:
		if (state_time > countdown_duration)
			set_state(game_state::playing);
		break;

	case game_state::playing:
		update_riders(dt);
		boosters.update(dt, level, players);
		detect_collisions();
		check_end_of_round();
		break;

	case game_state::round_over:
		// Le gagnant continue sur sa lancée, les perdants finissent de tomber
		update_riders(dt);
		detect_collisions();
		if (state_time > round_over_duration) {
			bool match_won = false;
			for (size_t i = 0; i < players.size(); ++i) {
				if (players[i].score >= options.points_to_win) {
					round_message = victory_message(i);
					match_won = true;
				}
			}
			if (match_won)
				set_state(game_state::match_over);
			else
				start_round();
		}
		break;
	}

	for (size_t i = 0; i < players.size(); ++i)
		player_cameras[i].update(players[i], options.camera, dt);
}

void scene_structure::update_riders(float dt)
{
	for (size_t i = 0; i < players.size(); ++i) {
		player_input input;
		if (state == game_state::playing)
			input = input_for(i, dt);
		players[i].update(input, dt, level);
	}
}

// D'où viennent les commandes de chaque moto : clavier ou IA
player_input scene_structure::input_for(size_t i, float dt)
{
	if (options.solo && i == 1)
		return ai.decide(players[1], players, level, boosters, dt);

	if (options.solo && mobile.is_active())
		return mobile.read();

	if (options.solo) {
		// Seul contre l'IA, on peut jouer avec ZQSD comme avec les flèches
		player_input const a = read_keyboard(inputs.keyboard, players[0].keys);
		player_input const b = read_keyboard(inputs.keyboard, players[1].keys);
		player_input input;
		input.throttle = std::max(-1.0f, std::min(1.0f, a.throttle + b.throttle));
		input.steer = std::max(-1.0f, std::min(1.0f, a.steer + b.steer));
		return input;
	}

	return read_keyboard(inputs.keyboard, players[i].keys);
}

void scene_structure::detect_collisions()
{
	// On note d'abord qui s'écrase, puis on applique : si deux motos se percutent
	// à la même frame, elles perdent toutes les deux.
	std::vector<bool> crashed(players.size(), false);

	for (size_t i = 0; i < players.size(); ++i) {
		player_structure const& player = players[i];
		if (!player.is_riding())
			continue;

		vec3 const front = player.front();
		if (level.blocks({ front.x, front.y }, front.z, moto_height))
			crashed[i] = true;

		for (size_t j = 0; j < players.size(); ++j) {
			int const ignored_segments = (i == j) ? 2 : 0; // son propre mur, juste derrière la roue
			if (players[j].trail.hits(player.previous_front, front, moto_height, ignored_segments))
				crashed[i] = true;
		}

		for (size_t j = i + 1; j < players.size(); ++j) {
			if (players[j].is_riding() && norm(player.position_3d() - players[j].position_3d()) < moto_collision_distance)
				crashed[i] = crashed[j] = true;
		}
	}

	for (size_t i = 0; i < players.size(); ++i) {
		if (crashed[i]) {
			players[i].crash();
			explosions.spawn(players[i].position_3d(), players[i].color);
		}
	}
}

void scene_structure::check_end_of_round()
{
	size_t survivor = 0;
	int riders = 0;
	for (size_t i = 0; i < players.size(); ++i) {
		if (players[i].is_riding()) {
			survivor = i;
			riders++;
		}
	}
	if (riders > 1)
		return;

	if (riders == 1) {
		players[survivor].score++;
		round_message = point_message(survivor);
	}
	else
		round_message = "Égalité";

	set_state(game_state::round_over);
}

std::string scene_structure::point_message(size_t scorer) const
{
	return "Point pour " + players[scorer].name;
}

std::string scene_structure::victory_message(size_t winner) const
{
	return players[winner].name + " remporte la partie !";
}


// Sur téléphone, le menu est en HTML (voir scripts/emscripten_template.html) : on traite ici ses demandes
void scene_structure::handle_mobile_events()
{
	int const difficulty = mobile.take_start_request();
	if (difficulty >= 0) {
		options.solo = true;
		options.difficulty = ai_difficulty(difficulty);
		if (options.camera == camera_mode::overview)
			options.camera = camera_mode::third_person;
		start_match();
	}

	if (mobile.take_menu_request())
		set_state(game_state::title);

	if (mobile.take_tap() && state == game_state::match_over)
		start_match();
}


// Affichage 3D
// ***************************************************************** //

void scene_structure::display_frame()
{
	float const dt = std::min(timer.update(), 0.05f); // pas de grand saut si une frame a été lente
	if (mobile.is_active())
		handle_mobile_events();
	update(dt);

	project::gui_scale = std::max(0.6f, std::min(1.6f, window.height / 1000.0f));
	render();
}

void scene_structure::render()
{
	int const w = window.width;
	int const h = window.height;
	if (w == 0 || h == 0)
		return; // fenêtre réduite

	send_moto_lights();

	bool const use_bloom = options.bloom && bloom.is_supported();
	if (use_bloom)
		bloom.begin(w, h);

	if (state == game_state::title) {
		// Lent travelling autour du plateau
		float const angle = 0.1f * timer.t;
		vec3 const eye = { 135 * std::sin(angle), -135 * std::cos(angle), 60 };
		camera_projection.field_of_view = 50.0f * Pi / 180;
		render_view(0, 0, w, h, look_at(eye, { 0, 0, 0 }));
	}
	else if (options.camera == camera_mode::overview) {
		camera_projection.field_of_view = 50.0f * Pi / 180;
		render_view(0, 0, w, h, camera_control.camera_model.matrix_view());
	}
	else {
		camera_projection.field_of_view = (options.camera == camera_mode::first_person ? 75.0f : 65.0f) * Pi / 180;
		if (options.solo)
			render_view(0, 0, w, h, player_cameras[0].view_matrix());
		else {
			// Ecran partagé : joueur 1 à gauche, joueur 2 à droite (comme sur le clavier)
			int const half = w / 2;
			render_view(0, 0, half - 1, h, player_cameras[0].view_matrix());
			render_view(half + 1, 0, w - half - 1, h, player_cameras[1].view_matrix());
		}
	}

	if (use_bloom)
		bloom.end_and_display();
	else
		glViewport(0, 0, w, h);
}

void scene_structure::render_view(int x, int y, int width, int height, mat4 const& view)
{
	glViewport(x, y, width, height);
	camera_projection.aspect_ratio = float(width) / float(height);
	environment.camera_projection = camera_projection.matrix();
	environment.camera_view = view;

	level.draw_skybox(environment);
	level.draw(environment);
	for (player_structure& player : players)
		player.draw(environment);
	boosters.draw(environment, timer.t);
	explosions.draw(environment);

	// Les murs en dernier : transparents, en mélange additif, sans écrire dans le z-buffer
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);
	for (player_structure const& player : players)
		player.trail.draw(environment);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void scene_structure::send_moto_lights()
{
	for (size_t i = 0; i < players.size(); ++i) {
		player_structure const& player = players[i];
		bool const lit = player.status != player_status::crashed;
		std::string const index = "[" + str(i) + "]";
		environment.uniform_generic.uniform_vec3["moto_light_position" + index] = player.position_3d() + vec3{ 0, 0, 1.2f };
		environment.uniform_generic.uniform_vec3["moto_light_color" + index] = lit ? player.color : vec3{ 0, 0, 0 };
	}
}


// Interface : écran titre et affichage tête haute
// ***************************************************************** //

void scene_structure::display_gui()
{
	if (state != game_state::title)
		draw_hud();
	else if (!mobile.is_active())
		draw_title_screen(); // sur téléphone, c'est le menu HTML qui s'affiche
}

void scene_structure::draw_centered_text(ImFont* font, float size, float y, ImU32 color, std::string const& text, float x_center)
{
	if (x_center < 0)
		x_center = ImGui::GetIO().DisplaySize.x / 2;
	ImVec2 const s = font->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());
	ImVec2 const pos = { x_center - s.x / 2, y - s.y / 2 };
	float const shadow = std::max(1.0f, 0.04f * size);
	ImDrawList* draw_list = ImGui::GetOverlayDrawList();
	draw_list->AddText(font, size, { pos.x + shadow, pos.y + shadow }, IM_COL32(0, 0, 0, 160), text.c_str());
	draw_list->AddText(font, size, pos, color, text.c_str());
}

void scene_structure::draw_title_screen()
{
	float const W = ImGui::GetIO().DisplaySize.x;
	float const H = ImGui::GetIO().DisplaySize.y;
	vec3 const cyan = { 0.4f, 0.9f, 1.0f };

	draw_centered_text(title_font, 0.17f * H, 0.17f * H, to_imgui_color(cyan), "TRON");
	draw_centered_text(text_font, 0.045f * H, 0.29f * H, to_imgui_color({ 0.8f, 0.9f, 1.0f }), "L I G H T   C Y C L E S");

	ImGui::SetNextWindowPos({ W / 2, 0.6f * H }, ImGuiCond_Always, { 0.5f, 0.5f });
	ImGui::SetNextWindowBgAlpha(0.6f);
	ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);

	int mode = options.solo ? 0 : 1;
	ImGui::Text("Mode");
	ImGui::RadioButton("Solo contre Clu", &mode, 0);
	ImGui::SameLine();
	ImGui::RadioButton("2 joueurs", &mode, 1);
	options.solo = (mode == 0);

	if (options.solo) {
		int difficulty = int(options.difficulty);
		ImGui::Text("Difficulté");
		ImGui::RadioButton("Facile", &difficulty, int(ai_difficulty::easy));
		ImGui::SameLine();
		ImGui::RadioButton("Normale", &difficulty, int(ai_difficulty::normal));
		ImGui::SameLine();
		ImGui::RadioButton("Difficile", &difficulty, int(ai_difficulty::hard));
		options.difficulty = ai_difficulty(difficulty);
	}

	ImGui::Spacing();
	int camera = int(options.camera);
	ImGui::Text("Vue");
	ImGui::RadioButton("Globale", &camera, int(camera_mode::overview));
	ImGui::SameLine();
	ImGui::RadioButton("3e personne", &camera, int(camera_mode::third_person));
	ImGui::SameLine();
	ImGui::RadioButton("1re personne", &camera, int(camera_mode::first_person));
	options.camera = camera_mode(camera);

	ImGui::Spacing();
	ImGui::SliderInt("Points pour gagner", &options.points_to_win, 1, 10);
	ImGui::SliderFloat("Longueur des murs", &options.wall_length, 40.0f, trail_length_limit, "%.0f m");
	if (bloom.is_supported())
		ImGui::Checkbox("Effet de lueur", &options.bloom);

	ImGui::Separator();
	if (options.solo) {
		ImGui::TextColored({ 0.1f, 0.75f, 1.0f, 1.0f }, "Tron");
		ImGui::SameLine(130 * project::gui_scale);
		ImGui::Text("Z Q S D ou flèches");
	}
	else {
		ImGui::TextColored({ 0.1f, 0.75f, 1.0f, 1.0f }, "Tron");
		ImGui::SameLine(130 * project::gui_scale);
		ImGui::Text("Z Q S D  (W A S D en QWERTY)");
		ImGui::TextColored({ 1.0f, 0.45f, 0.08f, 1.0f }, "Clu");
		ImGui::SameLine(130 * project::gui_scale);
		ImGui::Text("Flèches");
	}
	ImGui::Spacing();
	ImGui::TextDisabled("C : changer de vue    F11 : plein écran    Échap : menu / quitter");
	ImGui::End();

	float const pulse = 0.6f + 0.4f * std::sin(4.0f * timer.t);
	draw_centered_text(text_font, 0.05f * H, 0.9f * H, to_imgui_color({ 1, 1, 1 }, pulse), "Appuyez sur Entrée pour jouer");
}

void scene_structure::draw_hud()
{
	float const W = ImGui::GetIO().DisplaySize.x;
	float const H = ImGui::GetIO().DisplaySize.y;
	bool const split = !options.solo && options.camera != camera_mode::overview;

	// Scores : au-dessus de chaque moitié d'écran, ou côte à côte sur un seul écran
	for (size_t i = 0; i < players.size(); ++i) {
		player_structure const& player = players[i];
		float const x = split ? (i == 0 ? 0.25f : 0.75f) * W : (i == 0 ? 0.45f : 0.55f) * W;
		draw_centered_text(text_font, 0.1f * H, 0.07f * H, to_imgui_color(player.color), str(player.score), x);
		if (player.boost_timer > 0 && player.is_riding())
			draw_centered_text(text_font, 0.04f * H, 0.14f * H, to_imgui_color({ 1.0f, 0.3f, 0.9f }), "BOOST", x);
	}

	ImU32 const white = to_imgui_color({ 1, 1, 1 });
	ImU32 const light_blue = to_imgui_color({ 0.8f, 0.9f, 1.0f });
	if (state == game_state::countdown) {
		int const count = int(countdown_duration - state_time) + 1;
		draw_centered_text(title_font, 0.25f * H, 0.45f * H, white, str(count));
		if (mobile.is_active()) {
			draw_centered_text(text_font, 0.055f * H, 0.76f * H, light_blue, "Penche le téléphone pour tourner");
			draw_centered_text(text_font, 0.045f * H, 0.84f * H, light_blue, "Doigt à droite : accélérer    Doigt à gauche : freiner");
		}
	}
	else if (state == game_state::playing && state_time < 0.7f) {
		draw_centered_text(title_font, 0.2f * H, 0.45f * H, to_imgui_color({ 1, 1, 1 }, 1 - state_time / 0.7f), "GO");
	}
	else if (state == game_state::round_over) {
		draw_centered_text(text_font, 0.08f * H, 0.45f * H, white, round_message);
	}
	else if (state == game_state::match_over) {
		draw_centered_text(text_font, 0.09f * H, 0.42f * H, white, round_message);
		draw_centered_text(text_font, 0.045f * H, 0.55f * H, light_blue,
			mobile.is_active() ? "Touchez l'écran pour rejouer" : "Entrée : rejouer      Échap : menu");
	}
}


// Entrées clavier / souris
// ***************************************************************** //

void scene_structure::keyboard_event()
{
	inputs_keyboard_last_action_parameter const& key = inputs.keyboard.last_action;

	if (key.is_pressed(GLFW_KEY_ENTER) || key.is_pressed(GLFW_KEY_KP_ENTER)) {
		if (state == game_state::title || state == game_state::match_over)
			start_match();
	}

	if (key.is_pressed(GLFW_KEY_ESCAPE)) {
		if (state == game_state::title)
			glfwSetWindowShouldClose(window.glfw_window, GLFW_TRUE);
		else
			set_state(game_state::title);
	}

	if (key.is_pressed(GLFW_KEY_C) && state != game_state::title) {
		options.camera = camera_mode((int(options.camera) + 1) % 3);
		for (size_t i = 0; i < players.size(); ++i)
			player_cameras[i].snap(players[i], options.camera);
	}

	if (key.is_pressed(GLFW_KEY_F11)) {
		window.is_full_screen = !window.is_full_screen;
		if (window.is_full_screen)
			window.set_full_screen();
		else
			window.set_windowed_screen();
	}
}

void scene_structure::mouse_move_event()
{
	// La souris n'oriente que la vue globale
	if (state != game_state::title && options.camera == camera_mode::overview && !inputs.mouse.on_gui)
		camera_control.action_mouse_move(environment.camera_view);
}
