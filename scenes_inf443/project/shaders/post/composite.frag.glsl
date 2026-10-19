#version 330 core

// Image finale = scène + halo
in vec2 uv;
layout(location=0) out vec4 FragColor;

uniform sampler2D image_texture;
uniform sampler2D bloom_texture;
uniform float bloom_intensity;

void main()
{
	vec3 scene = texture(image_texture, uv).rgb;
	vec3 bloom = texture(bloom_texture, uv).rgb;
	FragColor = vec4(scene + bloom_intensity * bloom, 1.0);
}
