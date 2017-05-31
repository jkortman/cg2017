// Implementation of Camera class member functions.

#include <glm/glm.hpp>

#include "Demo.hpp"

void Demo::initialize()
{
    //Node n = ;
    path.push_back({glm::vec3(24.146, 83.500, -60.810), glm::vec3(-0.382, 0.218, -0.898)});
    path.push_back({glm::vec3(55.857, 24.881, 64.486), glm::vec3(-0.805, -0.163, 0.570)});
    path.push_back({glm::vec3(-56.148, 48.541, 121.107), glm::vec3(-0.933, -0.256, 0.252)});

}


node Demo::update_pos(float dt)
{
    time_prev = time;
    time += dt;
    if (time >= 1)
    {
        time = 0;
        segment = (segment+1)%3;
    }

    float c1 = pow(1.0-time,3.0);
    float c2 = 3.0*pow(1.0-time,2.0)*time;
    float c3 = 3.0*(1.0-time)*pow(time,2.0);
    float c4 = pow(time,3.0);

    glm::vec3 p1 = path.at(segment).pos;
    glm::vec3 d1 = path.at(segment).dir;
    glm::vec3 p2 = path.at( (segment+1)%3 ).pos;
    glm::vec3 d2 = path.at( (segment+1)%3 ).dir;

    
    return {c1*p1 + c2*(p1+d1) + c3*(p2+d2) + c4*p2, float(1-time)*d1 + time*d2};

}