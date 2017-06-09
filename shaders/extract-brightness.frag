#version 330

in vec2 TexCoord;

out vec4 FragColour;

uniform sampler2D SceneMap;

void main()
{
    // Output only regions above a certain brightness in each channel.
    vec3 colour = texture(SceneMap, TexCoord).rgb;
    float brightness = dot(colour, vec3(0.2126, 0.7152, 0.0722));
    if(brightness < 0.75) discard;
    FragColour = vec4(colour, 1.0);
}