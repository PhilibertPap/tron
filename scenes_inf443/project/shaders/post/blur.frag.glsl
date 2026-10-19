#version 330 core

// Flou gaussien à 9 échantillons dans une seule direction (texel_step)
in vec2 uv;
layout(location=0) out vec4 FragColor;

uniform sampler2D image_texture;
uniform vec2 texel_step;

const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
	vec3 color = texture(image_texture, uv).rgb * weights[0];
	for (int i = 1; i < 5; ++i) {
		vec2 offset = 1.5 * float(i) * texel_step;
		color += texture(image_texture, uv + offset).rgb * weights[i];
		color += texture(image_texture, uv - offset).rgb * weights[i];
	}
	FragColor = vec4(color, 1.0);
}
