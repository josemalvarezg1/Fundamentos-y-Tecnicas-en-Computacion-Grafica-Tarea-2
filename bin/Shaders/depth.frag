#version 330
out vec4 color;
in vec2 TexCoords;

uniform sampler2D depthMap;

void main() {

    float val = texture(depthMap, TexCoords).r;
    color = vec4(vec3(val), 1.0);
    
}  