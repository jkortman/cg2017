// Water struct
// A dynamic mesh for water.

#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <array>
#include <glm/glm.hpp>
#include <limits>
#include <vector>

struct Skybox
{
    // Create the skybox mesh.
    Skybox(float radius);

    // Mesh data.
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<std::array<unsigned int, 3>> indices;

    glm::mat4& update_model_matrix(glm::vec3 position);

    // Rendering information.
    unsigned int vao;
    glm::mat4 model_matrix;

private:
    // Generated mesh properties.
    const float radius;

    // Mesh building functions.
    void initialize();
};

#endif // SKYBOX_HPP