// Entity class
// An abstract class that represents a thing in the world.

#ifndef ENTITY_H
#define ENTITY_H

#include <glm/glm.hpp>

class Entity
{
public:
    glm::vec4 position;
    glm::vec4 direction;
};

#endif // ENTITY_H