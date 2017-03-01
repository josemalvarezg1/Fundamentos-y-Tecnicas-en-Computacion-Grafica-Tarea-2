#version 330

layout (location = 0) in vec3 position; 
layout (location = 1) in vec2 coord_texturas;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangents;

uniform mat4 view_matrix;
uniform mat4 projection_matrix; 
uniform mat4 model_matrix; 
uniform mat4 reflection_matrix; 
uniform mat4 planar_matrix; 
uniform mat4 light_matrix;
uniform vec3 lightPos; 
uniform vec3 view; 
uniform vec3 lightSpotDir;
uniform bool mirror;

out vec3 FragPos;
out vec3 normales;
out vec3 viewPos;
out vec2 coordTex;
out vec3 lightPosfrag;
out vec4 FragPosLightSpace;
out mat3 TBN;

void main() {

	gl_Position = projection_matrix * view_matrix * planar_matrix * reflection_matrix * model_matrix * vec4(position, 1.0); //La posición viene por la mult de matrices

	coordTex = coord_texturas;
	mat4 model = planar_matrix * reflection_matrix * model_matrix;

	FragPos = vec3(model * vec4(position, 1.0));
	lightPosfrag = lightPos;
	viewPos = view;
	normales = mat3(transpose(inverse(model))) * normal;

    vec3 T = normalize(normalize(tangents) - dot(normalize(tangents), normalize(normales)) * normalize(normales));
    vec3 N = normalize(normales);  
    vec3 B = cross(T, N);
    TBN = mat3(T, B, N); 

    FragPosLightSpace = light_matrix * vec4(FragPos, 1.0);

}