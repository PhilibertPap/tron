#version 330 core
precision highp float;

// Surfaces du niveau (sol, étages, rampes) : grille lumineuse calculée directement dans le shader.
// La largeur des lignes est exprimée en pixels grâce à fwidth, ce qui évite le scintillement
// des textures de grille vues en incidence rasante. Les motos éclairent le sol de leur couleur.
// uv : sur le dessus, position dans le rectangle de la surface ; sur les flancs, uv.y = 1 en haut.

in struct fragment_data
{
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
} fragment;

layout(location=0) out vec4 FragColor;

uniform sampler2D image_texture; // blanche par défaut
uniform vec2 surface_size;
uniform vec3 grid_color;

#define MOTO_LIGHTS 2
uniform vec3 moto_light_position[MOTO_LIGHTS];
uniform vec3 moto_light_color[MOTO_LIGHTS];

const vec3 base_color = vec3(0.005, 0.01, 0.02);
const float cell_size = 4.0;

// 1 sur une ligne de la grille, 0 ailleurs, avec un bord adouci d'environ un pixel
float grid_line(vec2 coord, float width_in_pixels)
{
	vec2 distance_to_line = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
	return 1.0 - clamp(min(distance_to_line.x, distance_to_line.y) - 0.5 * width_in_pixels, 0.0, 1.0);
}

vec3 moto_glow(vec3 p, float line)
{
	vec3 glow = vec3(0.0);
	for (int i = 0; i < MOTO_LIGHTS; ++i) {
		float d = length(moto_light_position[i] - p);
		glow += moto_light_color[i] * (0.15 + 1.5 * line) / (1.0 + 0.15 * d * d);
	}
	return glow;
}

void main()
{
	vec3 p = fragment.position;
	vec3 N = normalize(fragment.normal);
	vec3 tint = texture(image_texture, fragment.uv).rgb;

	float minor = grid_line(p.xy / cell_size, 1.0);
	float major = grid_line(p.xy / (5.0 * cell_size), 2.0);
	float line = max(0.5 * minor, major);

	vec3 color;
	if (N.z > 0.5) {
		// Dessus : grille + liseré sur le pourtour
		vec2 to_edge = min(fragment.uv, 1.0 - fragment.uv) * surface_size;
		float border = 1.0 - smoothstep(0.0, 0.35, min(to_edge.x, to_edge.y));
		color = base_color + grid_color * (line + 1.8 * border) + moto_glow(p, line);
	}
	else if (N.z < -0.5) {
		// Dessous d'un étage : sombre, grille à peine visible
		color = base_color + 0.25 * grid_color * line;
	}
	else {
		// Flancs : sombres, avec un liseré lumineux en haut
		float rim = smoothstep(0.7, 1.0, fragment.uv.y);
		color = base_color + 1.8 * grid_color * rim;
	}

	FragColor = vec4(color * tint, 1.0);
}
