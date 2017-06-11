#version 330

in vec2 TexCoord;

out vec4 FragColour;

uniform sampler2D SceneMap;
uniform sampler2D DepthMap;

void main()
{
    // Output only regions above a certain brightness in each channel.
    vec3 colour = texture(SceneMap, TexCoord).rgb;
    float brightness = dot(colour, vec3(0.5, 0.5, 0.4));
    float depth_check = 0.9;
    vec3 min_colour = vec3(0.7, 0.6, 0.8);
    if (colour.r < min_colour.r
        || colour.g < min_colour.g
        || colour.b < min_colour.b
        || texture(DepthMap, TexCoord).x < depth_check)
    {
        FragColour = vec4(vec3(0.0), 1.0);
    }
    else
    {
        FragColour = vec4(colour, 1.0);
    }
}