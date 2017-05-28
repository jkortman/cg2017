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
    Camera();
    Camera(
        const glm::vec3&    position,
        const glm::vec3&    direction,
        float               fov,
        float               aspect,
        float               near,
        float               far);

    void set_projection(float fov, float aspect, float near, float far);
    
    // update the view matrix to new values of position and direction.
    void update_view();

    // position is a vector for directional lighting.
    glm::vec3 position;
    glm::vec3 direction;
    glm::mat4 projection;
    glm::mat4 view;
};

#endif // CAMERA_H