// Implementation of Camera class member functions.

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.hpp"
#include "core.hpp"

Camera::Camera()
    : position(glm::vec3(0.0, 0.0, 0.0)),
      direction(glm::vec3(0.0, 0.0, 1.0))
{
    set_projection(DEFAULT_FOV, DEFAULT_ASPECT, DEFAULT_NEAR, DEFAULT_FAR);
    update_view();
}

Camera::Camera(
    const glm::vec3&    position,
    const glm::vec3&    direction,
    float               fov,
    float               aspect,
    float               near,
    float               far)
    : position(position),
      direction(direction)
{
    set_projection(fov, aspect, near, far);
    update_view();
}

void Camera::set_projection(float fov, float aspect, float near, float far)
{
    projection = glm::perspective(
        fov * 3.14f / 180.0f,
        aspect,       // aspect ratio
        near,         // near
        far);         // far
}

void Camera::update_view()
{
    view = glm::lookAt(
        position,
        position + direction,
        AXIS_Y);
}