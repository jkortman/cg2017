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
    landscape->edge = edge;
    landscape->size = size;
    
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

TerrainGenerator::Biome TerrainGenerator::get_biome(int row, int col)
{
    return Biome(std::round(biome_map.get(row, col)));
}


// ---------------------------------------
// -- Core terrain generation functions --
// ---------------------------------------

// Calls all of the core generator functions in order to create a terrain.
void TerrainGenerator::generate()
{
    const float height_limit = 32.0f;

    generate_base_map(height_limit);
    generate_positions();

    // Blur positions based on height.
    // This make the mountains craggier and the valleys smoother.
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            // Height_factor is 1.0 and decreases as it gets higher.
            // (negative blur ammounts are clamped to 0.0).
            float height = get_position(row, col).y;
            float height_factor =
                (height_limit - 3.0f * height) / height_limit;
            set_position(
                row, col, blur_value(Positions, row, col, height_factor, 5));
        }
    }

    generate_normals();

    // Blur normals. This makes the landscape look visually smoother.
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            set_normal(row, col, blur_value(Normals, row, col, 0.6f, 3));
        }
    }

    generate_materials();

    // Blur colours. This also makes the landscape look visually smoother.
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            set_colour(row, col, blur_value(Colours, row, col, 0.8f, 1));
        }
    }

    generate_indices();
}

// Stage 1: Use noise functions to generate a heightmap according to some
// noise function(s);
void TerrainGenerator::generate_base_map(float height_limit)
{
    heightmap = ValueMap(size);
    biome_map = ValueMap(size);

    // -- Generator method --
    // Uses a biome system that creates two value maps, moisture and altitude,
    // and uses them to assign biomes using the following rules:
    //  (C = Coast, D = Dirt, R = Rock, V = dark grass, G = light grass,
    //   F = Forest, S = Snow)
    //
    //  1.0 | S S S S S S S S S S
    //      | S S S S S S S S S S
    //      | R R R R R F F F F F
    //      | R R R R G F F F F F
    //      | R R R G G G F F F F
    //      | R R V V G G G F F F
    //      | R D V V V G G G F F
    //      | D D D V V V G G G F
    //      | D D D D V V V G G G
    //  0.0 | C C C C C C C C C C
    //        <C below here>
    //       --------------------
    //        0                 1
    // Each vertex has a component value describing how much it belongs in a
    // biome, which is generated by sampling from the above grid.
    const std::array<std::array<Biome, 10>, 10> biome_mapper = {{
        { S, S, S, S, S, S, S, S, S, S, },
        { S, S, S, S, S, S, S, S, S, S, },
        { R, R, R, R, R, F, F, F, F, F, },
        { R, R, R, R, G, F, F, F, F, F, },
        { R, R, R, G, G, G, F, F, F, F, },
        { R, R, V, V, G, G, G, F, F, F, },
        { R, D, V, V, V, G, G, G, F, F, },
        { D, D, D, V, V, V, G, G, G, F, },
        { D, D, D, D, V, V, V, G, G, G, },
        { C, C, C, C, C, C, C, C, C, C, },
    }};

    // Calculate the biome properties for a point with some altitude and moisture level.
    /*
    auto biome_properties = [&](
        float altitude,
        float moisture,
        std::array<float, int(num_biomes)>& biome_values)
        {
            // Each point x,y maps to a square made by four points in biome_mapper.
            //  A.      .B  | A point X has biome components for biomes A, B,
            //              | C, and D. For example, the component for
            //      X.      | component A is dist(X, A) / sum(dist(X, A/B/C/D)).
            //              |
            //  C.      D.  |

            // Map the altitude and float values to the two closest
            // indices to the values provided.
            int alt_min = int(std::floor(altitude));
            int alt_max = int(std::ceil(altitude));
            int mst_min = int(std::floor(moisture));
            int mst_max = int(std::ceil(moisture));

            // Calculate components for each biome.
            float sum = 0.0f;
            std::array<std::array<int, 2>, 2> points = 
                { {alt_min, mst_min}, {alt_min, mst_max}, 
                  {alt_max, mst_min}, {alt_max, mst_max}, };
            for (const auto& point: points)
            {
                // The biome associated with this point.
                Biome point_biome = biome_mapper[point[0]][point[1]];

                // Calculate distance from <alt, moist> to the point.
                float dist = std::sqrt(
                    pow(altitude - point[0], 2) + pow(moisture - point[1], 2));
                sum += dist;
                biome_values[int(point_biome)] += dist;
            }

            // Normalize the biome components.
            for (auto& component: biome_values)
            {
                component /= sum;
            }
        };
    */

    // -- Component maps --
    ValueMap altitude_map(size);
    ValueMap altitude_bias_map(size);
    ValueMap moisture_map(size);
    // This function returns an altitude bias.
    // it is 0 at the edges and 1.0 in the centre.
    auto bias_fn = [=](int row, int col, float offset=0.0f) {
        float x = float(row) / size;
        float y = float(col) / size;
        // Function 2x-2x^2 is interpolation of (0,0), (0.5,1.0), (1,0).
        return std::max(
            (offset + 1.0f) * (4*(x - x*x)) * (4*(y - y*y)) - offset,
            0.0f);
    };
    
    // -- Pass 1: Initialise maps --
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            float altitude = stb_perlin_noise3(
                float(row) * (1.0f / 64.0f) + 32.0f,
                float(col) * (1.0f / 64.0f) + 32.0f,
                0.0f,
                0,0,0);
            float moisture = stb_perlin_noise3(
                float(row) * (1.0f / 64.0f),
                float(col) * (1.0f / 64.0f),
                1.0f,
                0,0,0);

            // Condition the altitude such that is has a coastline.
            const float alt_offset = 1.0f;
            float altitude_bias = bias_fn(row, col, 0.5f);

            altitude_map.set(row, col, altitude * altitude_bias);
            //altitude_bias_map.set(row, col, altitude_bias);
            moisture_map.set(row, col, moisture);
        }
    }

    // -- Pass 2: Generate biomes -- 
    altitude_map.normalize(0.0f, 1.0f);
    moisture_map.normalize(0.0f, 1.0f); 
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            float altitude = altitude_map.get(row, col);
            float moisture = moisture_map.get(row, col);

            // Map altitude and moisture to indices in the array.
            int alt_index = std::floor(10 * (1.0 - altitude));  // because increasing alt_index
            int mst_index = std::floor(10 * altitude);          // is decreasing altitde

            // Get the biome for this value.
            //fprintf(stderr, "altitude: %f -> %d, moisture: %f -> %d\n", altitude, alt_index, moisture, mst_index);
            float biome = float(biome_mapper[alt_index][mst_index]);
            biome_map.set(row, col, biome);
            //fprintf(stderr, "Biome: %f (a: %f, m: %f)\n", biome, altitude, moisture);
        }
    }

    // -- Pass 3: Generate heightmap --
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            float height = altitude_map.get(row, col);
            heightmap.set(row, col, height);
        }
    }

    // -- Pass 4: Normalize values to [0, height_limit] --
    heightmap.normalize(0.0f, 10.0f);
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
                -edge / 2.0f + edge * float(row) / (size - 1),
                heightmap.get(row, col),
                -edge / 2.0f + edge * float(col) / (size - 1));

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

static float randf()
{
    return float(rand()) / (RAND_MAX - 1);
}

glm::vec3 randomize_shade(glm::vec3 colour, float amt)
{
    return
        colour
        + glm::vec3(1.0f, 1.0f, 1.0f) * 2.0f * (randf() - 0.5f) * amt;
}

// Stage 4: Generate materials accociated with each position.
void TerrainGenerator::generate_materials()
{
    // Build colours.
    const std::array<glm::vec3, num_biomes> biome_colours = {
        //glm::vec3(0.70f, 0.55f, 0.03f), // sand
        glm::vec3(1.0f, 0.0f, 0.0f), // sand
        glm::vec3(0.40f, 0.20f, 0.30f), // dirt
        glm::vec3(0.44f, 0.40f, 0.48f), // rock
        glm::vec3(0.07f, 0.40f, 0.15f), // dark grass
        glm::vec3(0.14f, 0.60f, 0.28f), // light grass
        glm::vec3(0.04f, 0.60f, 0.40f), // forest
        glm::vec3(1.00f, 1.00f, 1.00f), // snow
    };

    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            // Get the angle with the y-axis and normal.
            /*
            float yangle =
                (180 / 3.142)
                * acos(glm::dot(AXIS_Y, get_normal(row, col)));
            */

            //fprintf(stderr, "Biome: %d\n", int(get_biome(row, col)));
            glm::vec3 colour = biome_colours[int(get_biome(row, col))];
            //colour = randomize_shade(colour, 0.1f);
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
glm::vec3 TerrainGenerator::blur_value(
    Property property, int row, int col, float amt, int kernel_size)
{
    // Clamp amt to [0, 1].
    if (amt < 0.0f) amt = 0.0f;
    if (amt > 1.0f) amt = 1.0f;

    // Determine the property vector to blur.
    std::vector<glm::vec3>* properties = nullptr;
    switch (property)
    {
    case Positions: properties = &positions; break;
    case Normals:   properties = &normals;   break;
    case Colours:   properties = &colours;   break;
    default:
        fatal("TerrainGenerator::blur_value was passed an invalid property");
    }

    // Average the values over the kernel.
    glm::vec3 sum(0.0f, 0.0f, 0.0f);
    int num_values = 0;
    for (int i = row - kernel_size; i <= row + kernel_size; i += 1)
    {
        for (int j = col - kernel_size; j <= col + kernel_size; j += 1)
        {
            if (i >= 0 && j >= 0 && i < size && j < size)
            {
                sum += properties->at(size * i + j);
                num_values += 1;
            }
        }
    }
    // Return the blurred ammount.
    return (1.0f - amt) * properties->at(size * row + col)
                 + amt  * sum * (1.0f / num_values);
}

// ----------------------------------------------------
// -- Implementation for TerrainGenerator::ValueMap -- 
// ----------------------------------------------------
TerrainGenerator::ValueMap::ValueMap()
    : size(0), min(0.0f), max(0.0f)
{}

TerrainGenerator::ValueMap::ValueMap(int size)
    : size(size),
      min(std::numeric_limits<float>::max()),
      max(std::numeric_limits<float>::min())
{
    map.resize(size * size);
    std::fill(map.begin(), map.end(), 0.0f);
}

float TerrainGenerator::ValueMap::get(int row, int col)
{
    return map.at(row * size + col);
}

void TerrainGenerator::ValueMap::set(int row, int col, float value)
{
    map[row * size + col] = value;
    if (value < min) min = value;
    if (value > max) max = value;
}

void TerrainGenerator::ValueMap::normalize(float norm_min, float norm_max)
{
    // For each value, map it from range <min, max> to <norm_min, norm_max>.
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            float value = map[size * row + col];
            float prop = (value - min) / (max - min);
            float norm_value = norm_min + prop * (norm_max - norm_min);
            map[size * row + col] = norm_value;
        }
    }

    this->min = norm_min;
    this->max = norm_max;
}

