#version 330

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;
uniform mat4 LightSpaceMatrix;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosDeviceSpace;
out vec4 FragPosLightSpace;

void main() {
    TexCoord = a_TexCoord;
    Normal = NormalMatrix * a_Normal;
    FragPosLightSpace = LightSpaceMatrix * vec4(FragPos, 1.0);
    gl_Position =
        ProjectionMatrix
        * ViewMatrix
        * ModelMatrix
        * vec4(a_Position, 1.0);
    FragPosDeviceSpace = gl_Position;
}
