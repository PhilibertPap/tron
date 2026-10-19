#pragma once

#include "cgp/cgp.hpp"
#include "environment.hpp"

// Modèle 3D d'une moto, construit à partir de primitives.
// Repère local : x vers l'avant, z vers le haut, origine au sol entre les deux roues.
struct moto_model
{
	cgp::hierarchy_mesh_drawable hierarchy;

	void initialize(cgp::vec3 const& color);

	// heading : cap dans le plan, lean : inclinaison dans les virages, pitch : piqué du nez (chute)
	void set_pose(cgp::vec3 const& position, float heading, float lean, float pitch);

	void draw(environment_structure const& environment);
};
