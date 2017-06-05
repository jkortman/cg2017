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
    float water_level = 0.05*128.0;
    vec3 pos = a_Position;
    pos -= vec3(0.0, water_level, 0.0);
    pos *= vec3(1.0, -1.0, 1.0);
    pos += vec3(0.0, water_level, 0.0);
    FragPos = vec3(ModelMatrix * vec4(pos, 1.0));
    Colour = a_Colour;
    Normal = NormalMatrix * a_Normal;
    //FragPosLightSpace = LightSpaceMatrix * vec4(FragPos, 1.0);
    gl_Position =
        ProjectionMatrix
        * ViewMatrix
        * ModelMatrix
        * vec4(pos, 1.0);
}
