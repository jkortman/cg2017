// Water struct
// A dynamic mesh for water.

#ifndef WATER_HPP
#define WATER_HPP

#include <array>
#include <glm/glm.hpp>
#include <vector>

struct Water
{
    Water(int size, float edge, float level);
    void update(float dt);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> colours;
    std::vector<std::array<unsigned int, 3>> indices;
    struct {
        float shininess;
    } material;
    unsigned int vao;
    glm::mat4 model_matrix;
    glm::mat3 normal_matrix;

private:
    const int size;
    const float edge;
    const float level;

    void initialize();

    void calculate_normals();

    glm::vec3   get_position    (int row, int col);
    glm::vec3   get_normal      (int row, int col);
    glm::vec3   get_colour      (int row, int col);
    void        set_position    (int row, int col, glm::vec3 pos);
    void        set_normal      (int row, int col, glm::vec3 norm);
    void        set_colour      (int row, int col, glm::vec3 colour);
};

#endif // WATER_HPP