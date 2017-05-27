#version 330

in vec3 FragPos;

struct LightSource
{
    vec4 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform LightSource LightDay;
uniform float Time;

out vec4 FragColour;

void main() {
    vec3 Light = normalize(-LightDay.position.xyz);
    vec3 Pos = normalize(FragPos);
    vec3 SunPos  = Pos - Light;
    vec3 MoonPos = Pos + Light;

    // Sky
    vec4 day_colour = vec4(0.7,0.7,0.8+ FragPos.y / 1200.0,1.0);
    vec4 night_colour = vec4(0.0, 0.1, 0.3, 1.0);

    float bound = 0.3;
    float factor = Light.y/bound;
    if (factor > 1) factor = 1;
    if (factor < 0) factor = 0;

    FragColour = day_colour*factor + night_colour*(1-factor);

    // Sun/moon
    if (length(SunPos) < 0.1)
    {
        FragColour = vec4(1.0,1.0,0.0,1.0);
    }

    if (length(MoonPos) <0.05)
    {
        FragColour = vec4(1.0,1.0,1.0,1.0);
    }

}