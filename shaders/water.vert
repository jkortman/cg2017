#version 330

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Colour;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;
uniform mat4 LightSpaceMatrix;
uniform float Time;
uniform float VertDist;

out vec3 Colour;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosDeviceSpace;
out vec4 FragPosLightSpace;

const float pi = 3.1415;

vec3 wave(vec3 pos, float time)
{
    vec2 coord = pos.xz * 0.02;
    const float height_variance = 0.8;
    float a = cnoise(coord + 0.05 * time);
    float b = cnoise(coord + 1.0);
    float c = cnoise(coord + a + 0.07 * time);
    float d = cnoise(coord + b + 0.09 * time);
    float e = cnoise(vec2(a + b, c + d));
    float height_mod = e;
    float height = pos.y + height_variance * (height_mod - 0.5);
    return vec3(pos.x, height, pos.z);
}

void main()
{
    // If max_height is updated in main, it needs to be updated here too.
    const float max_height = 128.0;
    const float water_level = 0.05 * max_height;
    vec3 pos;

    // If we are at the edge of the water, move the triangle down to height 0.0.
    // This simulateously blocks the sun and improves horizon quality.
    if (a_Position.y == 0.0)
    {
        pos = a_Position;
    }
    else
    {
        pos = wave(a_Position, Time);
    }


    // Recalculate normal.
    // The normal is the cross product of two vectors:
    // current vertex here -> .-->. <- right
    //                        |
    //                        |
    //                        v  
    //               below -> .
    vec3 below = wave(vec3(
            a_Position.x,
            a_Position.y,
            a_Position.z + VertDist),
        Time);
    vec3 right = wave(vec3(
            a_Position.x - VertDist,
            a_Position.y,
            a_Position.z),
        Time);
    vec3 above = wave(vec3(
            a_Position.x,
            a_Position.y,
            a_Position.z - VertDist),
        Time);
    vec3 left = wave(vec3(
            a_Position.x + VertDist,
            a_Position.y,
            a_Position.z),
        Time);

    vec3 downward = below - a_Position;
    vec3 rightward = right - a_Position;
    Normal = normalize(NormalMatrix * cross(rightward, downward));

    // Colour slightly by height.
    Colour = a_Colour;
    FragPos = vec3(ModelMatrix * vec4(pos, 1.0));
    // The fragment position for calculating shadows not perturbed by noise.
    // If it was the waves would cast shadows on the water. This looks good,
    // but obscures the sun specularity during sunset, which looks bad.
    FragPosLightSpace = LightSpaceMatrix * vec4(FragPos, 1.0);
    //FragPosLightSpace = LightSpaceMatrix * vec4(a_Position, 1.0);

    gl_Position =
        ProjectionMatrix
        * ViewMatrix
        * ModelMatrix
        //* vec4(a_Position, 1.0);
        * vec4(pos, 1.0);
    FragPosDeviceSpace = gl_Position;
}

