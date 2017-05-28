// Simple shader for rendering a depth map onto a quad.

#version 330

in vec2 TexCoords;

out vec4 Colour;

uniform sampler2D DepthMap;

float linearize(float z)
{
    const float near = 0.05;
    const float far = 1200.0;
    return (2.0 * near) / (far + near - z * (far - near));
}

void main()
{             
    float depth = texture(DepthMap, TexCoords).r;
    Colour = vec4(vec3(depth), 1.0);
    //Colour = vec4(TexCoords, 0.0, 1.0);
}  