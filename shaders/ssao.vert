#version 330
// Authorship: James Kortman (a1648090)

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Colour;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;

out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosDeviceSpace;

void main() {
    FragPos = vec3(ModelMatrix * vec4(a_Position, 1.0));
    Normal = NormalMatrix * a_Normal;
    gl_Position =
        ProjectionMatrix
        * ViewMatrix
        * ModelMatrix
        * vec4(a_Position, 1.0);
    FragPosDeviceSpace = gl_Position;
}
