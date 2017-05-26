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
	FragColour = vec4(0.7, 0.7, 0.8 + FragPos.y / 1200.0, 1.0);
	
    vec3 light = LightDay.position.xyz;
    light = normalize(light);
    vec3 pos = normalize(FragPos);
    if (light.y < 0)
    {
    	if (pow(pos.x-light.x, 2.0)
    		+ pow(pos.z -light.z,2.0) < 0.01)
	    {
	    	FragColour = vec4(1.0,1.0,0.0,1.0);
		}
    }
    else
    {
    	FragColour = vec4(0.1, 0.1, 0.2 + FragPos.y / 1200.0, 1.0);
    	light = -light;
    	if (pow(pos.x-light.x, 2.0)
    		+ pow(pos.z -light.z,2.0) < 0.005)
	    {
	    	FragColour = vec4(1.0,1.0,1.0,1.0);
		}
    }
}