// Authorship: Jeremy Hughes (a1646624)
// Implementation of Demo class member functions.

#include <glm/glm.hpp>

#include "Demo.hpp"

void Demo::initialize()
{
    // Preset waypoints of positions and view directions to move between
    path.push_back({glm::vec3(77.623375,    127.170441, 223.853012  ), 
                    glm::vec3(-0.345340,    -0.286813,  -0.893570   )});
    path.push_back({glm::vec3(-25.301527,   43.711834,  149.005341  ), 
                    glm::vec3(-0.488909,    -0.228846,  -0.841776   )});
    path.push_back({glm::vec3(-116.194443,  23.120001,  56.134884   ), 
                    glm::vec3(-0.750126,    -0.068804,  -0.657697   )});
    path.push_back({glm::vec3(-193.906433,  25.927498,  -12.062922  ), 
                    glm::vec3(0.938399,     0.032146,   -0.344036   )});
    path.push_back({glm::vec3(-107.764679,  53.754005,  -51.407833  ), 
                    glm::vec3(0.490392,     0.414959,   0.766363    )});
    path.push_back({glm::vec3(-4.778440,    47.495693,  -11.931894  ), 
                    glm::vec3(0.518191,     -0.235773,  -0.822116   )});
    path.push_back({glm::vec3(85.165764,    20.252850,  -9.554564   ), 
                    glm::vec3(0.193764,     -0.067136,  0.978740    )});
    path.push_back({glm::vec3(127.177681,   15.682218,  82.138397   ), 
                    glm::vec3(-0.262573,    -0.067156,  0.962564    )});
    path.push_back({glm::vec3(142.845627,   24.333654,  173.451859  ), 
                    glm::vec3(-0.924682,    -0.113589,  -0.363377   )});
    path.push_back({glm::vec3(54.038975,    22.887941,  104.372223  ), 
                    glm::vec3(-0.376358,    0.060079,   -0.924514   )});
    path.push_back({glm::vec3(69.470825,    16.335619,  -51.539536  ), 
                    glm::vec3(0.354410,     -0.121278,  -0.927016   )});
    path.push_back({glm::vec3(108.299713,   15.798645,  -145.471832 ), 
                    glm::vec3(-0.530272,    0.449465,   -0.718655   )});
    path.push_back({glm::vec3(9.754910,     36.220127,  -210.334045 ), 
                    glm::vec3(-0.248728,    0.118151,   0.961169    )});
    path.push_back({glm::vec3(-115.727654,  34.545322,  -94.398827  ), 
                    glm::vec3(0.015763,     0.173253,   0.984750    )});
    path.push_back({glm::vec3(-124.571404,  51.518673,  -12.775669  ), 
                    glm::vec3(0.692939,     0.024141,   0.720590    )});
    path.push_back({glm::vec3(-119.334503,  54.853302,  63.432659   ), 
                    glm::vec3(0.944916,     0.278315,   0.172258    )});
}


node Demo::update_pos(float dt)
{
    // Motion defined along a third-order (cubic) Bezier curve. 
    // The control points (P1 and P2), are a point in the unit direction the player is facing, scaled by d_length;
    float d_length = 50.0f;
    float duration = 3.0f;

    time_prev = time;
    time += dt/duration;
    if (time >= 1)
    {
        time = 0;
        segment = (segment+1)%path.size();
    }

    float c1 = pow(1.0-time,3.0);
    float c2 = 3.0*pow(1.0-time,2.0)*time;
    float c3 = 3.0*(1.0-time)*pow(time,2.0);
    float c4 = pow(time,3.0);

    glm::vec3 p1 = path.at(segment).pos;
    glm::vec3 d1 = path.at(segment).dir;
    glm::vec3 p2 = path.at( (segment+1)%path.size() ).pos;
    glm::vec3 d2 = path.at( (segment+1)%path.size() ).dir;

    d1 *= d_length;
    d2 *= -d_length; // Negative so that the path is more quadratic than sinusoidal

    return {c1*p1 + c2*(p1+d1) + c3*(p2+d2) + c4*p2, float(1-time)*d1 - time*d2};
}