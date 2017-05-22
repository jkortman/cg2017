#version 330

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_TexCoord;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;

out vec4 Colour;
out vec3 Normal;

void main() {
    vec4 base_colour = vec4(0.0, 0.7, 0.1, 1.0);
    Colour = vec4(vec3(base_colour) + vec3(a_Position.y / 4.0), 1.0);
    Normal = NormalMatrix * a_Normal;
    gl_Position =
        ProjectionMatrix
        * ViewMatrix
        * ModelMatrix
        * vec4(a_Position, 1.0);
}
