#include "level.hpp"

using namespace cgp;

namespace
{
	constexpr float step_height = 0.6f; // dénivelé qu'une moto franchit sans s'arrêter

	// Couleur de la grille selon l'étage, pour qu'on les distingue d'un coup d'oeil
	vec3 const floor_color = { 0.0f, 0.35f, 0.55f };
	vec3 const first_floor_color = { 0.05f, 0.55f, 0.6f };
	vec3 const second_floor_color = { 0.35f, 0.3f, 0.8f };
	vec3 const kicker_color = { 0.7f, 0.1f, 0.6f };
}


// Surface
// ***************************************************************** //

bool surface_structure::contains(vec2 const& p) const
{
	return p.x >= corner_min.x && p.x <= corner_max.x && p.y >= corner_min.y && p.y <= corner_max.y;
}

float surface_structure::top(vec2 const& p) const
{
	float const t = (p[slope_axis] - corner_min[slope_axis]) / (corner_max[slope_axis] - corner_min[slope_axis]);
	return z_at_min + t * (z_at_max - z_at_min);
}

float surface_structure::bottom(vec2 const& p) const
{
	// Une rampe pleine s'enfonce un peu dans le sol pour ne pas laisser de jour en dessous
	if (solid)
		return std::min(z_at_min, z_at_max) - 0.3f;
	return top(p) - thickness;
}

mesh surface_structure::build_mesh() const
{
	vec2 const corners[4] = { corner_min, { corner_max.x, corner_min.y }, corner_max, { corner_min.x, corner_max.y } };
	vec3 up[4], down[4];
	for (int k = 0; k < 4; ++k) {
		up[k] = { corners[k].x, corners[k].y, top(corners[k]) };
		down[k] = { corners[k].x, corners[k].y, bottom(corners[k]) };
	}

	mesh m = mesh_primitive_quadrangle(up[0], up[1], up[2], up[3]);
	if (!solid)
		m.push_back(mesh_primitive_quadrangle(down[0], down[3], down[2], down[1]));
	for (int k = 0; k < 4; ++k) {
		int const next = (k + 1) % 4;
		m.push_back(mesh_primitive_quadrangle(down[k], down[next], up[next], up[k]));
	}
	return m;
}


// Niveau
// ***************************************************************** //

void level_structure::add_platform(vec2 const& corner_min, vec2 const& corner_max, float z)
{
	add_ramp(corner_min, corner_max, 0, z, z, false);
}

void level_structure::add_ramp(vec2 const& corner_min, vec2 const& corner_max, int slope_axis, float z_at_min, float z_at_max, bool solid)
{
	surface_structure s;
	s.corner_min = corner_min;
	s.corner_max = corner_max;
	s.slope_axis = slope_axis;
	s.z_at_min = z_at_min;
	s.z_at_max = z_at_max;
	s.solid = solid;

	float const highest = std::max(z_at_min, z_at_max);
	if (highest >= 16)
		s.grid_color = second_floor_color;
	else if (highest >= 8)
		s.grid_color = first_floor_color;
	else
		s.grid_color = floor_color;

	surfaces.push_back(s);
}

void level_structure::initialize(opengl_shader_structure const& ground_shader)
{
	float const h = floor_size / 2;

	// Sol principal : une dalle épaisse
	add_platform({ -h, -h }, { h, h }, 0);
	surfaces.back().thickness = 2.0f;

	// Premier étage (8 m) : deux terrasses reliées par une passerelle, une rampe depuis le sol pour chacune
	add_platform({ -70, -25 }, { -30, 25 }, 8);
	add_platform({ 30, -25 }, { 70, 25 }, 8);
	add_platform({ -30, 18 }, { 30, 25 }, 8);
	add_ramp({ -30, -6 }, { -10, 6 }, 0, 8, 0, true);
	add_ramp({ 10, -6 }, { 30, 6 }, 0, 0, 8, true);

	// Deuxième étage (16 m) : terrasse nord, accessible depuis les terrasses du premier étage
	add_platform({ -60, 45 }, { 60, 70 }, 16);
	add_ramp({ -62, 25 }, { -50, 45 }, 1, 8, 16, false);
	add_ramp({ 50, 25 }, { 62, 45 }, 1, 8, 16, false);

	// Tremplins : la rampe s'arrête net, la moto décolle avec sa vitesse
	add_ramp({ -25, -49 }, { -15, -42 }, 1, 0, 3.0f, true);
	add_ramp({ 15, -49 }, { 25, -42 }, 1, 0, 3.0f, true);
	surfaces[surfaces.size() - 1].grid_color = kicker_color;
	surfaces[surfaces.size() - 2].grid_color = kicker_color;

	for (surface_structure const& s : surfaces) {
		mesh_drawable d;
		d.initialize_data_on_gpu(s.build_mesh(), ground_shader);
		drawables.push_back(d);
	}

	// Les faces du cubemap sont données dans l'ordre attendu par CGP : x-, x+, y-, y+, z-, z+
	std::string const dir = project::path + "assets/skybox/";
	skybox.initialize_data_on_gpu();
	skybox.texture.initialize_cubemap_on_gpu(
		image_load_file(dir + "nx.png"),
		image_load_file(dir + "px.png"),
		image_load_file(dir + "ny.png"),
		image_load_file(dir + "py.png"),
		image_load_file(dir + "nz.png"),
		image_load_file(dir + "pz.png"));
}

bool level_structure::ground_height(vec2 const& p, float z, float& height) const
{
	bool found = false;
	for (surface_structure const& s : surfaces) {
		if (!s.contains(p))
			continue;
		float const t = s.top(p);
		if (t <= z + step_height && (!found || t > height)) {
			height = t;
			found = true;
		}
	}
	return found;
}

bool level_structure::blocks(vec2 const& p, float z, float moto_height) const
{
	for (surface_structure const& s : surfaces) {
		if (s.contains(p) && z < s.top(p) - step_height && z + moto_height > s.bottom(p))
			return true;
	}
	return false;
}

vec3 level_structure::random_point_on_top() const
{
	float const margin = 5.0f;
	float const h = floor_size / 2 - margin;
	vec2 const p = { rand_interval(-h, h), rand_interval(-h, h) };

	float z = 0;
	for (surface_structure const& s : surfaces)
		if (s.contains(p))
			z = std::max(z, s.top(p));
	return { p.x, p.y, z };
}

void level_structure::draw_skybox(environment_structure const& environment) const
{
	glDepthMask(GL_FALSE);
	cgp::draw(skybox, environment);
	glDepthMask(GL_TRUE);
}

void level_structure::draw(environment_structure const& environment) const
{
	for (size_t k = 0; k < surfaces.size(); ++k) {
		uniform_generic_structure uniforms;
		uniforms.uniform_vec2["surface_size"] = surfaces[k].size();
		uniforms.uniform_vec3["grid_color"] = surfaces[k].grid_color;
		cgp::draw(drawables[k], environment, 1, false, uniforms);
	}
}
