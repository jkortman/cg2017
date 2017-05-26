#version 330

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;
uniform float Time;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;

void main()
{
    FragPos = vec3(ModelMatrix * vec4(a_Position, 1.0));
    TexCoord = a_TexCoord;
    Normal = NormalMatrix * a_Normal;

    gl_Position =
        ProjectionMatrix
        * ViewMatrix
        * ModelMatrix
        * vec4(a_Position, 1.0);
}

