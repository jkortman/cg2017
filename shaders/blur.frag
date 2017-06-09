#version 330

in vec2 TexCoord;

out vec4 FragColour;

uniform sampler2D BloomMap;
uniform int BlurDirection;

// gaussian blur kernel
const float kernel[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);


void main()
{
    const float dist = 0.003;
    vec2 direction;
    if (BlurDirection == 0) direction = vec2(1.0, 0.0);
    else                    direction = vec2(0.0, 1.0);

    vec3 colour = vec3(0.0);
    const int radius = 1;
    for (int i = -2; i <= 2; i += 1)
    {
        colour += kernel[i + 2] * texture(BloomMap, TexCoord + direction * i * dist).rgb;
    }
    FragColour = vec4(colour, 1.0);
}