#version 330
// Authorship: James Kortman (a1648090)

in vec2 TexCoord;

out vec4 FragColour;

uniform sampler2D SceneMap;
uniform sampler2D BloomMap;

uniform vec3 ViewDir;
uniform vec3 LightDayDir;

void main()
{
    #if 1
    vec3 colour = texture(SceneMap, TexCoord).rgb;
    #else
    vec3 colour = 1.0 * texture(SceneMap, TexCoord).rgb
                + 2.0 * texture(BloomMap, TexCoord).rgb;
    #endif

    // Tone mapping
    #if 0

    colour = colour / (colour + vec3(1.0));
    #endif

    #if 0
    float s = max(dot( ViewDir, LightDayDir ), 0.0 );
    float day_factor = 0.5 + 0.5 * dot(LightDayDir, vec3(0.0, 1.0, 0.0));
    float exposure = 3.0 +  2.0 * s * day_factor;
    colour = vec3(1.0) - exp(-colour*exposure);
    #endif

    // Resaturate the image.
    #if 1
    vec3 hsv = rgb2hsv(colour);
    hsv.y = pow(hsv.y, 0.8);
    hsv.z = pow(hsv.z, 0.9);
    colour = hsv2rgb(hsv);
    #endif

    // Apply gamma correction.
    #if 0
    float gamma_factor = 1.0 / 2.2;
    colour = pow(colour, vec3(gamma_factor));
    #endif

    FragColour = vec4(colour, 1.0);
}