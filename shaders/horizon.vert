#version 330

layout (location = 0) in vec3 a_Position;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

void main() {
    gl_Position =
        ProjectionMatrix
        * ViewMatrix
        * ModelMatrix
        * vec4(a_Position, 1.0);
}
