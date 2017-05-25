#version 330

in vec3 FragPos;
in vec3 Colour;
in vec3 Normal;
in float crest;

out vec4 FragColour;

// Material settings
uniform vec3    MtlAmbient;     // Not used!
uniform vec3    MtlDiffuse;     // Not used!
uniform vec3    MtlSpecular;
uniform float   MtlShininess;
uniform float Time;

uniform vec3 ViewPos;

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
    vec3 specular = light.specular * spec;

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

void main()
{
    // If max_height is updated in main, it needs to be updated here too.
    const float max_height = 128.0;
    const float water_level = 0.05 * max_height;

    // Colour very slightly by depth to give indication of water level.
    float colour_mod = 1.0 + 0.30 * (FragPos.y - water_level);
    FragColour = colour_mod * match_to_palette(calculate_lighting(LightDay));

    //FragColour = vec4(vec3(float(PaletteSize)/10), 1.0);
}