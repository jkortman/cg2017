// Implementation of Landscape struct member functions.

#include "Landscape.hpp"

#include <glm/gtc/matrix_transform.hpp>

Landscape::Landscape()
    : model_matrix(glm::mat4(1.0f))
{
    normal_matrix = glm::mat3(
        glm::transpose(glm::inverse(model_matrix)));
}
