// Camera class
// A camera in the world with a position, direction,
// and associated transformation matrices.

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

#include "Entity.hpp"

class Camera: public Entity
{
public:
    Camera() = delete;
    Camera(
        const glm::vec4&    position,
        const glm::vec4&    target,
        float               fov,
        float               aspect,
        float               near,
        float               far);

    void set_projection(float fov, float aspect, float near, float far);
    
    // update the view matrix to new values of position and direction.
    void update_view();

    // position is a vector for directional lighting.
    glm::vec4 position;
    // target can be a vector or a point.
    // if a vector, it is a direction
    // if it is a point, it is what the camera is looking at.
    glm::vec4 target;
    glm::mat4 projection;
    glm::mat4 view;
};

#endif // CAMERA_H