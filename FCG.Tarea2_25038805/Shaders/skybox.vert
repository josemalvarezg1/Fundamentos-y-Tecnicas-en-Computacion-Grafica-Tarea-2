#version 330

layout (location=0) in vec3 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

out vec3 TexCoords;

void main() {

	gl_Position = projection * view * vec4(position, 1.0);
	TexCoords = position;

}
