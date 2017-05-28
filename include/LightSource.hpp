
#ifndef LIGHTSOURCE_HPP
#define LIGHTSOURCE_HPP

#include <glm/glm.hpp>

#include "Entity.hpp"

class LightSource
{
public:
    LightSource(
        glm::vec4 position  = glm::vec4(0.0, 0.0, 0.0, 1.0),
        glm::vec3 ambient   = glm::vec3(1.0, 1.0, 1.0),
        glm::vec3 diffuse   = glm::vec3(1.0, 1.0, 1.0),
        glm::vec3 specular  = glm::vec3(1.0, 1.0, 1.0),
        float K_constant    = 1.0f,
        float K_linear      = 0.0f,
        float K_quadratic   = 0.0f);
    glm::vec4 position;
    glm::vec3 ambient;  // ambient colour
    glm::vec3 diffuse;  // diffuse colour
    glm::vec3 specular; // specular colour

    // Attenuation settings for point and spot lights.
    void attenuate(float constant, float linear, float quadratic);
    float K_constant;
    float K_linear;
    float K_quadratic;

    // Projection and view matrices for shadow mapping.
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 light_space;
    glm::mat4& update_view();
};


#endif // LIGHTSOURCE_HPP