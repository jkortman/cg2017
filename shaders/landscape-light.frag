#version 330

in vec3 FragPos;
in vec3 Colour;
in vec3 Normal;

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
    vec4 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform LightSource LightDay;

uniform vec3 Palette[16];
uniform int PaletteSize;

vec2 calculate_lighting(in LightSource light) {
    vec3 norm = normalize(Normal);

    vec3 light_dir;
    if (light.position.w == 0.0) {
        light_dir = normalize(-light.position.xyz);
    } else {
        light_dir = normalize(vec3(light.position) - FragPos);
    }

    // Calculate ambient component.
    //vec3 ambient = colour * light.ambient;

    // Calculate diffuse component.
    float diff = max(dot(norm, light_dir), 0.0);
    //vec3 diffuse = diff * colour * light.diffuse;

    // Calculate specular component.
    vec3 view_dir = normalize(ViewPos - FragPos);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(norm, halfway_dir), 0.0), MtlShininess);
    //vec3 specular = MtlSpecular * light.specular * spec;

    //return vec4(ambient + diffuse + specular, 1.0);
    return vec2(diff, spec);
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

float linearize(float z)
{
    const float near = 0.05;
    const float far = 1000.0;
    return (2.0 * near) / (far + near - z * (far - near));
}

void main()
{
    /*
    // Determine the base colour of the fragment.
    vec3 colour = match_to_palette(Colour);
    vec2 light_intensity = calculate_lighting(LightDay);
    float diff = light_intensity.x;
    float spec = light_intensity.y;

    FragColour = vec4(
            //colour * discretize(diff),
            colour * discretize(0.8 * diff + 0.3 * spec),   // TODO: Replace these with material properties added by TerrainGenerator
            1.0);
    */

    // Depth buffer testing
    vec2 dt_coord = vec2(
        float(gl_FragCoord.x) / 640.0,
        float(gl_FragCoord.y) / 480.0);
    // the sampled texture is always (1,0,0)?
    float depth = linearize(texture(DepthMap, 0.5 * dt_coord).x); 
    FragColour = vec4(vec3(depth), 1.0);
    //FragColour = vec4(dt_coord.x, dt_coord.y, depth, 1.0);
    //FragColour = vec4(vec3(linearize(gl_FragCoord.z)), 1.0);
}



