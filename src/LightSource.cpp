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
        
    float scale = 128.0f;
    projection = glm::ortho(-scale, scale, -scale, scale, 0.05f, 1200.0f);
    update_view();
}

glm::mat4& LightSource::update_view()
{
    // Debug setup
    /*
    const glm::vec3 position(-23.953300f, 69.010078f, 49.062695f);
    const glm::vec3 dir(0.970498f, 0.017103f, -0.240506f);
    view = glm::lookAt(
        position,
        position + dir,
        AXIS_Y);
        */
    view = glm::lookAt(
        -700.0f * glm::vec3(position),
        glm::vec3(0.0f),
        AXIS_Y);
    light_space = projection * view;
    return view;
}