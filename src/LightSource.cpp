// Implementation of LightSource member functions.

#include "LightSource.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "core.hpp"

LightSource::LightSource(
    glm::vec4 position,
    float ambient,
    float diffuse,
    float specular,
    float K_constant,
    float K_linear,
    float K_quadratic,
    glm::vec3 spot_direction,
    float spot_angle)
    : position      (position),
      ambient       (ambient),
      diffuse       (diffuse),
      specular      (specular),
      K_constant    (K_constant),
      K_linear      (K_linear),
      K_quadratic   (K_quadratic),
      spot_direction(spot_direction),
      spot_angle    (spot_angle)
{
    float scale = 128.0f;
    projection = glm::ortho(-scale, scale, -scale, scale, 0.05f, 1200.0f);
    update_view();
}

void LightSource::attenuate(float constant, float linear, float quadratic)
{
    K_constant  = constant;
    K_linear    = linear;
    K_quadratic = quadratic;
}


glm::mat4& LightSource::update_view()
{
    view = glm::lookAt(
        -700.0f * glm::vec3(position),
        glm::vec3(0.0f),
        AXIS_Y);
    light_space = projection * view;
    return view;
}