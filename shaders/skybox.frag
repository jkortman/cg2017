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
uniform samplerCube skybox;

out vec4 FragColour; 

void main() {
    // TO DO: Need to pass in the horizon location to get the transition right

    bool debug = false;

    float dome_radius = 600; // In World Coordinates. Based on terrain generator Edge
    float dt = -Time/2.0; // Scales time
    float theta = Time / 2.0;


    // Vectors normalized to unit sphere - means only useful for relative direction, 
    // unless original direction tracked
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
    

    vec3 MoonPos = Pos - moon_light ;//Pos + Light;
    vec3 SunPosWorld  = PosWorld - Light;
    vec3 MoonPosWorld = PosWorld + Light;
    vec3 ShadowPos = Pos - shadow_pos ;

    
    // Sky
    vec4 day_colour = vec4(110.0/256.0, 170.0/256.0, 225.0/256.0, 1.0);//vec4(0.7,0.7,0.8+ FragPos.y / 5000.0,1.0);
    vec4 night_colour = vec4(0.0, 10.0/256.0, 40.0/256.0, 1.0); //vec4(0.0, 0.149 - FragPos.y / 2400.0, 0.301 - FragPos.y / 2400.0, 1.0);//
    
    vec3 level = normalize(ViewPos-FragPosWorld);

    float bound = 0.2;
    if ( -level.y < bound  )
    {
        night_colour += vec4(0.0,(bound+level.y)/4.0, (bound+level.y)/2.0,0.0);
        day_colour += vec4((bound+level.y)/2.0, (bound+level.y)/4.0, 0.0,0.0);
    }
    
    bound = ViewPos.y + dome_radius * 0.2; // Diameter of the Sun
    float factor = (LightWC.y+ViewPos.y + dome_radius*0.2)/bound;  
    if (factor > 1) factor = 1;
    if (factor < 0) factor = 0;

    vec4 sky_colour = day_colour*(factor) + night_colour*(1-factor);

    FragColour = sky_colour;

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
    
    float q = 0.065; // Radius of Moon
    float a = cos(Time/5.0)*q; // Edge of Moon Shadow
    a = 0.03;
    float err = 0.005;
    if (a < err && a > -err) a = -0.01;

    float h = (pow(q,2.0) - pow(a,2.0))/(-2.0*a); // Distance of Moon Shadow circle from Moon
    float r = sqrt(pow(h,2.0)+pow(q,2.0)); // Radius of Moon Shadow

    theta = h*1.24  ;
    mat3 rot;
    rot[0] = vec3(cos(theta),0,sin(theta));
    rot[1] = vec3(0,1,0);
    rot[2] = vec3(-sin(theta),0,cos(theta));

    ShadowPos = Pos - normalize(vec3(0.1*cos(theta) + 0.1*sin(theta), 0.1, -0.1*sin(theta)+0.1*cos(theta)));

    // Rotation mostly there. Possibly the angle wrong because of distance.

    
    if (length(MoonPos) < q)
    {
        // Left zone
        FragColour += (1.1-factor)*vec4(1.0,1.0,1.0,1.0);
        FragColour = vec4(1.0,1.0,1.0,1.0);
        // Right zone
        if (a < 0) FragColour = sky_colour; 

        // Shadow sphere
        if (length(ShadowPos) < r)
        {
            FragColour = sky_colour; // Left Zone
            if (a < 0) FragColour = vec4(1.0,1.0,1.0,1.0); // Right Zone
        }
        
    }

   //FragColour = vec4(0.7,0.7,0.7,1.0);
    FragColour = 0.8*FragColour + 0.2*texture(skybox, FragPos);

}