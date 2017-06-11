#version 330
// Authorship: James Kortman (a1648090)

in vec2 TexCoord;

out vec4 FragColour;

uniform sampler2D BloomMap;
uniform int BlurDirection;

// gaussian blur kernel
const int kernel_size = 15;
const float kernel[] = float[](
    0.0, 0.0, 0.000003, 0.000229, 0.005977,
    0.060598, 0.24173, 0.382925, 0.24173, 0.060598,
    0.005977, 0.000229, 0.000003, 0.0, 0.0);


void main()
{
    float dist = 0.002;
    vec2 direction;
    if (BlurDirection == 0) direction = vec2(1.0, 0.0);
    else                    direction = vec2(0.0, 1.0);

    vec3 colour = vec3(0.0);
    for (int i = 0; i <= kernel_size; i += 1)
    {
        colour += kernel[i] * texture(
            BloomMap,
            TexCoord + direction * (i - kernel_size/2) * dist).rgb;
    }
    FragColour = vec4(colour, 1.0);
}