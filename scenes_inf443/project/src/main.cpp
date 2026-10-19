#include "cgp/cgp.hpp"
#include "environment.hpp"
#include "scene.hpp"

#include <iostream>

scene_structure scene;

window_structure standard_window_initialization();
void initialize_default_shaders();
void animation_loop();

int main(int, char* argv[])
{
	std::cout << "Run " << argv[0] << std::endl;

	scene.window = standard_window_initialization();
	project::path = cgp::project_path_find(argv[0], "shaders/");
	initialize_default_shaders();

	scene.initialize();

	// La boucle principale est différente en C++ natif et en WebAssembly (le navigateur appelle lui-même la fonction)
#ifndef __EMSCRIPTEN__
	while (!glfwWindowShouldClose(scene.window.glfw_window))
		animation_loop();
#else
	emscripten_set_main_loop(animation_loop, 0, 1);
#endif

	cgp::imgui_cleanup();
	glfwDestroyWindow(scene.window.glfw_window);
	glfwTerminate();

	return 0;
}

void animation_loop()
{
	emscripten_update_window_size(scene.window.width, scene.window.height);

	glViewport(0, 0, scene.window.width, scene.window.height);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	imgui_create_frame();
	ImGui::GetIO().FontGlobalScale = project::gui_scale;
	scene.inputs.mouse.on_gui = ImGui::GetIO().WantCaptureMouse;

	scene.display_frame();
	scene.display_gui();

	imgui_render_frame(scene.window.glfw_window);
	glfwSwapBuffers(scene.window.glfw_window);
	glfwPollEvents();
}

void initialize_default_shaders()
{
	std::string const dir = project::path + "shaders/";

	mesh_drawable::default_shader.load(dir + "mesh/mesh.vert.glsl", dir + "mesh/mesh.frag.glsl");
	triangles_drawable::default_shader.load(dir + "mesh/mesh.vert.glsl", dir + "mesh/mesh.frag.glsl");
	curve_drawable::default_shader.load(dir + "single_color/single_color.vert.glsl", dir + "single_color/single_color.frag.glsl");

	image_structure const white_image = image_structure{ 1, 1, image_color_type::rgba, { 255, 255, 255, 255 } };
	mesh_drawable::default_texture.initialize_texture_2d_on_gpu(white_image);
	triangles_drawable::default_texture.initialize_texture_2d_on_gpu(white_image);
}


// Fenêtre et callbacks GLFW
// ***************************************************************** //

void window_size_callback(GLFWwindow* window, int width, int height);
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_click_callback(GLFWwindow* window, int button, int action, int mods);
void keyboard_callback(GLFWwindow* window, int key, int, int action, int mods);

window_structure standard_window_initialization()
{
	window_structure window;
	window.initialize(0, 0, "TRON", CGP_OPENGL_VERSION_MAJOR, CGP_OPENGL_VERSION_MINOR);

	std::cout << "Window " << window.width << "x" << window.height << std::endl;
	std::cout << cgp::opengl_info_display() << std::endl;

	cgp::imgui_init(window.glfw_window);

	glfwSetMouseButtonCallback(window.glfw_window, mouse_click_callback);
	glfwSetCursorPosCallback(window.glfw_window, mouse_move_callback);
	glfwSetWindowSizeCallback(window.glfw_window, window_size_callback);
	glfwSetKeyCallback(window.glfw_window, keyboard_callback);
	glfwSetScrollCallback(window.glfw_window, mouse_scroll_callback);

	return window;
}

void window_size_callback(GLFWwindow*, int width, int height)
{
	scene.window.width = width;
	scene.window.height = height;
}

void mouse_move_callback(GLFWwindow*, double xpos, double ypos)
{
	vec2 const pos_relative = scene.window.convert_pixel_to_relative_coordinates({ xpos, ypos });
	scene.inputs.mouse.position.update(pos_relative);
	scene.mouse_move_event();
}

void mouse_click_callback(GLFWwindow* window, int button, int action, int mods)
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	scene.inputs.mouse.click.update_from_glfw_click(button, action);
	scene.mouse_click_event();
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	scene.inputs.mouse.scroll = float(yoffset);
	scene.mouse_scroll_event();
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	if (ImGui::GetIO().WantCaptureKeyboard)
		return;

	scene.inputs.keyboard.update_from_glfw_key(key, action);
	scene.keyboard_event();
}
