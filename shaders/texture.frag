#version 330
// Authorship: James Kortman (a1648090)

in vec3 FragPos;
in vec2 TexCoord;
in vec3 Normal;
in vec4 FragPosDeviceSpace;
in vec4 FragPosLightSpace;

uniform sampler2D Texture;

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
    const float N = 4.0;
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


void main() {
    vec4 texel = texture(Texture, vec2(TexCoord.x, 1.0 - TexCoord.y));
    if (texel.a < 0.5) { discard; return; }

    vec3 colour = texel.rgb;

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
        discretize(light_day_intensity.y),
        discretize(light_point_intensity.y));

    // Lighting application
    // TODO: Replace these with material properties added by TerrainGenerator
    colour = colour * (MtlAmbient * ambi + MtlDiffuse * diff);

    FragColour = vec4(colour, 1.0);
    
}