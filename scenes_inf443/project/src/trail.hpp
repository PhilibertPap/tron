#pragma once

#include "cgp/cgp.hpp"
#include "environment.hpp"

#include <deque>

// Longueur maximale qu'on peut donner à un mur (sert à pré-allouer le mesh)
constexpr float trail_length_limit = 400.0f;

// Mur de lumière laissé par une moto.
// Le tracé est une suite de points (au sol, sur une rampe ou en l'air pendant un saut), espacés
// d'au moins `spacing`, plus une extrémité mobile `head` collée à l'arrière de la moto.
// Le mur monte verticalement au-dessus du tracé.
// Le mesh est alloué une seule fois sur le GPU puis mis à jour à chaque frame (pas de réallocation).
struct trail_structure
{
	cgp::vec3 color;
	float max_length = 120.0f;

	std::deque<cgp::vec3> points; // du plus ancien au plus récent
	cgp::vec3 head;

	void initialize(cgp::vec3 const& color, cgp::opengl_shader_structure const& shader);
	void reset(cgp::vec3 const& start);
	void extend(cgp::vec3 const& new_head);

	// Vrai si une moto de hauteur moto_height, dont l'avant va de a à b, traverse le mur.
	// ignored_recent_segments : nombre de segments les plus récents à ignorer
	// (pour qu'une moto ne détecte pas son propre mur au niveau de sa roue arrière)
	bool hits(cgp::vec3 const& a, cgp::vec3 const& b, float moto_height, int ignored_recent_segments = 0) const;

	void draw(environment_structure const& environment) const;

private:
	float fixed_length = 0; // longueur entre points.front() et points.back()
	cgp::mesh_drawable drawable;
	cgp::numarray<cgp::vec3> vertex_positions;
	cgp::numarray<cgp::vec2> vertex_uv;

	void update_gpu();
};
