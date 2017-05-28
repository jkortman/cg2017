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
};

#endif // MESH_H