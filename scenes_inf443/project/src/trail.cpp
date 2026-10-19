#include "trail.hpp"
#include "geometry.hpp"

using namespace cgp;

namespace
{
	constexpr float spacing = 0.5f;  // distance minimale entre deux points du mur
	constexpr float height = 1.1f;   // hauteur du mur
	constexpr int capacity = int(trail_length_limit / spacing) + 8; // nombre max de points
}

void trail_structure::initialize(vec3 const& color_arg, opengl_shader_structure const& shader)
{
	color = color_arg;

	// Chaque point du tracé donne deux sommets (bas et haut du mur), reliés en bande de triangles
	// (positions provisoires non dégénérées, remplacées dès le premier update_gpu)
	mesh m;
	for (int k = 0; k < capacity; ++k) {
		m.position.push_back({ float(k), 0, 0 });
		m.position.push_back({ float(k), 0, height });
	}
	for (int k = 0; k < capacity - 1; ++k) {
		unsigned int const i = 2 * k;
		m.connectivity.push_back({ i, i + 2, i + 1 });
		m.connectivity.push_back({ i + 1, i + 2, i + 3 });
	}
	m.fill_empty_field();
	drawable.initialize_data_on_gpu(m, shader);

	vertex_positions.resize(2 * capacity);
	vertex_uv.resize(2 * capacity);
	reset({ 0, 0, 0 });
}

void trail_structure::reset(vec3 const& start)
{
	points.clear();
	points.push_back(start);
	head = start;
	fixed_length = 0;
	update_gpu();
}

void trail_structure::extend(vec3 const& new_head)
{
	head = new_head;

	float const d = norm(head - points.back());
	if (d >= spacing) {
		points.push_back(head);
		fixed_length += d;
	}

	// Le mur se raccourcit par la queue
	while (points.size() > 2 && (fixed_length > max_length || points.size() >= capacity)) {
		fixed_length -= norm(points[1] - points[0]);
		points.pop_front();
	}

	update_gpu();
}

bool trail_structure::hits(vec3 const& a, vec3 const& b, float moto_height, int ignored_recent_segments) const
{
	// Segment k = 0 : segment mobile [points.back(), head], puis on remonte vers la queue
	int const n = int(points.size());
	for (int k = ignored_recent_segments; k < n; ++k) {
		vec3 const& p = (k == 0) ? head : points[n - k];
		vec3 const& q = points[n - 1 - k];

		float t, u;
		if (!segments_intersect(xy(a), xy(b), xy(p), xy(q), t, u))
			continue;

		// Les tracés se croisent vu du dessus : il y a choc si les hauteurs se recouvrent
		// (on peut passer sous un mur posé à l'étage, ou sauter par-dessus)
		float const z_moto = a.z + t * (b.z - a.z);
		float const z_wall = p.z + u * (q.z - p.z);
		if (z_moto < z_wall + height && z_moto + moto_height > z_wall)
			return true;
	}
	return false;
}

void trail_structure::update_gpu()
{
	int const n = int(points.size()) + 1; // points fixes + extrémité mobile
	float const total_length = fixed_length + norm(head - points.back());

	float s = 0; // abscisse curviligne depuis la queue
	for (int k = 0; k < capacity; ++k) {
		// Les sommets inutilisés sont tous ramenés sur la tête : triangles dégénérés, invisibles
		int const i = std::min(k, n - 1);
		vec3 const& p = (i < n - 1) ? points[i] : head;
		if (k > 0 && k < n)
			s += norm(p - points[i - 1]);
		float const u = total_length > 0 ? s / total_length : 1.0f;

		vertex_positions[2 * k] = p;
		vertex_positions[2 * k + 1] = p + vec3{ 0, 0, height };
		vertex_uv[2 * k] = { u, 0.0f };
		vertex_uv[2 * k + 1] = { u, 1.0f };
	}

	drawable.vbo_position.update(vertex_positions);
	drawable.vbo_uv.update(vertex_uv);
}

void trail_structure::draw(environment_structure const& environment) const
{
	uniform_generic_structure uniforms;
	uniforms.uniform_vec3["trail_color"] = color;
	cgp::draw(drawable, environment, 1, false, uniforms);
}
