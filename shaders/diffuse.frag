#version 330

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColour;

// Material settings
uniform vec3    MtlAmbient;
uniform vec3    MtlDiffuse;
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

void main()
{
    FragColour = vec4(MtlDiffuse, 1.0);
}