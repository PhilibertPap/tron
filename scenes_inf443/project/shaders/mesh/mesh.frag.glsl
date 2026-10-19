#version 330 core

// Shader par défaut des objets (motos, bonus, éclats).
// Eclairage de Phong avec :
//  - une lumière ambiante froide
//  - une lumière d'appoint venant du dessus
//  - deux lumières ponctuelles colorées, portées par les motos
// Un objet "lumineux" a simplement ambient = 1, diffuse = specular = 0.

in struct fragment_data
{
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
} fragment;

layout(location=0) out vec4 FragColor;

uniform sampler2D image_texture;
uniform mat4 view;

#define MOTO_LIGHTS 2
uniform vec3 moto_light_position[MOTO_LIGHTS];
uniform vec3 moto_light_color[MOTO_LIGHTS];

struct phong_structure {
	float ambient;
	float diffuse;
	float specular;
	float specular_exponent;
};

struct texture_settings_structure {
	bool use_texture;
	bool texture_inverse_v;
	bool two_sided;
};

struct material_structure
{
	vec3 color;
	float alpha;
	phong_structure phong;
	texture_settings_structure texture_settings;
};

uniform material_structure material;

const vec3 fill_light_direction = vec3(0.3, -0.4, 0.87);
const vec3 fill_light_color = vec3(0.35, 0.45, 0.6);

void main()
{
	mat3 O = transpose(mat3(view));
	vec3 last_col = vec3(view * vec4(0.0, 0.0, 0.0, 1.0));
	vec3 camera_position = -O * last_col;

	vec3 N = normalize(fragment.normal);
	if (material.texture_settings.two_sided && gl_FrontFacing == false)
		N = -N;
	vec3 V = normalize(camera_position - fragment.position);

	// Lumière d'appoint
	vec3 L = normalize(fill_light_direction);
	vec3 diffuse = max(dot(N, L), 0.0) * fill_light_color;
	vec3 specular = pow(max(dot(reflect(-L, N), V), 0.0), material.phong.specular_exponent) * fill_light_color;

	// Lumières des motos
	for (int i = 0; i < MOTO_LIGHTS; ++i) {
		vec3 to_light = moto_light_position[i] - fragment.position;
		float d = length(to_light);
		L = to_light / d;
		float attenuation = 1.0 / (1.0 + 0.08 * d + 0.03 * d * d);
		diffuse += max(dot(N, L), 0.0) * attenuation * moto_light_color[i];
		specular += pow(max(dot(reflect(-L, N), V), 0.0), material.phong.specular_exponent) * attenuation * moto_light_color[i];
	}

	vec2 uv_image = fragment.uv;
	if (material.texture_settings.texture_inverse_v)
		uv_image.y = 1.0 - uv_image.y;
	vec4 color_image_texture = texture(image_texture, uv_image);
	if (material.texture_settings.use_texture == false)
		color_image_texture = vec4(1.0);

	vec3 color_object = fragment.color * material.color * color_image_texture.rgb;

	vec3 color_shading = (material.phong.ambient + material.phong.diffuse * diffuse) * color_object
	                   + material.phong.specular * specular;

	FragColor = vec4(color_shading, material.alpha * color_image_texture.a);
}
