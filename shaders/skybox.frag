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
uniform vec3 moon_pos;
uniform vec3 shadow_pos;
uniform float shadow_radius;

out vec4 FragColour;

void main() {
    // TO DO: Need to pass in the horizon location to get the transition right

    bool debug = false;

    float dome_radius = 600;
    float dt = -Time/2.0; //15
    float theta = Time / 2.0;

    vec3 Light = normalize(-LightDay.position.xyz);
    vec3 LightWC = dome_radius*Light;
    vec3 Pos = normalize(FragPos);
    vec3 PosWorld = normalize(FragPosWorld);
    vec3 SunPos  = Pos - Light;

    // (sin(x), sin(x), cos(x)) for Moon first appears at night
    // (sin(x), -sin(x), -cos(x)) for Moon first appears in day
    vec3 moon_light = normalize(vec3(sin(dt), -sin(dt), -cos(dt) )); //normalize(vec3(0.5,0.5,0.5)); //
    moon_light = normalize(vec3(0.1,0.1,0.1));
    vec3 moon_shadow = normalize(vec3(0.2,0.1,0.2));
    /* float phase_center = -0.5*sin(theta)*tan(theta);
    float phase_radius = sqrt(pow(phase_center,2.0)+1);
    vec3 moon_shadow = moon_light - vec3(phase_center,0.0,0.0); */
    vec3 MoonPos = Pos - moon_light ;//Pos + Light;
    vec3 SunPosWorld  = PosWorld - Light;
    vec3 MoonPosWorld = PosWorld + Light;
    vec3 ShadowPos = Pos - shadow_pos ;

    //if (FragPosWorld.z < 0) LightWC *= -1;
    //float horizon = Pos-PosWorld;// 199*(ViewPos.y / ViewPos.z);

    // Sky
    vec4 day_colour = vec4(110.0/256.0, 170.0/256.0, 225.0/256.0, 1.0);//vec4(0.7,0.7,0.8+ FragPos.y / 5000.0,1.0);
    vec4 night_colour = vec4(0.0, 10.0/256.0, 40.0/256.0, 1.0); //vec4(0.0, 0.149 - FragPos.y / 2400.0, 0.301 - FragPos.y / 2400.0, 1.0);//
    
    //if (  <= 1000)
    vec3 cast = normalize(ViewPos-FragPosWorld);

    float bound = 0.2;
    if ( -cast.y < bound  )
    {
        night_colour += vec4(0.0,(bound+cast.y)/4.0, (bound+cast.y)/2.0,0.0);
        day_colour += vec4((bound+cast.y)/2.0, (bound+cast.y)/4.0, 0.0,0.0);
        //night_colour = vec4(0.0, 0.5/cast.y, 0.5/cast.y, 1.0);
    }
    //day_colour = night_colour;

    //night_colour = vec4(floor(FragPos.x), 1.0,1.0, 1.0);

    bound = ViewPos.y + dome_radius * 0.2; // Diameter of the Sun
    float factor = (LightWC.y+ViewPos.y + dome_radius*0.2)/bound;  
    if (factor > 1) factor = 1;
    if (factor < 0) factor = 0;

    FragColour = day_colour*(factor) + night_colour*(1-factor);
    //if (FragPosWorld.y  > 100 && FragPosWorld.y < 150) FragColour = vec4(1.0,0.0,1.0,1.0);
    //if (LightWC.y < 250) FragColour = vec4(0.0,1.0,1.0,1.0);

    if (debug)
    {
        if (FragPosWorld.y < 10 && FragPosWorld.y > -10) FragColour = vec4(0.0,0.5,0.0,1.0);
        if (FragPosWorld.x < 10 && FragPosWorld.x > -10) FragColour = vec4(0.5,0.0,0.0,1.0);
        if (FragPosWorld.z < 10 && FragPosWorld.z > -10) FragColour = vec4(0.0,0.0,0.5,1.0);

        if (FragPos.y < 5 && FragPos.y > 0) FragColour = vec4(0.0,1.0,0.0,1.0);
        if (FragPos.x < 5 && FragPos.x > 0) FragColour = vec4(1.0,0.0,0.0,1.0);
        if (FragPos.z < 5 && FragPos.z > 0) FragColour = vec4(0.0,0.0,1.0,1.0);

        if (length(SunPosWorld) < 0.1)
        {
            FragColour = vec4(1.0,0.0,0.0,1.0);
        }

        if (length(MoonPosWorld) <0.05) //
        {
            FragColour = vec4(0.0,0.0,0.0,1.0);
        }
    }
    



    // Sun/moon
    if (length(SunPos) < 0.08) FragColour = vec4(1.0,0.7,0.3,1.0);
    if (length(SunPos) < 0.065) FragColour = vec4(1.0,0.9,0.4,1.0);
    if (length(SunPos) < 0.057) FragColour = vec4(1.0,1.0,0.5,1.0); 
    if (length(SunPos) < 0.05) FragColour = vec4(1.0,1.0,0.7,1.0);  
    if (length(SunPos) < 0.04) FragColour = vec4(1.0,1.0,1.0,1.0);

     // Maybe vary size over time to simulate getting closer/further away
     // Maybe simulate change in form - track 
    
    float a = 0.7;
    float h = (1.0 -pow(a,2.0))/(-5.0*a);
    float r = sqrt(pow(h,2.0)+1.0);

    vec3 direction = h*(cross(MoonPos,vec3(0.0,1.0,0.0)));

    
    if (length(MoonPos) < 0.065) //0.15
    {
        FragColour += (1.1-factor)*vec4(1.0,1.0,1.0,1.0);
        /* if (length(ShadowPos) < length(ShadowPos-MoonPos) -0.01)
        {
            FragColour = night_colour;
        } */
        
    }



     /*    if ( length(MoonPos -moon_shadow) < phase_radius)
    {
        FragColour = vec4(0.0,0.0,0.0,1.0);
    } */
}