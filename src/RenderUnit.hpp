// Authorship: James Kortman (a1648090)
// RenderUnit struct
// Contains all the necessary data to render an object (and nothing else).

#ifndef RENDERUNIT_H
#define RENDERUNIT_H

#include <glm/glm.hpp>

#include "Mesh.hpp"
#include "core.hpp"

struct RenderUnit {
    glm::mat4   model_matrix;
    glm::mat3   normal_matrix;
    Mesh*       mesh;
    ShaderID    program_id;
};

#endif // RENDERUNIT_H