// Object class
// A physical object in the world with some shape, position, orientation.

#ifndef OBJECT_H
#define OBJECT_H

#include "Entity.hpp"
#include "Core.hpp"

#include <glm/glm.hpp>

class Object: public Entity
{
public:
    Mesh* mesh;
    glm::vec3 position
    glm::mat4 model_matrix;
    ShaderID shader_program;
};

#endif