#pragma once

#include "cgp/cgp.hpp"
#include "environment.hpp"

#include <vector>

// Surface sur laquelle on peut rouler : plateau horizontal ou rampe.
// L'emprise au sol est un rectangle aligné sur les axes ; l'altitude varie linéairement
// le long de x (slope_axis = 0) ou de y (slope_axis = 1).
struct surface_structure
{
	cgp::vec2 corner_min;
	cgp::vec2 corner_max;
	float z_at_min = 0;    // altitude du côté corner_min, le long de l'axe de la pente
	float z_at_max = 0;    // altitude du côté corner_max
	int slope_axis = 0;
	bool solid = false;    // rampe pleine jusqu'en bas, ou dalle fine sous laquelle on peut passer
	float thickness = 0.6f;
	cgp::vec3 grid_color = { 0.0f, 0.35f, 0.55f };

	bool contains(cgp::vec2 const& p) const;
	float top(cgp::vec2 const& p) const;
	float bottom(cgp::vec2 const& p) const;
	cgp::vec2 size() const { return corner_max - corner_min; }
	cgp::mesh build_mesh() const;
};

// Le niveau : un grand plateau au sol, des étages reliés par des rampes, des tremplins,
// et le vide tout autour.
struct level_structure
{
	float floor_size = 160.0f;
	float void_altitude = -4.0f; // une moto qui descend plus bas est perdue

	std::vector<surface_structure> surfaces;

	void initialize(cgp::opengl_shader_structure const& ground_shader);

	// Altitude du sol sous p pour une moto à l'altitude z. On ne considère que les surfaces
	// atteignables (pas un étage situé au-dessus de la moto). Renvoie false s'il n'y a que le vide.
	bool ground_height(cgp::vec2 const& p, float z, float& height) const;

	// Vrai si un volume du décor occupe l'espace de la moto (flanc d'une rampe, bord d'un étage...)
	bool blocks(cgp::vec2 const& p, float z, float moto_height) const;

	// Point au hasard sur la surface la plus haute à cet endroit (pour placer les bonus)
	cgp::vec3 random_point_on_top() const;

	void draw_skybox(environment_structure const& environment) const;
	void draw(environment_structure const& environment) const;

private:
	std::vector<cgp::mesh_drawable> drawables;
	cgp::skybox_drawable skybox;

	void add_platform(cgp::vec2 const& corner_min, cgp::vec2 const& corner_max, float z);
	void add_ramp(cgp::vec2 const& corner_min, cgp::vec2 const& corner_max, int slope_axis, float z_at_min, float z_at_max, bool solid);
};
