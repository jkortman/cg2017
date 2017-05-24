// Implementation of Landscape struct member functions.

#include "Landscape.hpp"

#include <glm/gtc/matrix_transform.hpp>

Landscape::Landscape()
    : model_matrix(glm::mat4(1.0f))
{
    normal_matrix = glm::mat3(
        glm::transpose(glm::inverse(model_matrix)));
}

glm::vec3 Landscape::get_pos_at(glm::vec3 player_pos)
{
	float step = (2 * edge)/(-1 + size * 2);
    int row_x = (player_pos.x + edge/2)/step;
    int row_z = (player_pos.z + edge/2)/step;
    int index = 3 * (size * row_x + row_z);
    return glm::vec3(positions.at(index), positions.at(index+1), positions.at(index+2));
}