// Implementation of Landscape struct member functions.

#include "Landscape.hpp"

#include <numeric>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Landscape::Landscape()
    : model_matrix(glm::mat4(1.0f))
{
    normal_matrix = glm::mat3(
        glm::transpose(glm::inverse(model_matrix)));
}

glm::vec3 Landscape::get_pos_at(glm::vec3 player_pos) const
{
    const float step = (2 * edge)/(-1 + size * 2);
    int row_x = (player_pos.x + edge/2)/step;
    int row_z = (player_pos.z + edge/2)/step;
    int index = size * row_x + row_z;
    return glm::vec3(positions.at(index));
}

float Landscape::get_height_at(float x, float z) const
{
    // Determine the vertices which the point is inside
    const float step = (2 * edge)/(-1 + size * 2);
    std::array<glm::vec3, 3> verts;
    verts[0] = get_pos_at(glm::vec3(x + 0.5f * step, 0.0f, z + 0.5f * step));
    verts[1] = get_pos_at(glm::vec3(x + step, 0.0f, z));
    verts[2] = get_pos_at(glm::vec3(x, 0.0f, z + step));

    // Weight the component heights according to the distance to each point.
    std::array<float, 3> dists;
    auto dist = [=](float x, float z, const glm::vec3& pos)
    {
        return std::sqrt(std::pow(x - pos.x, 2)
                       + std::pow(z - pos.z, 2));
    };
    dists[0] = dist(x, z, verts[0]);
    dists[1] = dist(x, z, verts[1]);
    dists[2] = dist(x, z, verts[2]);
    
    float sum = std::accumulate(std::begin(dists), std::end(dists), 0.0f);

    return verts[0].y * dists[0] / sum
         + verts[1].y * dists[1] / sum
         + verts[2].y * dists[2] / sum;
}