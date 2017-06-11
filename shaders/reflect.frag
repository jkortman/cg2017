#version 330
// Authorship: James Kortman (a1648090)

in vec3 FragPos;
in vec3 Colour;
in vec3 Normal;

layout (location = 0) out vec4 FragColour;

// Material settings
uniform vec3    MtlAmbient;     // Not used!
uniform vec3    MtlDiffuse;     // Not used!
uniform vec3    MtlSpecular;
uniform float   MtlShininess;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

uniform vec3 ViewPos;

uniform float Time;

struct LightSource
{
    vec4    position;
    float   ambient;
    float   diffuse;
    float   specular;
    float   K_constant;
    float   K_linear;
    float   K_quadratic;
    vec3    spot_direction;
    float   spot_cos_angle;
};
uniform LightSource LightDay;
uniform int NumLights;
uniform LightSource Lights[4];

uniform vec3 Palette[16];
uniform int PaletteSize;

float discretize(float value)
{
    // We map a continuous value to one of N
    // fixed values between 0 and 1+W, where W is some factor to force white
    // highlights on the landscape.
    // For N = 5 this is equivalent to:
    //      if      (value < 0.25)      return 0.00;
    //      else if (value < 0.50)      return 0.33;
    //      else if (value < 0.75)      return 0.66;
    //      else if (value < 1.00)      return 1.00;
    //      else                        return 1.33;
    /*
    if      (value < 0.25)      return 0.00;
    else if (value < 0.50)      return 0.33;
    else if (value < 0.75)      return 0.66;
    else if (value < 1.00)      return 1.00;
    else                        return 1.33;
    */
    const float N = 3.0;
    return floor(N * value) / (N - 1.0);
}

vec3 calculate_lighting(in LightSource light) {
    vec3 norm = normalize(Normal);

    vec3 light_dir;
    float attenuation;
    if (light.position.w == 0.0) {
        // Directional light
        light_dir = normalize(-light.position.xyz);
        attenuation = 1.0;
    } else {
        // Point or spot light
        light_dir = normalize(vec3(light.position) - FragPos);
        float dist = length(vec3(light.position) - FragPos);
        attenuation = 1.0 / (  light.K_constant
                             + light.K_linear * dist
                             + light.K_quadratic * dist * dist);
        // If a spot light, attenuate by the error.
        float cos_angle = dot(light_dir, normalize(-light.spot_direction));
        if (cos_angle < light.spot_cos_angle)
        {
            float amt = light.spot_cos_angle - cos_angle;
            attenuation -= amt;
        }
    }

    // Calculate ambient component.
    float ambi = 1.0;

    // Calculate diffuse component.
    float diff = max(dot(norm, light_dir), 0.0);

    // Calculate specular component.
    vec3 view_dir = normalize(ViewPos - FragPos);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(norm, halfway_dir), 0.0), MtlShininess);

    return attenuation * vec3(ambi, diff, spec);
}

vec3 match_to_palette(vec3 colour)
{
    float min_dist = 1.0 / 0.0; // infinity
    int min_index = 0;

    for (int i = 0; i < PaletteSize; i += 1) {
        float dist =
            (pow(Palette[i].r - colour.r, 2)
            + pow(Palette[i].g - colour.g, 2)
            + pow(Palette[i].b - colour.b, 2));
        if (dist < min_dist) {
            min_dist = dist;
            min_index = i;
        }
    }

    return Palette[min_index];
}

// Returns a vector of weights for each season.
const float year = 50.0;
void season_colours(out vec3 mul, out vec3 add)
{
    // Each season has an additive and a multiplicative colour.
    vec3 seasons_mul[4] = vec3[](
        vec3( 1.000,  0.900,  0.500),
        1.4 * vec3( 0.500,  0.370,  0.040),
        vec3( 0.800,  0.800,  1.000),
        vec3( 0.700,  1.000,  0.800)
    );
    vec3 seasons_add[4] = vec3[](
        vec3( 0.300,  0.300,  0.300),
        vec3( 0.100,  0.080,  0.029),
        vec3(-0.100, -0.100,  0.000),
        vec3( 0.000,  0.100,  0.000)
    );

    // Get the normalized time, t, which is 0 at the start of the year and
    // 2pi at the end.
    float t = 2.0 * 3.141 * mod(Time, year) / year;

    // calculate the weights for each season.
    vec4 weights = vec4(
        max(0.0,  sin(t)),
        max(0.0,  cos(t)),
        max(0.0, -sin(t)),
        max(0.0, -cos(t)));

    mul = vec3(0.0);
    add = vec3(0.0);
    for (int i = 0; i < 4; i += 1)
    {
        mul += weights[i] * seasons_mul[i];
        add += weights[i] * seasons_add[i];
    }
}

void main()
{
    float water_level = 0.05*128.0;
    if (FragPos.y > water_level) discard;

    // --------------------------------
    // -- Fragment colour processing --
    // --------------------------------
    vec3 colour = match_to_palette(Colour);

    // Experimental seasons stuff.
    // Each season has a multiplicative and additive colour.
    // The season values are interpolated between based on the time.
    #if 0
        //vec3 mseason_summer = vec3( 1.0,  0.6,  0.5);
        //vec3 aseason_summer = vec3( 0.1,  0.2,  0.0);
        vec3 mul;
        vec3 add;
        season_colours(mul, add);
        colour = colour * mul + add;
    #endif

    FragColour = vec4(colour, 1.0);
}
