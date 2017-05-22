// Implementation of TerrainGenerator class member functions.

#include "TerrainGenerator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

// A heightmap is a matrix of height floats.
// width is leftwards, breadth is downwards.
using HeightT = float;
struct Heightmap {
    Heightmap(int width)
        : width(width)
    {
        map.resize(width * width);
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
    std::vector<HeightT> map;
};

// stb_perlin takes on completely unique values at integer points.
// 'scale' is the width between these points.
static Heightmap generate_heightmap(int width, float scale, float height_scale)
{
    Heightmap hm(width);

    for (int row = 0; row < hm.width; row += 1)
    {
        for (int col = 0; col < hm.width; col += 1)
        {
            #if 0
            float height = stb_perlin_noise3(
                float(row) / scale,     // x
                float(col) / scale,     // y
                0.0f,                   // z
                0, 0, 0);               // wrapping - ignore.
            #endif
            float height = stb_perlin_ridge_noise3(
                float(row) / scale,     // x
                float(col) / scale,     // y
                0.0f,                   // z
                2.0f,                   // lacunarity
                0.5f,                   // gain
                1.0f,                   // offset
                6,                      // octaves
                0, 0, 0);               // wrapping - ignore.
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

// Blur values by taking the mean of the values and their neightbours.
// kernel_size is the radius of the kernel - 1 will take into consideration
// all adjacent values.
void blur(std::vector<glm::vec3>& values, int size, int kernel_size)
{
    auto blur_element = [&](int row, int col)
    {
        glm::vec3 sum(0.0f, 0.0f, 0.0f);
        int num_values = 0;
        for (int i = row - kernel_size; i <= row + kernel_size; i += 1)
        {
            for (int j = col - kernel_size; j <= col + kernel_size; j += 1)
            {
                if (i >= 0 && j >= 0 && i < size && j < size)
                {
                    sum += values[size * i + j];
                    num_values += 1;
                }
            }
        }
        values[size * row + col] = sum * (1.0f / num_values);
    };

    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            blur_element(row, col);
        }
    }
}

Landscape* TerrainGenerator::generate()
{
    const float size = 400.0f;
    Heightmap hm = generate_heightmap(500, 100.0f, 40.0f);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> colours;
    std::vector<std::array<unsigned int, 3>> indices;

    // Build positions.
    // A vertex exists for every value in the heightmap.
    // for a height with position <row, height> in heightmap,
    // The vertex <u, v> is:
    //  u = -size/2 + size * (row / width)
    //  v = -size/2 + size * (row / width)
    for (int row = 0; row < hm.width; row += 1)
    {
        for (int col = 0; col < hm.width; col += 1)
        {
            positions.push_back(
                glm::vec3(
                    -size / 2.0f + size * row / (hm.width - 1),
                    hm.get(row, col),
                    -size / 2.0f + size * col / (hm.width - 1)
                ));
        }
    }

    // Build normals.
    // The normal is the cross product of two vectors:
    // current vertex r, c -> .-->. <- r, c+1
    //                        |
    //                        |
    //                        v  
    //               r+1,c -> .
    for (int row = 0; row < hm.width; row += 1)
    {
        for (int col = 0; col < hm.width; col += 1)
        {
            if (row == hm.width - 1)
            {
                // Copy the normal to the left.
                normals.push_back(
                    normals[row * hm.width + (col - 1)]);
            } else if (col == hm.width - 1)
            {
                // Copy the normal above.
                normals.push_back(
                    normals[(row - 1) * hm.width + col]);
            } else
            {
                // Get the points at, above, and to the right of the current point.
                glm::vec3 at = positions[row * hm.width + col];
                glm::vec3 below = positions[(row + 1) * hm.width + col];
                glm::vec3 right = positions[row * hm.width + (col + 1)];

                glm::vec3 downward = below - at;
                glm::vec3 rightward = right - at;
                glm::vec3 norm = glm::normalize(glm::cross(rightward, downward));
                normals.push_back(norm);
            }
        }
    }

    // Blur normals.
    blur(normals, hm.width, 1);

    // Build colours.
    const glm::vec3 colour_dirt(0.4f, 0.2f, 0.03f);
    const glm::vec3 colour_grass(0.1f, 0.5f, 0.2f);
    const glm::vec3 colour_rock(0.44f, 0.4f, 0.48f);
    const glm::vec3 colour_snow(0.95f, 0.95f, 1.0f);

    for (int row = 0; row < hm.width; row += 1)
    {
        for (int col = 0; col < hm.width; col += 1)
        {
            // Get the angle with the y-axis and normal.
            float yangle =
                (180 / 3.142)
                * acos(glm::dot(
                    AXIS_Y,
                    normals[row * hm.width + col]));
            float height = positions[row * hm.width + col].y;

            // Pick the colour.
            glm::vec3 colour;

            if (height < 8.0f) colour = colour_dirt;
            else if (height < 24.0f) colour = colour_grass;
            else if (height < 31.0f) {
                if (yangle > 30.0f) colour = colour_rock;
                else colour = colour_snow;
            } else colour = colour_snow;
            
            //fprintf(stderr, " yangle: %f, height: %f -> colour: %f %f %f\n", yangle, height, colour.x, colour.y, colour.z);
            colours.push_back(colour);
        }
    }

    // Blur colours.
    blur(colours, hm.width, 3);

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
        for (int col = 0; col < hm.width - 1; col += 1)
        {
            // First triangle (upper-left on diagram).
            indices.push_back(std::array<unsigned int, 3>({
                (unsigned int)( row      * hm.width + col),
                (unsigned int)( row      * hm.width + col + 1),
                (unsigned int)((row + 1) * hm.width + col),
            }));
            // Second triangle (lower-right on diagram).
            indices.push_back({
                (unsigned int)((row + 1) * hm.width + col),
                (unsigned int)( row      * hm.width + col + 1),
                (unsigned int)((row + 1) * hm.width + col + 1),
            });
        }
    }

    // Initialize landscape.
    auto landscape = new Landscape;
    landscape->width = size;
    landscape->breadth = size;
    
    // Copy values into landscape.
    for (int i = 0; i < positions.size(); i += 1)
    {
        landscape->positions.push_back(positions[i].x);
        landscape->positions.push_back(positions[i].y);
        landscape->positions.push_back(positions[i].z);
        landscape->normals.push_back(normals[i].x);
        landscape->normals.push_back(normals[i].y);
        landscape->normals.push_back(normals[i].z);
        landscape->colours.push_back(colours[i].x);
        landscape->colours.push_back(colours[i].y);
        landscape->colours.push_back(colours[i].z);
    }
    for (int i = 0; i < indices.size(); i += 1)
    {
        landscape->indices.push_back(indices[i][0]);
        landscape->indices.push_back(indices[i][1]);
        landscape->indices.push_back(indices[i][2]);
    }

    // Note that ambient and diffuse probably aren't used in the shader,
    // in favor of the per-vertex colours.
    landscape->material.ambient   = glm::vec3(0.1f, 0.5f, 0.2f);
    landscape->material.diffuse   = glm::vec3(0.3f, 0.3f, 0.3f);
    landscape->material.specular  = glm::vec3(0.4f, 0.4f, 0.4f);
    landscape->material.shininess = 4.0f;

    // Populate landscape with objects.

    return landscape;
    return nullptr;
}