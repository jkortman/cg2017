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
    projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.05f, 1200.0f);
    update_view_matrix();
}

glm::mat4& LightSource::update_view_matrix()
{
    // TODO: Switch to actual position.
    view = glm::lookAt(
        glm::vec3(-2.0f, 4.0f, -1.0f), 
        glm::vec3( 0.0f, 0.0f,  0.0f), 
        AXIS_Y);
    return view;
}