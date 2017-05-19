// Implementation of Mesh class member functions.

#include <algorithm>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Mesh.hpp"

Mesh* Mesh::load_obj(const std::string& dir, const std::string& file) {
    Mesh* mesh = new Mesh();
    mesh->dir = dir;
    std::string err;
    bool result = tinyobj::LoadObj(
        mesh->shapes,
        mesh->materials,
        err,
        (dir+file).c_str(),
        dir.c_str());
    mesh->num_shapes = mesh->shapes.size();

    if (!result) {
        std::fprintf(stderr, "Error in Mesh::load(): %s", err.c_str());
        return nullptr;
    }

    return mesh;
}

