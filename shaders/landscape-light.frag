#version 330

in vec3 FragPos;
in vec3 Colour;
in vec3 Normal;
in vec4 FragPosLightSpace;

out vec4 FragColour;

// Material settings
uniform vec3    MtlAmbient;     // Not used!
uniform vec3    MtlDiffuse;     // Not used!
uniform vec3    MtlSpecular;
uniform float   MtlShininess;

uniform vec3 ViewPos;

uniform sampler2D DepthMap;

struct LightSource
{
    vec4    position;
    vec3    ambient;
    vec3    diffuse;
    vec3    specular;
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
    const int N = 5;
    if      (value < 0.25)      return 0.00;
    else if (value < 0.50)      return 0.33;
    else if (value < 0.75)      return 0.66;
    else if (value < 1.00)      return 1.00;
    else                        return 1.33;
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

float edge_detection()
{
    // --------------------
    // -- Edge detection -- 
    // --------------------
    const int no_edge = 0, simple = 1, sobel = 2;
    int edge_method = no_edge;

    vec2 st = 0.5 * vec2(
        float(gl_FragCoord.x) / 640.0,
        float(gl_FragCoord.y) / 480.0); 
    const float d_s = 0.75 / 640.0;
    const float d_t = 0.75 / 480.0;
    float grad;

    if (edge_method == no_edge)
    {
        grad = 1.0;
    }
    else if (edge_method == simple)
    {
        float above = linearize(texture(DepthMap, vec2(st.s, st.t - d_t)).x); 
        float left  = linearize(texture(DepthMap, vec2(st.s - d_s, st.t)).x); 
        float depth = linearize(texture(DepthMap, st).x);
        const float line_dark = 10.0;
        grad = 1.0 - line_dark * (2.0 * depth - above - left);
    }
    else if (edge_method == sobel)
    {
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
                        vec2(st.s - j * d_s, st.t - i * d_t))
                    .x);
            }
        }

        float gx = dot(Mx[0], samples[0]) + dot(Mx[1], samples[1]) + dot(Mx[2], samples[2]); 
        float gy = dot(My[0], samples[0]) + dot(My[1], samples[1]) + dot(My[2], samples[2]);
        
        const float line_dark = 5.0;
        grad = 1.0 - line_dark * (abs(gx) + abs(gy));             // manhattan dist
        //grad = 1.0 - line_dark * sqrt(gx*gx + gy*gy); // geometric distance
    }

    return grad;
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
    // --------------------------------
    // -- Fragment colour processing --
    // --------------------------------
    vec3 colour = match_to_palette(Colour);
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

    // TODO: Replace these with material properties added by TerrainGenerator
    float shadow = in_shadow();

    float ambi = max(light_day_intensity.x, light_point_intensity.x);
    float diff = max(
        (1.0 - shadow) * discretize(light_day_intensity.y),
        discretize(light_point_intensity.y));
    float spec = max(
        (1.0 - shadow) * discretize(light_day_intensity.z),
        discretize(light_point_intensity.z));

    vec3 shaded_colour = 
        colour
          * (0.15 * ambi + 0.8 * diff + 0.3 * spec);

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
        FragColour = vec4(shaded_colour, 1.0);
    }
    if (fog_mode == fog_mode_normal)
    {
        FragColour = vec4(
            fog(shaded_colour, dist, fog_colour),
            1.0);
    }
    else if (fog_mode == fog_mode_scatter)
    {
        FragColour = vec4(
            fog_scatter(
                shaded_colour,
                dist,
                fog_colour, // regular fog colour
                fog_sun_colour, // colour when aligned with sun
                view_dir,
                -light_dir),
            1.0);
    }

    // Depth buffer testing
    vec2 st = 0.5 * vec2(
        float(gl_FragCoord.x) / 640.0,
        float(gl_FragCoord.y) / 480.0);
    //float depth = linearize(texture(DepthMap, st).x);
    float depth = texture(DepthMap, st).x;
    //FragColour = vec4(vec3(depth), 1.0);
    //FragColour = vec4(vec3(in_shadow()), 1.0);

    //FragColour = vec4(vec3(linearize(gl_FragCoord.z)), 1.0);
}



