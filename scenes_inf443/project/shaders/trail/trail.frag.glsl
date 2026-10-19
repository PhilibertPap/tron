#version 330 core
precision highp float;

// Mur de lumière : couleur pleine, plus intense sur les arêtes haute et basse,
// semi-transparent au milieu, et qui s'estompe vers la queue du mur.
// uv.x : position le long du mur (0 = queue, 1 = moto), uv.y : hauteur (0 = sol, 1 = haut)

in struct fragment_data
{
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
} fragment;

layout(location=0) out vec4 FragColor;

uniform sampler2D image_texture; // blanche par défaut
uniform vec3 trail_color;

void main()
{
	float v = fragment.uv.y;
	float edge = smoothstep(0.82, 1.0, v) + 0.5 * smoothstep(0.15, 0.0, v);
	float tail_fade = smoothstep(0.0, 0.06, fragment.uv.x);

	vec3 color = trail_color * (0.45 + 1.2 * edge) + 0.6 * edge;
	float alpha = (0.4 + 0.6 * edge) * tail_fade;

	FragColor = vec4(color, alpha) * texture(image_texture, fragment.uv);
}
