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
    projection = glm::perspective(
        DEFAULT_FOV * 3.14f / 180.0f,
        DEFAULT_ASPECT,       // aspect ratio
        0.05f,         // near
        1200.0f);         // far
    update_view();
}

glm::mat4& LightSource::update_view()
{
    const glm::vec3 position(-2.0f, 30.0f, -1.0f);
    const glm::vec3 at(0.0f, 0.0f, 0.0f);
    view = glm::lookAt(
        position,
        at,
        AXIS_Y);
    light_space = projection * view;
    return view;
}