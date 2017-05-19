
#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include <glm/glm.hpp>

#include "Entity.hpp"

class LightSource: public Entity
{
public:
    // If position is a vector, the object is a directional light
    // otherwise it is a point light.
    glm::vec4 position;
    float strength;     // The light strength - 1 = max, 0 = off.
    glm::vec3 ambient;  // ambient colour
    glm::vec3 diffuse;  // diffuse colour
    glm::vec3 specular; // specular colour
};

#endif // LIGHTSOURCE_H