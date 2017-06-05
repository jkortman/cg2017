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

vec4 calculate_lighting(in LightSource light, in vec3 colour) {
    vec3 norm = normalize(Normal);

    vec3 light_dir;
    if (light.position.w == 0.0) {
        light_dir = normalize(-light.position.xyz);
    } else {
        light_dir = normalize(vec3(light.position) - FragPos);
    }

    // Calculate ambient component.
    vec3 ambient = colour * light.ambient;

    // Calculate diffuse component.
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * colour;// * light.diffuse;

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
    const float night_thresh = -0.2;
    if (light_angle < night_thresh) return 1.0;

    // perform perspective divide
    vec3 lit_coords = 0.5 + 0.5 * FragPosLightSpace.xyz / FragPosLightSpace.w;
    // Get depth of current fragment from light's perspective
    float frag_depth = lit_coords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.005;

    // If outside of light view and it's day, default to no shadows.
    if (light_angle > 0.0
        && (lit_coords.s > 1.0 || lit_coords.s < 0.0
            || lit_coords.t > 1.0 || lit_coords.t < 0.0)) return 0.0;

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

    if (light_angle < 0.0)
    {
        float x = light_angle / night_thresh;
        shadow = x + (1.0 - x) * shadow;
    }
    return shadow;
}

// Get the world reflection colour.
// If alpha channel is zero, the landscape has no reflection
// at the current viewing angle.
vec4 world_reflection_bad(vec3 view_dir)
{
    //           --------- landscape map face
    //         ^      ^
    //       h |      |
    //         v      | /
    // view dir -----> / water surface
    //                /
    // h is the effective height of the landscape map.
    const float h = 10.0;

    // Flat surfaces reflect nothing.
    if (Normal == vec3(0.0, 1.0, 0.0)) return vec4(0.0);

    // Reflect the view direction on the water surface.
    vec3 r = reflect(view_dir, Normal);

    // Project the vector r onto the landscape surface.
    float x = h / r.y;
    vec3 p = FragPos + x * r;

    // Map p.xz to [0, 1] from [-200, 200]
    vec2 st = 0.5 + 0.5 * p.xz / 200.0;
    st = vec2(clamp(st.s, 0.0, 1.0),
              clamp(st.t, 0.0, 1.0));

    vec4 col = texture(ReflectMap, st);

    // blur the colour.
    float dist = 1.0 / 1024.0;
    col = (
        texture(ReflectMap, st + vec2(-dist, -dist)),
        texture(ReflectMap, st + vec2(  0.0, -dist)),
        texture(ReflectMap, st + vec2( dist, -dist)),
        texture(ReflectMap, st + vec2(-dist,   0.0)),
        texture(ReflectMap, st + vec2(  0.0,   0.0)),
        texture(ReflectMap, st + vec2( dist,   0.0)),
        texture(ReflectMap, st + vec2(-dist,  dist)),
        texture(ReflectMap, st + vec2(  0.0,  dist)),
        texture(ReflectMap, st + vec2( dist,  dist))) / 9.0;

    if (col.xyz == vec3(0.0))
    {
        return vec4(0.0);
    }

    return col;
}


vec4 world_reflection()
{
    vec3 coords = 0.5 + 0.5 * FragPosDeviceSpace.xyz / FragPosDeviceSpace.w;
    return texture(ReflectMap, coords.st);
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
    const vec3 water_colour = vec3(0.30, 0.30, 1.00);
    float reflect_amt = 0.7;
    vec3 reflect_colour = vec3(world_reflection());
    vec3 base_colour = (1.0 - reflect_amt) * water_colour
                     + reflect_amt * vec3(reflect_colour);

    // Colour very slightly by depth to give indication of water level.
    float shadow = in_shadow(light_dir);
    float colour_mod = 1.0 + 0.30 * (FragPos.y - water_level);
    float shadow_amt = 0.85;
    vec3 shaded_colour = vec3(
        (1.0 - shadow)
        * colour_mod
        * ((1.0 - reflect_amt)*match_to_palette(calculate_lighting(LightDay, water_colour))
           + reflect_amt*calculate_lighting(LightDay, reflect_colour)));

    // Determine fog colours by time of day.
    vec3 fog_colour_day = vec3(0.5, 0.6, 0.7);
    vec3 fog_colour_sun = vec3(1.0, 0.9, 0.7);
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

}