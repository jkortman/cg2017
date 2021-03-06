#version 330
// Authorship: Jeremy Hughes (a1646624)

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

out vec3 FragPos;
out vec3 FragPosWorld;

void main() {
    FragPosWorld = vec3(ModelMatrix * vec4(a_Position, 1.0));
    FragPos = a_Position;
    gl_Position =
        ProjectionMatrix
        * ViewMatrix
        * ModelMatrix
        * vec4(a_Position, 1.0);
}
