#version 330

in vec2 TexCoord;

out vec4 FragColour;

uniform sampler2D SceneMap;
uniform sampler2D BloomMap;

void main()
{
    #if 1
    vec3 colour = texture(SceneMap, TexCoord).rgb;
    #else
    vec3 colour = texture(SceneMap, TexCoord).rgb
                + texture(BloomMap, TexCoord).rgb;

    float brightness = dot(colour, vec3(0.2126, 0.7152, 0.0722));

    // Tone mapping
    colour = colour / (colour + 1.0);

    // Apply gamma correction.
    float gamma_factor = 1.0 / 1.2;
    colour = pow(colour, vec3(gamma_factor));

    // Resaturate the image.
    vec3 x = vec3(1.0, 1.4, 1.4);
    vec3 hsv = rgb2hsv(colour);
    hsv = min(vec3(1.0), x*hsv);
    colour = hsv2rgb(hsv);
    #endif

    FragColour = vec4(colour, 1.0);
}