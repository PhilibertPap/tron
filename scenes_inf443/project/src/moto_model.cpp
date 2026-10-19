#include "moto_model.hpp"

using namespace cgp;

namespace
{
	mesh_drawable make_part(mesh const& m, vec3 const& color, float specular = 0.6f)
	{
		mesh_drawable part;
		part.initialize_data_on_gpu(m);
		part.material.color = color;
		part.material.phong = { 0.15f, 0.7f, specular, 64.0f };
		return part;
	}

	// Pièce lumineuse : couleur pleine, non éclairée
	mesh_drawable make_glowing_part(mesh const& m, vec3 const& color)
	{
		mesh_drawable part;
		part.initialize_data_on_gpu(m);
		part.material.color = color;
		part.material.phong = { 1.0f, 0.0f, 0.0f, 1.0f };
		return part;
	}
}

void moto_model::initialize(vec3 const& color)
{
	vec3 const dark_metal = { 0.16f, 0.18f, 0.22f };
	vec3 const black_glass = { 0.03f, 0.04f, 0.06f };

	float const wheel_radius = 0.42f;
	float const tire_thickness = 0.15f;
	float const wheel_z = wheel_radius + tire_thickness;
	float const wheel_x = 0.95f;

	mesh const tire = mesh_primitive_torus(wheel_radius, tire_thickness, { 0, 0, 0 }, { 0, 1, 0 });
	mesh rim = mesh_primitive_torus(wheel_radius, 0.045f, { 0, tire_thickness, 0 }, { 0, 1, 0 });
	rim.push_back(mesh_primitive_torus(wheel_radius, 0.045f, { 0, -tire_thickness, 0 }, { 0, 1, 0 }));

	// Noeuds vides servant de pivots : "moto" porte le cap et la position, "chassis" l'inclinaison
	hierarchy.add(mesh_drawable(), "moto");
	hierarchy.add(mesh_drawable(), "chassis", "moto");

	hierarchy.add(make_part(mesh_primitive_ellipsoid({ 1.15f, 0.30f, 0.32f }, { 0, 0, 0.66f }), dark_metal), "body", "chassis");
	hierarchy.add(make_part(mesh_primitive_ellipsoid({ 0.55f, 0.22f, 0.18f }, { -0.2f, 0, 0.93f }), black_glass, 1.0f), "cockpit", "chassis");
	hierarchy.add(make_glowing_part(mesh_primitive_ellipsoid({ 1.17f, 0.31f, 0.08f }, { 0, 0, 0.62f }), color), "stripe", "chassis");
	hierarchy.add(make_glowing_part(mesh_primitive_sphere(0.08f, { 1.12f, 0, 0.72f }), { 1, 1, 1 }), "headlight", "chassis");
	hierarchy.add(make_glowing_part(mesh_primitive_sphere(0.07f, { -1.13f, 0, 0.72f }), color), "taillight", "chassis");

	hierarchy.add(make_part(tire, dark_metal, 0.2f), "front_tire", "chassis", { wheel_x, 0, wheel_z });
	hierarchy.add(make_part(tire, dark_metal, 0.2f), "rear_tire", "chassis", { -wheel_x, 0, wheel_z });
	hierarchy.add(make_glowing_part(rim, color), "front_rim", "front_tire");
	hierarchy.add(make_glowing_part(rim, color), "rear_rim", "rear_tire");
}

void moto_model::set_pose(vec3 const& position, float heading, float lean, float pitch)
{
	rotation_transform const yaw = rotation_transform::from_axis_angle({ 0, 0, 1 }, heading);
	rotation_transform const nose = rotation_transform::from_axis_angle({ 0, 1, 0 }, pitch);

	hierarchy["moto"].transform_local.translation = position;
	hierarchy["moto"].transform_local.rotation = yaw * nose;
	hierarchy["chassis"].transform_local.rotation = rotation_transform::from_axis_angle({ 1, 0, 0 }, -lean);

	hierarchy.update_local_to_global_coordinates();
}

void moto_model::draw(environment_structure const& environment)
{
	cgp::draw(hierarchy, environment);
}
