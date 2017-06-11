/*
Authorship: James Kortman (a1648090) - object loading
            Jeremy Hughes (a1646624) - bounding boxes
*/
// Mesh class
// Provides access to a mesh structure of shapes and associated
// materials, each shape having 

#ifndef MESH_H
#define MESH_H

#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

#include "tiny_obj_loader.h"

enum bound_type {box, cylinder, sphere};
struct Bound {bound_type type; glm::vec3 center; glm::vec3 dims;};

struct Mesh
{
    // Create a Mesh
    static Mesh* load_obj(
        const std::string& dir, const std::string& objfile);
    // Mesh properties.
    std::vector<tinyobj::shape_t> shapes;
    int num_shapes;
    std::vector<tinyobj::material_t> materials;
    // vaos for each shape
    std::vector<unsigned int> vaos;
    // the texture ID for each shape
    std::vector<GLuint> textureIDs;
    // the dir to search for mtl and tex files
    std::string dir;
    // The palette, if one exists.
    std::vector<glm::vec3> palette;
    // Bounding Boxes
    std::vector<Bound> bounds;
};

#endif // MESH_H