// Implementation of Camera class member functions.

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.hpp"
#include "core.hpp"

Camera::Camera(
    const glm::vec4&    position,
    const glm::vec4&    target,
    float               fov,
    float               aspect,
    float               near,
    float               far)
    : position(position),
      target(target)
{
    set_projection(fov, aspect, near, far);
    update_view();
}

void Camera::set_projection(float fov, float aspect, float near, float far)
{
    projection = glm::perspective(
        fov * 3.14f / 180.0f,
        aspect,         // aspect ratio
        near,           // near
        far);         // far
}

void Camera::update_view()
{
    glm::vec3 looking_at;
    if (target.w == 0.0f)
    {
        looking_at = glm::vec3(position + target);
    } else
    {
        looking_at = glm::vec3(target);
    }
    view = glm::lookAt(
        glm::vec3(position),
        looking_at,
        AXIS_Y);
}