#version 330

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;

out vec2 TexCoord;

void main() {
    TexCoord = a_TexCoord;
    gl_Position =
        ProjectionMatrix
        * ViewMatrix
        * ModelMatrix
        * vec4(a_Position, 1.0);
}
