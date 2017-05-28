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
    glm::vec3 pos = glm::vec3(30.0f, 30.0f, 30.0f);
    //glm::vec3 dir = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 at = glm::vec3(10.0f, 0.0f, 10.0f);
    view = glm::lookAt(
        pos,
        at,
        AXIS_Y);
}