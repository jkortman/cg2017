// Implementation of TerrainGenerator class member functions.

#include "TerrainGenerator.hpp"

Landscape* TerrainGenerator::generate()
{
    const float size = 100.0f;

    auto landscape = new Landscape;
    landscape->width = size;
    landscape->breadth = size;
    landscape->height = 0.0f;
    landscape->positions =
    {
        -size / 2.0f, 0.0f, -size / 2.0f,
         size / 2.0f, 0.0f, -size / 2.0f,
        -size / 2.0f, 0.0f,  size / 2.0f,
         size / 2.0f, 0.0f,  size / 2.0f,
    };

    landscape->normals = 
    {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };

    landscape->indices = 
    {
        0,2,1,
        1,2,3,
    };

    landscape->material.ambient   = glm::vec3(0.1, 0.5, 0.2);
    landscape->material.diffuse   = glm::vec3(0.4, 0.8, 0.6);
    landscape->material.specular  = glm::vec3(0.3, 0.7, 0.3);
    landscape->material.shininess = 100.0f;

    return landscape;
}