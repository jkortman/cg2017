#version 330

in vec2 TexCoord;

out vec4 FragColour;

uniform sampler2D SceneMap;
uniform sampler2D BloomMap;

void main()
{
    #if 1
    vec3 colour = texture(BloomMap, TexCoord).rgb;
    #else
    vec3 colour = 1.0 * texture(SceneMap, TexCoord).rgb
                + 1.8 * texture(BloomMap, TexCoord).rgb;
    #endif

    // Tone mapping
    #if 0
    colour = colour / (colour + vec3(1.0));
    #endif

    // Resaturate the image.
    #if 0
    vec3 hsv = rgb2hsv(colour);
    hsv.y = pow(hsv.y, 0.6);
    hsv.z = pow(hsv.z, 0.8);
    colour = hsv2rgb(hsv);
    #endif

    // Apply gamma correction.
    #if 0
    float gamma_factor = 1.0 / 1.3;
    colour = pow(colour, vec3(gamma_factor));
    #endif

    FragColour = vec4(colour, 1.0);
}