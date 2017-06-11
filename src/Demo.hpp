// Authorship: xJeremy Hughes (a1646624)
// Demo class
// Handles setting up and running a camera demo to fly through a scene

#ifndef DEMO_HPP
#define DEMO_HPP

#include <glm/glm.hpp>
#include <vector>

#include "Entity.hpp"

struct node {glm::vec3 pos; glm::vec3 dir; };
class Demo
{
public:
    void initialize();
    node update_pos(float dt);

    std::vector<node> path;
    bool demo_mode = false;
    int segment = 0;
private:
    float time = 0;
    float time_prev = 0;
};

#endif // DEMO_HPP