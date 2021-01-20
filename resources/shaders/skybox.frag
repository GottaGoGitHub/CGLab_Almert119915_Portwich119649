#version 150

uniform samplerCube samplerCube_tex;
in vec3 camDirection;

out vec4 out_Color;

void main() {
    out_Color = texture(samplerCube_tex, camDirection);
}