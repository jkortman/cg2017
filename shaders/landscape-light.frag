#version 330

in vec3 FragPos;
in vec3 Colour;
in vec3 Normal;
in vec4 FragPosDeviceSpace;
in vec4 FragPosLightSpace;

out vec4 FragColour;

// Material settings
uniform vec3    MtlAmbient;     // Not used!
uniform vec3    MtlDiffuse;     // Not used!
uniform vec3    MtlSpecular;
uniform float   MtlShininess;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

uniform vec3 ViewPos;

uniform sampler2D DepthMap;
uniform sampler2D ShadowDepthMap;
uniform sampler2D SSAOMap;

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

// The positions of the samples to take for ambient occlusion.
// Each element in SSAOSamples is a 3D point relative to the fragment.
uniform vec3 SSAOSamples[64];
uniform int SSAONumSamples;


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
    const float N = 5.0;
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

float linearize(float z)
{
    const float near = 0.05;
    const float far = 1000.0;
    return (2.0 * near) / (far + near - z * (far - near));
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


float edge_detect()
{
    vec2 st = (0.5 + 0.5 * FragPosDeviceSpace.xyz / FragPosDeviceSpace.w).xy;
    float ds = 1.0 / 640.0; 
    float dt = 1.0 / 480.0; 
    float grad;

    mat3 Mx = mat3( 
         1.0,  2.0,  1.0, 
         0.0,  0.0,  0.0, 
        -1.0, -2.0, -1.0);
    mat3 My = mat3( 
        1.0,  0.0, -1.0, 
        2.0,  0.0, -2.0, 
        1.0,  0.0, -1.0);
    mat3 samples;

    /*
    const vec2 positions[] = vec2[](
        vec2(-ds,  dt),
        vec2(0.0,  dt),
        vec2( ds,  dt),
        vec2(-ds, 0.0),
        vec2(0.0, 0.0),
        vec2( ds, 0.0),
        vec2(-ds, -dt),
        vec2(0.0, -dt),
        vec2( ds, -dt));
    */
    for (int i = -1; i <= 1; i += 1)
    {
        for (int j = -1; j <= 1; j += 1)
        {
            samples[i+1][j+1] = linearize(
                texture(
                    DepthMap,
                    vec2(st.s - j * ds, st.t - i * dt))
                .x);
        }
    }

    float gx = dot(Mx[0], samples[0]) + dot(Mx[1], samples[1]) + dot(Mx[2], samples[2]); 
    float gy = dot(My[0], samples[0]) + dot(My[1], samples[1]) + dot(My[2], samples[2]);
    
    grad = abs(gx) + abs(gy);       // manhattan dist
    //grad = sqrt(gx*gx + gy*gy);     // geometric distance

    // Decrease weight factor for heavier lines.
    // At very low factors, the landscape vertices will become apparent.
    const float weight_factor = 0.4;
    float edge_value = 1.0 - pow(grad, weight_factor);
    return edge_value;
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

// Set to true to multisample the shadow map for smooth shadows.
#define MULTISAMPLE true
#define SAMPLE_RADIUS 2
float in_shadow(vec3 light_dir)
{
    // perform perspective divide
    vec3 lit_coords = 0.5 + 0.5 * FragPosLightSpace.xyz / FragPosLightSpace.w;
    // Get depth of current fragment from light's perspective
    float frag_depth = lit_coords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.005;

    float shadow = 0.0;
    if (MULTISAMPLE)
    {
        // The distance to sample neighbouring texels at.
        float dist = 0.5 / textureSize(ShadowDepthMap, 0).x;
        for (int i = -SAMPLE_RADIUS; i <= SAMPLE_RADIUS; i += 1)
        {
            for (int j = -SAMPLE_RADIUS; j <= SAMPLE_RADIUS; j += 1)
            {
                // Get the depth of the texel neightbour i,j
                vec2 neighbour_coords = vec2(lit_coords.x + i * dist, lit_coords.y + j * dist);
                float neighbour_depth = texture(ShadowDepthMap, neighbour_coords).r; 
                //shadow += (frag_depth - bias) > neighbour_depth  ? (1.0/9.0) : 0.0;
                // interpolate from 0 to 1/9 based on how large the difference is.
                
                // This has errors when the sun is below the horizon,
                // so we increase shadow amount when that is the case.
                // We use a correction factor for to push the shadow amt towards
                // the maximum using the angle of the sun.
                const float e = 1.0;
                float horizon_correction = max(0.0, -dot(light_dir, vec3(0.0, 1.0, 0.0)));
                shadow += max(0.0, 
                    min(1.0/9.0,
                        horizon_correction +
                        e * (frag_depth - bias - neighbour_depth)));
            }
        }
    }
    else
    {
        // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float lit_depth = texture(ShadowDepthMap, lit_coords.xy).r; 
        shadow = (frag_depth - bias) > lit_depth  ? 1.0 : 0.0;
    }
    return shadow;
}

void main()
{
    // --------------------------------
    // -- Fragment colour processing --
    // --------------------------------
    vec3 colour = match_to_palette(Colour);

    // Experimental seasons stuff.
    // Each season has a multiplicative and additive colour.
    // The season values are interpolated between based on the time.
    #if 0
        vec3 mul;
        vec3 add;
        season_colours(mul, add);
        colour = colour * mul + add;
    #endif

    vec3 light_day_intensity = calculate_lighting(LightDay);
    vec3 light_point_intensity = vec3(0.0);

    for (int i = 0; i < NumLights; i += 1)
    {
        light_point_intensity += calculate_lighting(Lights[i]);
    }

    float dist = length(ViewPos - FragPos);
    // TODO: Remove duplicate code between this and lighting calculations
    // for calculating light_dir and view_dir.
    vec3 view_dir = normalize(ViewPos - FragPos);
    vec3 light_dir = normalize(-LightDay.position.xyz);

    float shadow = in_shadow(light_dir);

    float ambi = max(light_day_intensity.x, light_point_intensity.x);
    float diff = max(
        (1.0 - shadow) * discretize(light_day_intensity.y),
        discretize(light_point_intensity.y));
    float spec = max(
        (1.0 - shadow) * discretize(light_day_intensity.z),
        discretize(light_point_intensity.z));

    // Lighting application
    // TODO: Replace these with material properties added by TerrainGenerator
    #if 1
    colour = colour * (0.15 * ambi + 0.8 * diff + 0.3 * spec);
    #endif

    // Determine fog colours by time of day.
    vec3 fog_colour_day = vec3(0.5, 0.6, 0.7);
    vec3 fog_colour_sun = vec3(1.0, 0.9, 0.6);
    vec3 fog_colour_night = vec3(0.08, 0.08, 0.2);
    float day_factor = 0.5 + 0.5 * dot(light_dir, vec3(0.0, 1.0, 0.0));
    vec3 fog_colour = mix(fog_colour_night, fog_colour_day, day_factor);
    vec3 fog_sun_colour = mix(fog_colour_night, fog_colour_sun, day_factor);

    const int fog_mode_none = 0,
              fog_mode_normal = 1,
              fog_mode_scatter = 2;
    const int fog_mode = fog_mode_scatter;
    // Perform fog colour modification
    #if 1
    if (fog_mode == fog_mode_normal)
    {
        colour = fog(colour, dist, fog_colour);
    }
    else if (fog_mode == fog_mode_scatter)
    {
        colour = fog_scatter(
            colour,
            dist,
            fog_colour, // regular fog colour
            fog_sun_colour, // colour when aligned with sun
            view_dir,
            -light_dir);
    }
    #endif

    // Sobel edge highlighting
    #if 0
    colour = edge_detect() * colour;
    #endif

    // SSAO
    #if 1
    vec3 coords = 0.5 + 0.5 * FragPosDeviceSpace.xyz / FragPosDeviceSpace.w;
    vec2 offset = vec2(0.003);
    float occlusion = 0.0;
    const int radius = 4;
    for (int i = -radius; i <= radius; i += 1)
    {
        for (int j = -radius; j <= radius; j += 1)
        {
            
            occlusion += texture(SSAOMap, coords.xy + offset * vec2(i,j)).r;

        }
    }
    colour =
        (occlusion / float(pow(2 * radius + 1, 2)))
        * colour;
    #endif

    FragColour = vec4(colour, 1.0);

    // Depth buffer testing
    #if 0
    vec2 st;
    vec3 coords;
    float depth;

    coords = 0.5 + 0.5 * FragPosDeviceSpace.xyz / FragPosDeviceSpace.w;
    //coords = 0.5 + 0.5 * FragPosLightSpace.xyz / FragPosLightSpace.w;
    st = coords.xy;
    depth = texture(DepthMap, st).a;
    //depth = texture(ShadowDepthMap, st).x;
    //depth = linearize(depth);
    FragColour = vec4(vec3(depth), 1.0);
    //FragColour = texture(TopDownMap, st);
    //FragColour = vec4(vec3(edge), 1.0);
    #endif
}

