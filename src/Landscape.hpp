// Landscape struct
// A landscape is some mesh and associated objects.
// Landscapes are intended to be generated with some TerrainGenerator,
// and then consumed by a Scene.

#ifndef LANDSCAPE_HPP
#define LANDSCAPE_HPP

#include <glm/glm.hpp>
#include <vector>

#include "Mesh.hpp"
#include "Object.hpp"

struct Landscape
{
    Landscape();

    // Pointed-to objects are not owned!
    // Landscapes should be consumed by a Scene after being generated,
    // which then owns the contents of the Landscape.
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> colours;
    std::vector<unsigned int> indices;
    std::vector<Object*> objects;
    unsigned int vao;
    glm::mat4 model_matrix;
    glm::mat3 normal_matrix;
    struct {
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        float shininess;
    } material;
    float width,    // x-size
          breadth,  // y-size
          height;   // z-size
};

#endif // LANDSCAPE_HPP