#version 330

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Colour;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;
uniform float Time;
uniform float VertDist;

out vec3 Colour;
out vec3 Normal;
out vec3 FragPos;

const float pi = 3.1415;

vec3 wave(vec3 pos, float time)
{
    // Wave component 1: Constant per time frame to simulate slow tides.
    // Low frequency
    float wave1 = sin(Time / (5.0 * 2.0 * pi));   // 5 seconds per cycle

    // Wave component 2: A high-freq sine wave that moves
    // inward then outward radially.
    float radius = pow(pos.x, 2) + pow(pos.y, 2);
    float wave2 = sin((radius + Time) / (2.0 * pi));

    float height = pos.y + wave1;
    return vec3(pos.x, height, pos.z);
}

void main()
{
    // If max_height is updated in main, it needs to be updated here too.
    const float max_height = 128.0;
    const float water_level = 0.05 * max_height;
    const float water_variance = 0.01 * max_height;
    FragPos = vec3(ModelMatrix * vec4(a_Position, 1.0));
    Colour = a_Colour;

    vec3 pos = wave(a_Position, Time);

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
    vec3 downward = below - a_Position;
    vec3 rightward = right - a_Position;
    Normal = cross(rightward, downward);

    gl_Position =
        ProjectionMatrix
        * ViewMatrix
        * ModelMatrix
        * vec4(pos, 1.0);
}
