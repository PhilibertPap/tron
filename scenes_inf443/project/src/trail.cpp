#include "trail.hpp"

using namespace cgp;

mesh create_trail(const std::vector<vec3>& positions)
{
	mesh trail_mesh;
	int N = positions.size();
	if (positions.size() >= 2) {
		for (size_t i = 0; i+1 < N; ++i) {
			vec3 p1 = positions[i];
			vec3 p2 = positions[i + 1];
			vec3 p3 = p1 + vec3(0.0f, 0.0f, 0.5f);
			vec3 p4 = p2 + vec3(0.0f, 0.0f, 0.5f);

			trail_mesh.position.push_back(p1);
			trail_mesh.position.push_back(p2);
			trail_mesh.position.push_back(p3);
			trail_mesh.position.push_back(p4);

			// Connectivité
			trail_mesh.connectivity.push_back({ i * 4, i * 4 + 1, i * 4 + 2 });
			trail_mesh.connectivity.push_back({ i * 4 + 1, i * 4 + 3, i * 4 + 2 });
		}
	}
	for (vec3 const& p : trail_mesh.position) {
		trail_mesh.color.push_back({ 0.0f, 0.8f, 1.0f });
	}
	trail_mesh.fill_empty_field();
	return trail_mesh;
}