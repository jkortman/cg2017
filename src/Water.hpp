// Water struct
// A dynamic mesh for water.

#ifndef WATER_HPP
#define WATER_HPP

#include <array>
#include <glm/glm.hpp>
#include <limits>
#include <vector>

struct Water
{
    // Create the water mesh.
    Water(
        int size,
        float edge,
        float level,
        glm::vec2 min = glm::vec2(
            std::numeric_limits<float>::min(),
            std::numeric_limits<float>::min()),
        glm::vec2 max = glm::vec2(
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max()));

    // Get the distance between vertices.
    float vert_dist();

    // Mesh data.
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> colours;
    std::vector<std::array<unsigned int, 3>> indices;

    // Material data.
    struct {
        float shininess;
    } material;

    glm::vec3 base_colour;
    std::vector<glm::vec3> palette;

    // Rendering information.
    unsigned int vao;
    glm::mat4 model_matrix;
    glm::mat3 normal_matrix;

private:
    // Generated mesh properties.
    const int size;
    const float edge;
    const float level;

    // The max and min bounds (in x,z) for generating faces.
    glm::vec2 min;
    glm::vec2 max;

    // Mesh building functions.
    void initialize();
    void calculate_normals();

    // Coordinate-based element access.
    glm::vec3   get_position    (int row, int col);
    glm::vec3   get_normal      (int row, int col);
    glm::vec3   get_colour      (int row, int col);
    void        set_position    (int row, int col, glm::vec3 pos);
    void        set_normal      (int row, int col, glm::vec3 norm);
    void        set_colour      (int row, int col, glm::vec3 colour);
};

#endif // WATER_HPP