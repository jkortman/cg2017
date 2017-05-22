// Implementation of TerrainGenerator class member functions.

#include "TerrainGenerator.hpp"

#include <algorithm>
#include <vector>

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

// A heightmap is a matrix of height floats.
// width is leftwards, breadth is downwards.
using HeightT = float;
struct Heightmap {
    Heightmap(int width, int breadth)
        : width(width), breadth(breadth)
    {
        map.resize(width * breadth);
        std::fill(map.begin(), map.end(), 0.0f);
    }
    HeightT get(int row, int col)
    {
        return map.at(row * width + col);
    }
    void set(int row, int col, HeightT height)
    {
        map[row * width + col] = height;
    }
    int width;
    int breadth;
    std::vector<HeightT> map;
};

// stb_perlin takes on completely unique values at integer points.
// 'scale' is the width between these points.
static Heightmap generate_heightmap(int width, int breadth, float scale, float height_scale)
{
    Heightmap hm(width, breadth);

    for (int row = 0; row < hm.width; row += 1)
    {
        for (int col = 0; col < hm.breadth; col += 1)
        {
            float height = stb_perlin_noise3(
                float(row) / scale,
                float(col) / scale,
                0.0f,
                0, 0, 0);
            hm.set(row, col, height_scale * height);
        }
    }

    return hm;
}

Landscape* TerrainGenerator::generate_flat()
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

Landscape* TerrainGenerator::generate()
{
    const float size = 100.0f;
    Heightmap hm = generate_heightmap(100, 100, 30.0f, 3.0f);

    auto landscape = new Landscape;
    landscape->width = size;
    landscape->breadth = size;
    //landscape->height = 0.0f;

    // Build positions.
    // A vertex exists for every value in the heightmap.
    // for a height with position <row, height> in heightmap,
    // The vertex <u, v> is:
    //  u = -size/2 + size * (row / width)
    //  v = -size/2 + size * (row / width)
    for (int row = 0; row < hm.width; row += 1)
    {
        for (int col = 0; col < hm.breadth; col += 1)
        {
            landscape->positions.push_back(
                -size / 2.0f + size * row / (hm.width - 1));    // x
            landscape->positions.push_back(hm.get(row, col));   // y
            landscape->positions.push_back(
                -size / 2.0f + size * col / (hm.breadth - 1));  // z
        }
    }

    // Build normals.
    for (int row = 0; row < hm.width; row += 1)
    {
        for (int col = 0; col < hm.breadth; col += 1)
        {
            landscape->normals.push_back(0.0f);
            landscape->normals.push_back(1.0f);
            landscape->normals.push_back(0.0f);
        }
    }

    // Build indices.
    // For each vertex except the lowest row and furthest left column,
    // connect positions into two triangles:
    // current vertex r, c -> .___. <- r, c+1
    //                        |  /|
    //                        | / |
    //                        |/  |
    //               r+1,c -> .___. <- r+1, c+1
    for (int row = 0; row < hm.width - 1; row += 1)
    {
        for (int col = 0; col < hm.breadth - 1; col += 1)
        {
            // First triangle (upper-left on diagram).
            landscape->indices.push_back( row      * hm.width + col);
            landscape->indices.push_back( row      * hm.width + col + 1);
            landscape->indices.push_back((row + 1) * hm.width + col);
            // Second triangle (lower-right on diagram).
            landscape->indices.push_back((row + 1) * hm.width + col);
            landscape->indices.push_back( row      * hm.width + col + 1);
            landscape->indices.push_back((row + 1) * hm.width + col + 1);
        }
    }

    landscape->material.ambient   = glm::vec3(0.1, 0.5, 0.2);
    landscape->material.diffuse   = glm::vec3(0.4, 0.8, 0.6);
    landscape->material.specular  = glm::vec3(0.3, 0.7, 0.3);
    landscape->material.shininess = 100.0f;

    return landscape;
}