#version 330

in vec3 FragPos;
in vec3 Colour;
in vec3 Normal;
in float crest;
in vec4 FragPosLightSpace;

out vec4 FragColour;

// Material settings
uniform vec3    MtlAmbient;     // Not used!
uniform vec3    MtlDiffuse;     // Not used!
uniform vec3    MtlSpecular;
uniform float   MtlShininess;
uniform float Time;

uniform vec3 ViewPos;

uniform sampler2D DepthMap;

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

vec4 calculate_lighting(in LightSource light) {
    vec3 norm = normalize(Normal);

    vec3 light_dir;
    if (light.position.w == 0.0) {
        light_dir = normalize(-light.position.xyz);
    } else {
        light_dir = normalize(vec3(light.position) - FragPos);
    }

    // Calculate ambient component.
    vec3 ambient = Colour * light.ambient;

    // Calculate diffuse component.
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * Colour;// * light.diffuse;

    // Calculate specular component.
    vec3 view_dir = normalize(ViewPos - FragPos);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(norm, halfway_dir), 0.0), 100.0);
    vec3 specular = vec3(light.specular * spec);

    return vec4(ambient + diffuse + specular, 1.0);
}

vec4 match_to_palette(vec4 colour) {
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

    return vec4(Palette[min_index], colour.a);
}

// Fog calculation.
const float b = 0.003;

vec3 fog(in vec3 fragment, in float dist, in vec3 fog_colour)
{
    float f = 1.0 - exp(-dist * b);
    return mix(fragment, fog_colour, f);
}

// http://www.iquilezles.org/www/articles/fog/fog.htm
vec3 fog_scatter(in vec3 fragment, in float dist, in vec3 fog_colour, in vec3 fog_colour_sun, in vec3 view_dir, in vec3 light_dir)
{
    float f = 1.0 - exp(-dist * b);
    float s = max(dot( view_dir, light_dir ), 0.0 );
    float fog_scale = max(dot(light_dir, vec3(0.0, 1.0, 0.0)), 0.0);
    // The new fog colour should scale between the fragment colour and the
    // scattered fog colour using the angle of the sun.
    vec3 fog_colour_new = mix(fog_colour, fog_colour_sun, pow(s, 8.0));
    return mix(fragment, fog_colour_new, f);
}

// Set to true to multisample the shadow map for smooth shadows.
#define MULTISAMPLE true
#define SAMPLE_RADIUS 2
float in_shadow()
{
    // perform perspective divide
    vec3 lit_coords = 0.5 + 0.5 * FragPosLightSpace.xyz / FragPosLightSpace.w;
    // Get depth of current fragment from light's perspective
    float frag_depth = lit_coords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.002;

    float shadow = 0.0;
    if (MULTISAMPLE)
    {
        // The distance to sample neighbouring texels at.
        //vec2 dist = 1.0 / textureSize(DepthMap, 0);
        float dist = 1.0/2048.0;
        for (int i = -SAMPLE_RADIUS; i <= SAMPLE_RADIUS; i += 1)
        {
            for (int j = -SAMPLE_RADIUS; j <= SAMPLE_RADIUS; j += 1)
            {
                // Get the depth of the texel neightbour i,j
                vec2 neighbour_coords = vec2(lit_coords.x + i * dist, lit_coords.y + j * dist);
                float neighbour_depth = texture(DepthMap, neighbour_coords).r; 
                shadow += (frag_depth - bias) > neighbour_depth  ? (1.0/9.0) : 0.0;
            }
        }
    }
    else
    {
        // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float lit_depth = texture(DepthMap, lit_coords.xy).r; 
        shadow = (frag_depth - bias) > lit_depth  ? 1.0 : 0.0;
    }
    return shadow;
}

void main()
{
    // If max_height is updated in main, it needs to be updated here too.
    const float max_height = 128.0;
    const float water_level = 0.05 * max_height;

    // Colour very slightly by depth to give indication of water level.
    float shadow = in_shadow();
    float colour_mod = 1.0 + 0.30 * (FragPos.y - water_level);
    vec4 shaded_colour = colour_mod * match_to_palette(calculate_lighting(LightDay));

    // TODO: Remove duplicate code between this and lighting calculations
    // for calculating light_dir and view_dir.
    vec3 view_dir = normalize(ViewPos - FragPos);
    vec3 light_dir = normalize(-LightDay.position.xyz);
    float dist = length(ViewPos - FragPos);

    // Determine fog colours by time of day.
    vec3 fog_colour_day = vec3(0.5, 0.6, 0.7);
    vec3 fog_colour_sun = vec3(1.0, 0.9, 0.7);
    vec3 fog_colour_night = vec3(0.2, 0.3, 0.4);
    float day_factor = 0.5 + 0.5 * dot(light_dir, vec3(0.0, 1.0, 0.0));
    vec3 fog_colour = mix(fog_colour_night, fog_colour_day, day_factor);
    vec3 fog_sun_colour = mix(fog_colour_night, fog_colour_sun, day_factor);

    const int fog_mode_none = 0,
              fog_mode_normal = 1,
              fog_mode_scatter = 2;
    const int fog_mode = fog_mode_scatter;
    if (fog_mode == fog_mode_none)
    {
        FragColour = shaded_colour;
    }
    if (fog_mode == fog_mode_normal)
    {
        FragColour = vec4(
            fog(shaded_colour.xyz, dist, fog_colour),
            1.0);
    }
    else if (fog_mode == fog_mode_scatter)
    {
        FragColour = vec4(
            fog_scatter(
                shaded_colour.xyz,
                dist,
                fog_colour, // regular fog colour
                fog_sun_colour, // colour when aligned with sun
                view_dir,
                -light_dir),
            1.0);
    }
    //FragColour = vec4(vec3(1.0-shadow), 1.0);
}