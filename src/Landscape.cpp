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


// Get the triangle that encloses point (x,z),
// as indices.
std::array<int, 3> Landscape::get_tri(float x, float z) const
{
    std::array<int, 3> indices;

    const float step = (2 * edge)/(-1 + size * 2);
    int row_x = (x + edge/2)/step;
    int row_z = (z + edge/2)/step;
    indices[0] = size * row_x + row_z;
    indices[1] = indices[0] + 1;
    indices[2] = indices[0] + size;
    
    #if 0
    fprintf(
        stderr,
        "(%.2f, %.2f) is inside:\n"
        "  (%.2f, %.2f).__.(%.2f, %.2f)\n"
        "                  | /\n"
        "  (%.2f, %.2f)|/\n",
        x, z,
        positions.at(indices[0]).x, positions.at(indices[0]).z, 
        positions.at(indices[1]).x, positions.at(indices[1]).z, 
        positions.at(indices[2]).x, positions.at(indices[2]).z);
    #endif

    return indices;
}

float Landscape::get_height_at(float x, float z) const
{
    std::array<int, 3> indices = get_tri(x, z);
    glm::vec3 downward = 
        positions.at(indices[1])
        - positions.at(indices[0]);
    glm::vec3 rightward = 
        positions.at(indices[2])
        - positions.at(indices[0]);

    rightward = (1.0f / rightward.x) * rightward;
    downward = (1.0f / downward.z) * downward;
    float dx = x - positions.at(indices[0]).x;
    float dz = z - positions.at(indices[0]).z;
    glm::vec3 position =
        positions.at(indices[0])
        + dx * rightward
        + dz * downward;

    #if 0
    fprintf(stderr,
        "get_height_at(%.2f, %.2f) -> (%.2f, %.2f, %.2f)\n",
        x, z, position.x, position.y, position.z);
    fprintf(stderr,
        "position =\n"
        "  (%.2f, %.2f, %.2f)\n"
        "  + %.2f * (%.2f, %.2f, %.2f)\n"
        "  + %.2f * (%.2f, %.2f, %.2f)\n",
        positions.at(indices[0]).x,
        positions.at(indices[0]).y,
        positions.at(indices[0]).z,
        dx, rightward.x, rightward.y, rightward.z,
        dz, downward.x, downward.y, downward.z);
    #endif

    return position.y;
}







glm::vec3 Landscape::get_pos_at(glm::vec3 player_pos) const
{
    const float step = (2 * edge)/(-1 + size * 2);
    int row_x = (player_pos.x + edge/2)/step;
    int row_z = (player_pos.z + edge/2)/step;
    int index = size * row_x + row_z;
    return glm::vec3(positions.at(index));
}

/*
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
}*/