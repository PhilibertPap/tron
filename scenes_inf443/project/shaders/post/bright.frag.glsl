#version 330 core

// Ne garde que les zones lumineuses de l'image
in vec2 uv;
layout(location=0) out vec4 FragColor;

uniform sampler2D image_texture;

const float threshold = 0.55;

void main()
{
	vec3 color = texture(image_texture, uv).rgb;
	float brightness = max(color.r, max(color.g, color.b));
	float weight = clamp((brightness - threshold) / (1.0 - threshold), 0.0, 1.0);
	FragColor = vec4(color * weight, 1.0);
}
