// Implementation of TerrainGenerator class member functions.

#include "TerrainGenerator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

#include <glm/glm.hpp>

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

// Create a new TerrainGenerator.
// The terrain will consist of size*size vertices, and will have
// dimensions edge*edge.
TerrainGenerator::TerrainGenerator(int size, float edge)
    : size(size), edge(edge)
{
    positions.resize(size * size);
    normals.resize(size * size);
    colours.resize(size * size);
    generate();
}

// Convert the contained terrain data into a landscape object.
Landscape* TerrainGenerator::landscape()
{
    // Initialize landscape object.
    Landscape* landscape = new Landscape;
    landscape->width   = edge;
    landscape->breadth = edge;
    
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

    // TODO: Make this toggleable.
    // Note that ambient and diffuse probably aren't used in the shader,
    // in favor of the per-vertex colours.
    landscape->material.ambient   = glm::vec3(0.1f, 0.5f, 0.2f);
    landscape->material.diffuse   = glm::vec3(0.3f, 0.3f, 0.3f);
    landscape->material.specular  = glm::vec3(0.4f, 0.4f, 0.4f);
    landscape->material.shininess = 4.0f;

    return landscape;
}

// ---------------------------
// -- Data access functions --
// ---------------------------
glm::vec3 TerrainGenerator::get_position(int row, int col)
{
    return positions[size * row + col];
}

glm::vec3 TerrainGenerator::get_normal(int row, int col)
{
    return normals[size * row + col];
}

glm::vec3 TerrainGenerator::get_colour(int row, int col)
{
    return colours[size * row + col];
}

void TerrainGenerator::set_position(int row, int col, glm::vec3 pos)
{
    positions[size * row + col] = pos;
}

void TerrainGenerator::set_normal(int row, int col, glm::vec3 norm)
{
    normals[size * row + col] = norm;
}

void TerrainGenerator::set_colour(int row, int col, glm::vec3 colour)
{
    colours[size * row + col] = colour;
}

// ---------------------------------------
// -- Core terrain generation functions --
// ---------------------------------------

// Calls all of the core generator functions in order to create a terrain.
void TerrainGenerator::generate()
{
    generate_heightmap(40.0f, 100.0f);
    generate_positions();
    generate_normals();
    generate_materials();
    generate_indices();
}

// Stage 1: Use noise functions to generate a heightmap according to some
// noise function(s);
void TerrainGenerator::generate_heightmap(float height_limit, float scale)
{
    heightmap = Heightmap(size);
    float minheight = std::numeric_limits<float>::max();
    float maxheight = std::numeric_limits<float>::min();

    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            float height = stb_perlin_ridge_noise3(
                float(row) / scale,     // x
                float(col) / scale,     // y
                0.0f,                   // z
                2.0f,                   // lacunarity
                0.5f,                   // gain
                1.0,                    // offset
                6,                      // octaves
                0, 0, 0);               // wrapping - ignore.

            if (height < minheight) minheight = height;
            if (height > maxheight) maxheight = height;

            heightmap.set(row, col, height);
        }
    }

    // Normalize values to [0, max_height]
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            float height = heightmap.get(row, col);
            height =
                height_limit
                * (height - minheight) / (maxheight - minheight);
            heightmap.set(row, col, height);
        }
    }
}

// Stage 2: Convert the heightmap into positions.
void TerrainGenerator::generate_positions()
{
    // Build positions.
    // A vertex exists for every value in the heightmap.
    // For a height with position <row, height> in heightmap,
    // the vertex <x, y, z> is:
    //  x = -size/2 + size * (row / size)
    //  y = the heightmap value
    //  z = -size/2 + size * (col / size)
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            glm::vec3 position(
                -size / 2.0f + edge * row / (size - 1),
                heightmap.get(row, col),
                -size / 2.0f + edge * col / (size - 1));

            set_position(row, col, position);
        }
    }
}

// Stage 3: Generate normals.
void TerrainGenerator::generate_normals()
{
    // Build normals.
    // The normal is the cross product of two vectors:
    // current vertex row, col -> .-->. <- row, col+1
    //                            |
    //                            |
    //                            v  
    //               row+1,col -> .
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            glm::vec3 norm;
            if      (row == size-1) norm = get_normal(  row, col-1);
            else if (col == size-1) norm = get_normal(row-1, col-1);
            else
            {
                // Get the points at, above, and to the right of the current point.
                glm::vec3 at    = get_position(  row,   col);
                glm::vec3 below = get_position(row+1,   col);
                glm::vec3 right = get_position(  row, col+1);

                glm::vec3 downward = below - at;
                glm::vec3 rightward = right - at;
                norm = glm::normalize(glm::cross(rightward, downward));
            }
            set_normal(row, col, norm);
        }
    }
}

// Stage 4: Generate materials accociated with each position.
void TerrainGenerator::generate_materials()
{
    // Build colours.
    const glm::vec3 colour_dirt(0.4f, 0.2f, 0.03f);
    const glm::vec3 colour_grass(0.1f, 0.5f, 0.2f);
    const glm::vec3 colour_rock(0.44f, 0.4f, 0.48f);
    const glm::vec3 colour_snow(0.95f, 0.95f, 1.0f);

    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            // Get the angle with the y-axis and normal.
            float yangle =
                (180 / 3.142)
                * acos(glm::dot(AXIS_Y, get_normal(row, col)));

            // Get the height at this position.
            float height = get_position(row, col).y;

            // Pick the colour.
            glm::vec3 colour;
            if      (height < 8.0f)  colour = colour_dirt;
            else if (height < 24.0f) colour = colour_grass;
            else if (height < 31.0f) {
                if (yangle > 30.0f) colour = colour_rock;
                else colour = colour_snow;
            } else colour = colour_snow;
            
            //fprintf(stderr, " yangle: %f, height: %f -> colour: %f %f %f\n", yangle, height, colour.x, colour.y, colour.z);
            set_colour(row, col, colour);
        }
    }
}

// Stage 5: Generate indices.
void TerrainGenerator::generate_indices()
{
    // Build indices.
    // For each vertex except the lowest row and furthest left column,
    // connect positions into two triangles:
    // current vertex r, c -> .___. <- r, c+1
    //                        |  /|
    //                        | / |
    //                        |/  |
    //               r+1,c -> .___. <- r+1, c+1
    for (int row = 0; row < size - 1; row += 1)
    {
        for (int col = 0; col < size - 1; col += 1)
        {
            // First triangle (upper-left on diagram).
            indices.push_back(std::array<unsigned int, 3>({
                (unsigned int)( row      * size + col),
                (unsigned int)( row      * size + col + 1),
                (unsigned int)((row + 1) * size + col),
            }));
            // Second triangle (lower-right on diagram).
            indices.push_back({
                (unsigned int)((row + 1) * size + col),
                (unsigned int)( row      * size + col + 1),
                (unsigned int)((row + 1) * size + col + 1),
            });
        }
    }
}

// ----------------------------------
// -- Processing utility functions --
// ----------------------------------
// Blur a property of a vertex on the map by some ammount.
void TerrainGenerator::blur_value(
    Property property, int row, int col, float amt, int kernel_size)
{}

// ----------------------------------------------------
// -- Implementation for TerrainGenerator::Heightmap -- 
// ----------------------------------------------------
TerrainGenerator::Heightmap::Heightmap()
    : size(0)
{}

TerrainGenerator::Heightmap::Heightmap(int size)
    : size(size)
{
    map.resize(size * size);
    std::fill(map.begin(), map.end(), 0.0f);
}

float TerrainGenerator::Heightmap::get(int row, int col)
{
    return map.at(row * size + col);
}

void TerrainGenerator::Heightmap::set(int row, int col, float height)
{
    map[row * size + col] = height;
}