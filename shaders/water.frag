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

struct LightSource
{
    vec4 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform LightSource LightDay;

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
    vec3 diffuse = diff * Colour * light.diffuse;

    // Calculate specular component.
    vec3 view_dir = normalize(ViewPos - FragPos);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(norm, halfway_dir), 0.0), MtlShininess);
    vec3 specular = MtlSpecular * light.specular * spec;

    return vec4(ambient + diffuse + specular, 1.0);
}

void main()
{
    FragColour = calculate_lighting(LightDay);
}