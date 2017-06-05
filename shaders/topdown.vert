#version 330

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Colour;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;
uniform mat4 LightSpaceMatrix;

out vec3 Colour;
out vec3 Normal;
out vec3 FragPos;

void main() {
    FragPos = vec3(ModelMatrix * vec4(a_Position, 1.0));
    Colour = a_Colour;
    Normal = NormalMatrix * a_Normal;
    //FragPosLightSpace = LightSpaceMatrix * vec4(FragPos, 1.0);
    gl_Position =
        ProjectionMatrix
        * ViewMatrix
        * ModelMatrix
        * vec4(a_Position, 1.0);
}
