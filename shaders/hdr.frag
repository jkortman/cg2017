#version 330

in vec2 TexCoord;

out vec4 FragColour;

uniform sampler2D SceneMap;
uniform sampler2D BloomMap;

void main()
{
    #if 0
    vec3 colour = texture(SceneMap, TexCoord).rgb;
    #else
    vec3 colour = texture(SceneMap, TexCoord).rgb
                + texture(BloomMap, TexCoord).rgb;

    float brightness = dot(colour, vec3(0.2126, 0.7152, 0.0722));

    // Tone mapping
    colour = colour / (colour + vec3(1.0));

    // Resaturate the image.
    vec3 hsv = rgb2hsv(colour);
    hsv.y = pow(hsv.y, 0.4);
    hsv.z = pow(hsv.z, 0.8);
    colour = hsv2rgb(hsv);

    // Apply gamma correction.
    float gamma_factor = 1.0 / 1.3;
    colour = pow(colour, vec3(gamma_factor));

    #endif

    FragColour = vec4(colour, 1.0);
}