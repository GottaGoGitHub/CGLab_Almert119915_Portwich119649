#version 150
#extension GL_ARB_explicit_attrib_location : require
// Khronos information about extension :
// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_explicit_attrib_location.txt
// VAO attributes

layout(location = 0) in vec4 in_Position;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

out vec3 camDirection;

// https://gamedev.stackexchange.com/questions/60313/implementing-a-skybox-with-glsl-version-330/60377

void main() {
    mat4 inverseProj = inverse(ProjectionMatrix);
    mat3 inverseModView = transpose(mat3(ViewMatrix));
    vec3 unprojected = (inverseProj * in_Position).xyz;
    camDirection = inverseModView * unprojected;
    gl_Position = in_Position;
}