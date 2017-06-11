// Authorship: James Kortman (a1648090) & Jeremy Hughes (a1646624)
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
    glm::vec3           get_pos_at      (glm::vec3 player_pos) const;
    std::array<int, 3>  get_tri         (float x, float z) const;
    float               get_height_at   (float x, float z) const;

    // Mesh details for rendering.
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> colours;
    std::vector<unsigned int> indices;

    // The colour palette used by the landscape.
    // May be required for a shader program.
    std::vector<glm::vec3> palette;

    // Geometery details.
    // The length of an edge in world coordinates
    // (generated landscapes are square).
    float edge;
    // The number of vertices along an edge.
    float size;

    // Pointed-to objects are not owned!
    // Landscapes should be consumed by a Scene after being generated,
    // which then owns the contents of the Landscape.
    std::vector<Object*> objects;

    // Rendering details.
    // The Landscape must be assigned VAO by a Renderer.
    unsigned int vao;
    glm::mat4 model_matrix;
    glm::mat3 normal_matrix;
    struct {
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        float shininess;
    } material;
};

#endif // LANDSCAPE_HPP