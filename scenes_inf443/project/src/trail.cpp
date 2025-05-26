#include "trail.hpp"

using namespace cgp;

mesh create_trail(const std::vector<vec3>& positions, vec3& trail_color)
{
    mesh trail_mesh;
    int N = positions.size();
    if (positions.size() >= 2) {
        for (size_t i = 0; i + 1 < N; ++i) {
            vec3 p1 = positions[i];
            vec3 p2 = positions[i + 1];
            vec3 p3 = p1 + vec3(0.0f, 0.0f, 0.5f);
            vec3 p4 = p2 + vec3(0.0f, 0.0f, 0.5f);

            // Calcul de la normale pour ce quad
            vec3 u = p2 - p1;
            vec3 v = p3 - p1;
            vec3 normal_front = normalize(cross(u, v));
            vec3 normal_back = -normal_front;

            // Sommets face avant
            size_t base = trail_mesh.position.size();
            trail_mesh.position.push_back(p1);
            trail_mesh.position.push_back(p2);
            trail_mesh.position.push_back(p3);
            trail_mesh.position.push_back(p4);
            for (int j = 0; j < 4; ++j)
                trail_mesh.normal.push_back(normal_front);

            // Sommets face arrière
            trail_mesh.position.push_back(p1);
            trail_mesh.position.push_back(p2);
            trail_mesh.position.push_back(p3);
            trail_mesh.position.push_back(p4);
            for (int j = 0; j < 4; ++j)
                trail_mesh.normal.push_back(normal_back);

            // Triangles face avant
            trail_mesh.connectivity.push_back({ base, base + 1, base + 2 });
            trail_mesh.connectivity.push_back({ base + 1, base + 3, base + 2 });

            // Triangles face arrière (ordre inversé)
            trail_mesh.connectivity.push_back({ base + 6, base + 5, base + 4 });
            trail_mesh.connectivity.push_back({ base + 6, base + 7, base + 5 });
        }
    }
    for (vec3 const& p : trail_mesh.position) {
        trail_mesh.color.push_back(trail_color);
    }
    for (size_t i = 0; i < trail_mesh.position.size(); ++i) {
        trail_mesh.uv.push_back({ 0.0f, 0.0f });
    }
    // Pas de fill_empty_field() ici !
    return trail_mesh;
}