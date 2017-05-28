// Implementation of LightSource member functions.

#include "LightSource.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "core.hpp"

LightSource::LightSource(
    glm::vec4 position,
    glm::vec3 ambient,
    glm::vec3 diffuse,
    glm::vec3 specular)
    : position(position),
      ambient(ambient),
      diffuse(diffuse),
      specular(specular)
{
    float scale = 128.0f;
    projection = glm::ortho(-scale, scale, -scale, scale, 0.05f, 1200.0f);
    update_view();
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