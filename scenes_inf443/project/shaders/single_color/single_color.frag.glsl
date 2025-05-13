#version 330 core


// Inputs coming from the vertex shader
in struct fragment_data
{
    vec3 position; // position in the world space
    vec3 normal;   // normal in the world space
    vec3 color;    // current color on the fragment
    vec2 uv;       // current uv-texture on the fragment

} fragment;

// Output of the fragment shader - output color
layout(location=0) out vec4 FragColor;

// View matrix
uniform mat4 view;

struct material_structure
{
	vec3 color;  // Uniform color of the object
};
uniform material_structure material;

// Ambiant uniform controled from the GUI
uniform float ambiant;
uniform float diffuse;
uniform float specular;
uniform float specular_exp;
uniform float d_max;
uniform float dl_max;

uniform vec3 light_color;
uniform vec3 light_position;

uniform vec3 fog_color;

uniform vec3 light_color2;
uniform vec3 light_position2;

void main()
{
    vec3 current_color;
    vec3 current_color1;
    vec3 current_color2;
  
    mat3 O = transpose(mat3(view)); // get the orientation matrix
    vec3 last_col = vec3(view * vec4(0.0, 0.0, 0.0, 1.0)); // get the last column
    vec3 camera_position = -O * last_col;
    vec3 L1 = normalize(light_position - fragment.position);
    vec3 L2 = normalize(light_position2 - fragment.position);
    vec3 N = normalize(fragment.normal);
    vec3 R1 = normalize(reflect(fragment.position - light_position, fragment.normal));
    vec3 R2 = normalize(reflect(fragment.position - light_position2, fragment.normal));
    vec3 V = normalize(camera_position - fragment.position);

    float d = length(camera_position - fragment.position);
    float dl1 = length(light_position - fragment.position);
    float dl2 = length(light_position2 - fragment.position);

    float diffuse_value1 = max(dot(N, L1), 0.0);
    float diffuse_magnitude1 = diffuse * diffuse_value1;
    float diffuse_value2 = max(dot(N, L2), 0.0);
    float diffuse_magnitude2 = diffuse * diffuse_value2;

    float specular_value1 = pow(max(dot(R1, V), 0.0), specular_exp);
    float specular_magnitude1 = specular * specular_value1;
    float specular_value2 = pow(max(dot(R2, V), 0.0), specular_exp);
    float specular_magnitude2 = specular * specular_value2;

    float fog_value = min(d / d_max, 1.0);
    float attenuation_value1 = min(dl1 / dl_max, 1.0);
    float attenuation_value2 = min(dl2 / dl_max, 1.0);

    current_color1 = ((ambiant + diffuse_magnitude1) * material.color + specular_magnitude1)* light_color;
    current_color1 = (1 - fog_value) * current_color1 + fog_value * fog_color;
    current_color1 = (1 - attenuation_value1) * current_color1;

    current_color2 = ((ambiant + diffuse_magnitude2) * material.color + specular_magnitude2)* light_color2;
    current_color2 = (1 - fog_value) * current_color2 + fog_value * fog_color;
    current_color2 = (1 - attenuation_value2) * current_color2;
    if (light_position2.z < -0.51f) 
    current_color2 = vec3(0.0);

    current_color = current_color1 + current_color2;
    FragColor = vec4(current_color, 1.0);
}