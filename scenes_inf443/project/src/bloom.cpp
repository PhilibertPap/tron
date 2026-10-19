#include "bloom.hpp"

using namespace cgp;

namespace
{
	constexpr int blur_iterations = 3;
}


// Cible de rendu
// ***************************************************************** //

void bloom_effect::render_target::initialize(bool with_depth)
{
	texture.initialize_texture_2d_on_gpu(1, 1, GL_RGB8);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id, 0);

	if (with_depth) {
		glGenRenderbuffers(1, &depth_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1, 1);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void bloom_effect::render_target::resize(int width, int height)
{
	if (texture.width == width && texture.height == height)
		return;
	texture.width = width;
	texture.height = height;

	glBindTexture(GL_TEXTURE_2D, texture.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (depth_buffer != 0) {
		glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
}

void bloom_effect::render_target::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}


// Effet
// ***************************************************************** //

void bloom_effect::initialize(std::string const& dir)
{
	scene_target.initialize(true);
	blur_target[0].initialize(false);
	blur_target[1].initialize(false);

	scene_target.resize(64, 64);
	scene_target.bind();
	supported = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	bright_shader.load(dir + "screen.vert.glsl", dir + "bright.frag.glsl");
	blur_shader.load(dir + "screen.vert.glsl", dir + "blur.frag.glsl");
	composite_shader.load(dir + "screen.vert.glsl", dir + "composite.frag.glsl");

	// Rectangle qui couvre tout l'écran, en coordonnées normalisées [-1,1]
	screen_quad.initialize_data_on_gpu(mesh_primitive_quadrangle({ -1,-1,0 }, { 1,-1,0 }, { 1,1,0 }, { -1,1,0 }), composite_shader);
}

void bloom_effect::begin(int width_arg, int height_arg)
{
	width = width_arg;
	height = height_arg;

	scene_target.resize(width, height);
	scene_target.bind();
	glViewport(0, 0, width, height);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void bloom_effect::end_and_display()
{
	glDisable(GL_DEPTH_TEST);

	int const w = std::max(width / 2, 1);
	int const h = std::max(height / 2, 1);
	blur_target[0].resize(w, h);
	blur_target[1].resize(w, h);
	glViewport(0, 0, w, h);

	blur_target[0].bind();
	draw_pass(bright_shader, scene_target.texture, uniform_generic_structure());

	for (int k = 0; k < blur_iterations; ++k) {
		uniform_generic_structure horizontal;
		horizontal.uniform_vec2["texel_step"] = { 1.0f / w, 0.0f };
		blur_target[1].bind();
		draw_pass(blur_shader, blur_target[0].texture, horizontal);

		uniform_generic_structure vertical;
		vertical.uniform_vec2["texel_step"] = { 0.0f, 1.0f / h };
		blur_target[0].bind();
		draw_pass(blur_shader, blur_target[1].texture, vertical);
	}

	// Composition finale, directement à l'écran
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	uniform_generic_structure composite;
	composite.uniform_float["bloom_intensity"] = intensity;
	screen_quad.supplementary_texture["bloom_texture"] = blur_target[0].texture;
	draw_pass(composite_shader, scene_target.texture, composite);
	screen_quad.supplementary_texture.clear();

	glEnable(GL_DEPTH_TEST);
}

void bloom_effect::draw_pass(opengl_shader_structure const& shader, opengl_texture_image_structure const& input, uniform_generic_structure const& uniforms)
{
	screen_quad.shader = shader;
	screen_quad.texture = input;
	draw(screen_quad, environment_generic_structure(), 1, false, uniforms);
}
