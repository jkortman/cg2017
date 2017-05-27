#version 330

in vec3 FragPos;
in vec3 FragPosWorld;

struct LightSource
{
    vec4 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform LightSource LightDay;
uniform float Time;
uniform vec3 ViewPos;

out vec4 FragColour;

void main() {
    // TO DO: Need to pass in the horizon location to get the transition right

    float dome_radius = 600;
    float dt = -Time/ 300.0;

    vec3 Light = normalize(-LightDay.position.xyz);
    vec3 LightWC = dome_radius*Light;
    vec3 Pos = normalize(FragPos);
    vec3 PosWorld = normalize(FragPosWorld);
    vec3 SunPos  = Pos - Light;
    vec3 MoonLight = normalize(vec3(cos(dt), 0.5, sin(dt) ));
    vec3 MoonPos = Pos - MoonLight ;//Pos + Light;
    vec3 SunPosWorld  = PosWorld - Light;
    vec3 MoonPosWorld = PosWorld + Light;

    //if (FragPosWorld.z < 0) LightWC *= -1;
    //float horizon = Pos-PosWorld;// 199*(ViewPos.y / ViewPos.z);


    // Sky
    vec4 day_colour = vec4(0.7,0.7,0.8+ FragPos.y / 1200.0,1.0);
    vec4 night_colour = vec4(0.0, 0.1, 0.3, 1.0);

    float bound = ViewPos.y+dome_radius*0.2; // Diameter of the Sun
    float factor = (LightWC.y+ViewPos.y + dome_radius*0.2)/bound;  
    if (factor > 1) factor = 1;
    if (factor < 0) factor = 0;

    FragColour = day_colour*(factor) + night_colour*(1-factor);
    //if (FragPosWorld.y  > 100 && FragPosWorld.y < 150) FragColour = vec4(1.0,0.0,1.0,1.0);
    //if (LightWC.y < 250) FragColour = vec4(0.0,1.0,1.0,1.0);

    /* if (FragPosWorld.y < 10 && FragPosWorld.y > -10) FragColour = vec4(1.0,0.0,0.0,1.0);
    if (FragPosWorld.x < 10 && FragPosWorld.x > -10) FragColour = vec4(1.0,0.0,0.0,1.0);
    if (FragPosWorld.z < 10 && FragPosWorld.z > -10) FragColour = vec4(1.0,0.0,0.0,1.0);

    if (FragPos.y < 5 && FragPos.y > 0) FragColour = vec4(0.0,1.0,0.0,1.0);
    if (FragPos.x < 5 && FragPos.x > 0) FragColour = vec4(0.0,1.0,0.0,1.0);
    if (FragPos.z < 5 && FragPos.z > 0) FragColour = vec4(0.0,1.0,0.0,1.0); */


    // Sun/moon
    if (length(SunPos) < 0.1)
    {
        FragColour = vec4(1.0,1.0,0.0,1.0);
    }

    if (length(MoonPos) <0.065) // Maybe vary over time to simulate getting closer/further away
    {
        FragColour += (1.1-factor)*vec4(1.0,1.0,1.0,1.0);
        //if (FragColour.a == 0) FragColour.a = ;
    }

    /* if (length(SunPosWorld) < 0.1)
    {
        FragColour = vec4(1.0,0.0,0.0,1.0);
    }

    if (length(MoonPosWorld) <0.05)
    {
        FragColour = vec4(0.0,0.0,0.0,1.0);
    } */


}