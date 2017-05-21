// Player class
// A position and direction for a player in the world.

#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.hpp"

class Player: public Entity
{
public:
    glm::vec3 position;
    glm::vec3 direction;
};

#endif // PLAYER_H