#pragma once

#include "cgp/cgp.hpp"

#include <string>

// Effet de lueur (bloom) en post-traitement :
//  1. la scène est rendue dans une texture
//  2. on garde les zones les plus lumineuses, à demi-résolution
//  3. on les floute plusieurs fois (flou gaussien séparé horizontal / vertical)
//  4. on ajoute ce halo à l'image de départ
struct bloom_effect
{
	float intensity = 1.2f;

	// Faux si la carte graphique (ou le navigateur) refuse nos textures de rendu
	bool is_supported() const { return supported; }

	void initialize(std::string const& shader_directory);

	// Tous les draw() suivants s'écrivent dans la texture de la scène
	void begin(int width, int height);
	// Calcule le halo et affiche le résultat final à l'écran
	void end_and_display();

private:
	// Image rendue hors écran : une texture couleur, avec ou sans z-buffer
	struct render_target
	{
		GLuint fbo = 0;
		GLuint depth_buffer = 0;
		cgp::opengl_texture_image_structure texture;

		void initialize(bool with_depth);
		void resize(int width, int height);
		void bind() const;
	};

	bool supported = false;
	int width = 0;
	int height = 0;

	render_target scene_target;
	render_target blur_target[2];

	cgp::mesh_drawable screen_quad;
	cgp::opengl_shader_structure bright_shader;
	cgp::opengl_shader_structure blur_shader;
	cgp::opengl_shader_structure composite_shader;

	void draw_pass(cgp::opengl_shader_structure const& shader, cgp::opengl_texture_image_structure const& input, cgp::uniform_generic_structure const& uniforms);
};
