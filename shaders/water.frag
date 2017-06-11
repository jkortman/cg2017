#version 330
// Authorship: James Kortman (a1648090)

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
uniform float   Time;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

uniform vec3 ViewPos;

uniform sampler2D DepthMap;
uniform sampler2D ShadowDepthMap;
uniform sampler2D ReflectMap;

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
    const float N = 7.0;
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
    float ambi = light.ambient;

    // Calculate diffuse component.
    float diff = light.diffuse * max(dot(norm, light_dir), 0.0);

    // Calculate specular component.
    vec3 view_dir = normalize(ViewPos - FragPos);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = light.specular * pow(max(dot(norm, halfway_dir), 0.0), MtlShininess);

    return attenuation * vec3(ambi, diff, spec);
}

vec3 match_to_palette(vec3 colour) {
    float min_dist = 1.0 / 0.0; // infinity
    int min_index = 0;

    for (int i = 0; i < PaletteSize; i += 1) {
        float dist =
            ( pow(Palette[i].r - colour.r, 2)
            + pow(Palette[i].g - colour.g, 2)
            + pow(Palette[i].b - colour.b, 2));
        if (dist < min_dist) {
            min_dist = dist;
            min_index = i;
        }
    }

    return Palette[min_index];
}

// Fog calculation.
const float b = 0.003;

vec3 fog(in vec3 fragment, in float dist, in vec3 fog_colour)
{
    float f = 1.0 - exp(-dist * b);
    return mix(fragment, fog_colour, f);
}

// This light-scattering method is derived from
// http://www.iquilezles.org/www/articles/fog/fog.htm
vec3 fog_scatter(in vec3 fragment, in float dist, in vec3 fog_colour, in vec3 fog_colour_sun, in vec3 view_dir, in vec3 light_dir)
{
    float f = 1.0 - exp(-dist * b);
    float s = max(dot(view_dir, light_dir), 0.0 );
    float fog_scale = max(dot(light_dir, vec3(0.0, 1.0, 0.0)), 0.0);
    // The new fog colour should scale between the fragment colour and the
    // scattered fog colour using the angle of the sun.
    vec3 fog_colour_new = mix(fog_colour, fog_colour_sun, pow(s, 8.0));
    return mix(fragment, fog_colour_new, f);
}

float linearize(float z)
{
    const float near = 0.05;
    const float far = 1000.0;
    return (2.0 * near) / (far + near - z * (far - near));
}

float edge_detect()
{
    vec2 st = (0.5 + 0.5 * FragPosDeviceSpace.xyz / FragPosDeviceSpace.w).xy;
    float ds = 1.0 / 640.0; 
    float dt = 1.0 / 640.0; 
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

// Set to true to multisample the shadow map for smooth shadows.
#define MULTISAMPLE true
#define SAMPLE_RADIUS 2
float in_shadow(vec3 light_dir)
{
    // The cosine of the light angle.
    float light_angle = dot(light_dir, vec3(0.0, 1.0, 0.0));

    // The threshold at which shadows are on by default.
    // Between 0.0 and night_thresh, we interpolate between proper shadows
    // and full shadows.
    const float night_thresh = 0.1;
    // Day factor is 1.0 at day and 0.0 at night.
    float day_factor = clamp(light_angle / night_thresh, 0.0, 1.0);

    // perform perspective divide
    vec3 lit_coords = 0.5 + 0.5 * FragPosLightSpace.xyz / FragPosLightSpace.w;
    // Get depth of current fragment from light's perspective
    float frag_depth = lit_coords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.005;

    // If outside of light view and it's day, default to no shadows.
    if (light_angle > 0.0
        && (lit_coords.s > 1.0 || lit_coords.s < 0.0
            || lit_coords.t > 1.0 || lit_coords.t < 0.0)) return 1.0 - day_factor;

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
                // We use a correction factor to push the shadow amt towards
                // the maximum using the angle of the sun.
                const float e = 1.0;
                float horizon_correction = max(0.0, -light_angle);
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

    return max(shadow, 1.0 - day_factor);
}

vec4 world_reflection()
{
    vec4 norm = ProjectionMatrix * ViewMatrix
    * vec4(
        Normal * vec3(1.0, 0.0, 1.0)
        + vec3(0.0, -0.04, 0.0),         // height correction to remove blue gaps
        0.0);
    vec4 reflect_pos = FragPosDeviceSpace + 10.0*norm;
    vec3 coords = 0.5 + 0.5 * reflect_pos.xyz / reflect_pos.w;
    return texture(ReflectMap, coords.st);
}

// Get the caustic factor,
// which determines whether todraw swirly lines on the water.
// 1.0 = draw, 0.0 = don't draw. In-between is for antialiasing.
float caustic_factor()
{
    // Generate a 2D noise offset.
    vec2 coord = FragPos.xz * 0.04;
    float t = 0.01 * Time;
    float a = cnoise(coord + 0.05 * t);
    float b = cnoise(coord + 2.0 + 0.17 * t);
    float c = cnoise(vec2(a, b));
    float d = cnoise(vec2(b + 0.05 * c * t, -2.0));
    vec2 noise = vec2(a, b);
        
    float val = cellnoise(0.03 * FragPos.xz + 0.2 * noise, Time);
    float thresh = 0.03;
    return val <= thresh ? 1.0 : 0.0;
}

void main()
{
    // If max_height is updated in main, it needs to be updated here too.
    const float max_height = 128.0;
    const float water_level = 0.05 * max_height;

    // TODO: Remove duplicate code between this and lighting calculations
    // for calculating light_dir and view_dir.
    vec3 view_dir = normalize(ViewPos - FragPos);
    vec3 light_dir = normalize(-LightDay.position.xyz);
    float dist = length(ViewPos - FragPos);

    // Colour by reflection.
    float reflect_amt = 0.6;
    vec3 reflect_colour = vec3(world_reflection());
    vec3 base_colour = (1.0 - reflect_amt) * Colour
                     + reflect_amt * vec3(reflect_colour);
    
    // Get the shaded, lit colour.
    float shadow = in_shadow(light_dir);
    vec3 light_day_intensity = calculate_lighting(LightDay);
    vec3 light_point_intensity = vec3(0.0);

    for (int i = 0; i < NumLights; i += 1)
    {
        light_point_intensity += calculate_lighting(Lights[i]);
    }

    float ambi = max(light_day_intensity.x, light_point_intensity.x);
    float diff = max(
        (1.0 - shadow) * discretize(light_day_intensity.y),
        discretize(light_point_intensity.y));
    float spec = max(
        (1.0 - shadow) * discretize(light_day_intensity.z),
        discretize(light_point_intensity.z));

    // TODO: Replace these with material properties added by Water generation code.
    vec3 shaded_colour = 
        base_colour * (ambi + diff)
        + 0.02 * caustic_factor()
        + spec;

    // Colour very slightly by depth to give indication of water level.
    float colour_mod = 1.0 + 0.15 * (FragPos.y - water_level);
    shaded_colour = colour_mod * shaded_colour;

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
    
    if (fog_mode == fog_mode_normal)
    {
        shaded_colour = fog(shaded_colour, dist, fog_colour);
    }
    else if (fog_mode == fog_mode_scatter)
    {
        shaded_colour = fog_scatter(
            shaded_colour,
            dist,
            fog_colour, // regular fog colour
            fog_sun_colour, // colour when aligned with sun
            view_dir,
            -light_dir);
    }

    //float edge = edge_detect();
    //shaded_colour = edge * shaded_colour;

    FragColour = vec4(vec3(shaded_colour), 1.0);
    //FragColour = vec4(vec3(1.0 - in_shadow(light_dir)), 1.0);
    //FragColour = vec4(vec3(caustic_factor()), 1.0);
}


