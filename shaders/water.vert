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
out vec4 FragPosLightSpace;
out float crest;

const float pi = 3.1415;

float cnoise(in vec2 P);

vec3 wave(vec3 pos, float time)
{
    vec2 coord = pos.xz * 0.02;
    const float height_variance = 0.6;
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
    Normal = NormalMatrix * cross(rightward, downward);

    // Cresting: If this point is higher than it's neighbours, it is a crest.
    if (a_Position.y > below.y
        && a_Position.y > above.y
        && a_Position.y > right.y
        && a_Position.y > left.y)
    {
        //crest = 1.0;
        crest = max(
            max(a_Position.y - below.y, a_Position.y - above.y),
            max(a_Position.y - right.y, a_Position.y - left.y));
    } else
    {
        crest = 0.0;
    }

    // Colour slightly by height.
    Colour = a_Colour;
    FragPos = vec3(ModelMatrix * vec4(pos, 1.0));
    FragPosLightSpace = LightSpaceMatrix * vec4(FragPos, 1.0);

    gl_Position =
        ProjectionMatrix
        * ViewMatrix
        * ModelMatrix
        * vec4(pos, 1.0);
}

// GLSL textureless classic 2D noise "cnoise",
// with an RSL-style periodic variant "pnoise".
// Author:  Stefan Gustavson (stefan.gustavson@liu.se)
// Version: 2011-08-22
//
// Many thanks to Ian McEwan of Ashima Arts for the
// ideas for permutation and gradient selection.
//
// Copyright (c) 2011 Stefan Gustavson. All rights reserved.
// Distributed under the MIT license. See LICENSE file.
// https://github.com/ashima/webgl-noise
//

vec4 mod289(vec4 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
    return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

vec2 fade(vec2 t) {
    return t*t*t*(t*(t*6.0-15.0)+10.0);
}

// Classic Perlin noise
float cnoise(vec2 P)
{
    vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    Pi = mod289(Pi); // To avoid truncation effects in permutation
    vec4 ix = Pi.xzxz;
    vec4 iy = Pi.yyww;
    vec4 fx = Pf.xzxz;
    vec4 fy = Pf.yyww;
    
    vec4 i = permute(permute(ix) + iy);
    
    vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
    vec4 gy = abs(gx) - 0.5 ;
    vec4 tx = floor(gx + 0.5);
    gx = gx - tx;
    
    vec2 g00 = vec2(gx.x,gy.x);
    vec2 g10 = vec2(gx.y,gy.y);
    vec2 g01 = vec2(gx.z,gy.z);
    vec2 g11 = vec2(gx.w,gy.w);
    
    vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
    g00 *= norm.x;
    g01 *= norm.y;
    g10 *= norm.z;
    g11 *= norm.w;
    
    float n00 = dot(g00, vec2(fx.x, fy.x));
    float n10 = dot(g10, vec2(fx.y, fy.y));
    float n01 = dot(g01, vec2(fx.z, fy.z));
    float n11 = dot(g11, vec2(fx.w, fy.w));
    
    vec2 fade_xy = fade(Pf.xy);
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
    return 2.3 * n_xy;
}

// Classic Perlin noise, periodic variant
float pnoise(vec2 P, vec2 rep)
{
    vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    Pi = mod(Pi, rep.xyxy); // To create noise with explicit period
    Pi = mod289(Pi);        // To avoid truncation effects in permutation
    vec4 ix = Pi.xzxz;
    vec4 iy = Pi.yyww;
    vec4 fx = Pf.xzxz;
    vec4 fy = Pf.yyww;
    
    vec4 i = permute(permute(ix) + iy);
    
    vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
    vec4 gy = abs(gx) - 0.5 ;
    vec4 tx = floor(gx + 0.5);
    gx = gx - tx;
    
    vec2 g00 = vec2(gx.x,gy.x);
    vec2 g10 = vec2(gx.y,gy.y);
    vec2 g01 = vec2(gx.z,gy.z);
    vec2 g11 = vec2(gx.w,gy.w);
    
    vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
    g00 *= norm.x;
    g01 *= norm.y;
    g10 *= norm.z;
    g11 *= norm.w;
    
    float n00 = dot(g00, vec2(fx.x, fy.x));
    float n10 = dot(g10, vec2(fx.y, fy.y));
    float n01 = dot(g01, vec2(fx.z, fy.z));
    float n11 = dot(g11, vec2(fx.w, fy.w));
    
    vec2 fade_xy = fade(Pf.xy);
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
    return 2.3 * n_xy;
}