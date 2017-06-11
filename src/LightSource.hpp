// Authorship: James Kortman (a1648090)

#ifndef LIGHTSOURCE_HPP
#define LIGHTSOURCE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Entity.hpp"

class LightSource
{
public:
    LightSource(
        glm::vec4 position       = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        float     ambient        = 1.0f,
        float     diffuse        = 1.0f,
        float     specular       = 1.0f,
        float     K_constant     = 1.0f,
        float     K_linear       = 0.0f,
        float     K_quadratic    = 0.0f,
        glm::vec3 spot_direction = glm::vec3(0.0f, 0.0f, 0.0f),
        float     spot_angle     = 180.0f);
    glm::vec4 position;
    float     ambient;  // ambient amt
    float     diffuse;  // diffuse amt
    float     specular; // specular amt

    // Attenuation settings for point and spot lights.
    void attenuate(float constant, float linear, float quadratic);
    float K_constant;
    float K_linear;
    float K_quadratic;

    // Spot light maximum angle (in degrees).
    // Set to 180deg or more for a spot light.
    glm::vec3 spot_direction;
    float spot_angle;

    // Projection and view matrices for shadow mapping.
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 light_space;
    glm::mat4& update_view();
};

#endif // LIGHTSOURCE_HPP