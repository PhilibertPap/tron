#version 330 core

// Rectangle plein écran pour les passes de post-traitement
layout (location = 0) in vec3 vertex_position;
layout (location = 3) in vec2 vertex_uv;

out vec2 uv;

void main()
{
	uv = vertex_uv;
	gl_Position = vec4(vertex_position.xy, 0.0, 1.0);
}
