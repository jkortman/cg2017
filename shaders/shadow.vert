#version 330

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

uniform mat4 LightSpaceMatrix;
uniform mat4 ModelMatrix;

void main() {
    gl_Position =
        LightSpaceMatrix
        * ModelMatrix
        * vec4(a_Position, 1.0);
}