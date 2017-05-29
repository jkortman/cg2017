// Implementation of TerrainGenerator class member functions.

#include "TerrainGenerator.hpp"

#include <algorithm>
#include <cassert>
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
TerrainGenerator::TerrainGenerator(
    int seed, int size, float edge, float max_height,
    ResourceManager* resources)
    : size(size), edge(edge), max_height(max_height), resources(resources)
{
    positions.resize(size * size);
    normals.resize(size * size);
    colours.resize(size * size);
    biomes.resize(size * size);

    // Build colours.
    std::vector<std::array<unsigned char, 3>> biome_colours_raw = {{
        /* Error      */ {{ 255,   0,   0 }},
        /* Ocean      */ {{ 222, 198, 160 }},
        /* Beach      */ {{ 209, 184, 142 }},
        /* Dunes      */ {{ 160, 144, 120 }},
        /* Veldt      */ {{ 201, 209, 157 }},
        /* Grassland  */ {{ 137, 169,  90 }},
        /* Woodland   */ {{ 104, 147,  91 }},
        /* Forest     */ {{  71, 135,  87 }},
        /* PineForest */ {{ 153, 169, 121 }},
        /* Rock       */ {{  85,  85,  85 }},
        /* Bare       */ {{ 136, 136, 136 }},
        /* Moor       */ {{ 136, 152, 120 }},
        /* Tundra     */ {{ 187, 187, 171 }},
        /* LightSnow  */ {{ 221, 221, 228 }},
        /* HeavySnow  */ {{ 238, 238, 238 }},
    }};
    for (int i = 0; i < biome_colours_raw.size(); i += 1)
    {
        biome_colours.push_back(glm::vec3(
            float(biome_colours_raw[i][0]) / 255.0f,
            float(biome_colours_raw[i][1]) / 255.0f,
            float(biome_colours_raw[i][2]) / 255.0f));
    }

    generate(seed, max_height);
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
        landscape->positions.push_back(positions[i]);
        landscape->normals.push_back(normals[i]);
        landscape->colours.push_back(colours[i]);
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
    landscape->material.shininess = 0.05f;

    // Copy the palette over to the landscape.
    landscape->palette = std::vector<glm::vec3>(
        biome_colours.begin() + 1,  // skip error colour
        biome_colours.end());

    // Give generated objects to landscape.
    landscape->objects = objects;

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

TerrainGenerator::Biome TerrainGenerator::get_biome(int row, int col)
{
    return biomes[size * row + col];
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

void TerrainGenerator::set_biome(int row, int col, Biome biome)
{
    biomes[size * row + col] = biome;
}

// ---------------------------------------
// -- Core terrain generation functions --
// ---------------------------------------

// Calls all of the core generator functions in order to create a terrain.
void TerrainGenerator::generate(int seed, float max_height)
{
    generate_base_map(seed, max_height);
    generate_positions();
    generate_normals();

    /*
    // Blur normals
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            blur_value(Normals, row, col, 0.8f, 2);
        }
    }
    */

    generate_indices();
    populate();
}

// Stage 1: Use noise functions to generate a heightmap according to some
// noise function(s);
void TerrainGenerator::generate_base_map(int seed, float max_height)
{
    heightmap = ValueMap(size);

    // -- Generator method --
    // Uses a biome system that creates two value maps, moisture and altitude,
    // and uses them to assign biomes using the following rules:
    //  (O = Ocean, C = Coast, D = Dirt, R = Rock,
    //   V = dark grass, G = light grass, F = Forest, S = Snow)
    // todo

    // Moisture and altitude can be obtained for a row/col point using
    // these functions.
    auto altitude_at = [=](int row, int col) -> float
    {
        const float frow = float(row);
        const float fcol = float(col);
        float alt = 0.5 + 0.5 * stb_perlin_fbm_noise3(
            float(row) / 128.0f,            // Coordinates
            float(col) / 128.0f,
            0.5f,
            2.0,                            // Frequency increase per octave
            0.5,                            // Multiplier per successive octave
            6,                              // Number of octaves
            0,0,0);                         // wrapping settings
        // Flatten the altitude to force plains.
        alt = std::pow(alt, 3.5f);
        // The altitude is modified by distance from the centre.
        const float distance =
            std::sqrt(
                std::pow(frow - 0.5 * float(size), 2)
                + std::pow(fcol - 0.5 * float(size), 2))
            / std::sqrt(2 * std::pow(0.5 * float(size), 2));
        // Changing a,b,c changes the island generated.
        // see: http://www.redblobgames.com/maps/terrain-from-noise/
        const float a = 0.20f;
        const float b = 0.85f;
        const float c = 0.40f;
        alt = (alt + a) - b * std::pow(distance, c);
        //alt = (alt + a) * b * std::pow(distance, c);
        return alt;
    };
    auto moisture_at = [=](int row, int col) -> float
    {
        return stb_perlin_fbm_noise3(
            float(row) / 128.0f,            // Coordinates
            float(col) / 128.0f,
            1.5f,
            2.1,                            // Frequency increase per octave
            0.4,                            // Multiplier per successive octave
            6,                              // Number of octaves
            0,0,0);                         // wrapping settings
    };
    auto assign_biome = [=](float altitude, float moisture) -> Biome
    {
        if (altitude < 0.05f) return Ocean;
        if (altitude < 0.07f) return Beach;
        if (altitude < 0.30f)
        {
            if (moisture < 0.12f) return Dunes;
            if (moisture < 0.25f) return Veldt;
            if (moisture < 0.70f) return Grassland;
            if (moisture < 0.90f) return Woodland;
            return Forest;
        }
        if (altitude < 0.6f)
        {
            if (moisture < 0.25) return Veldt;
            if (moisture < 0.5f) return Grassland;
            if (moisture < 0.7f) return Woodland;
            return Forest;
        }
        if (altitude < 0.84f)
        {
            if (moisture < 0.3f) return Tundra;
            if (moisture < 0.6f) return Moor;
            return PineForest;
        }
        if (moisture < 0.2f) return Rock;
        if (moisture < 0.4f) return Bare;
        if (moisture < 0.7f) return LightSnow;
        return HeavySnow;
    };

    // -- Initialization pass --
    // Sets and normalizes the altitude and moisture maps.
    ValueMap altitude_map(size);
    ValueMap moisture_map(size);
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            altitude_map.set(row, col, altitude_at(row, col));
            moisture_map.set(row, col, moisture_at(row, col));
        }
    }
    altitude_map.normalize(0.0f, 1.0f);
    moisture_map.normalize(0.0f, 1.0f);

    // The altitude is modified to force a coast look using a quarter-circle.
    auto interpol = [](float x, float x0, float x1, float y0, float y1)
    {
        return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
    };
    
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            float altitude = altitude_map.get(row, col);
            float moisture = moisture_map.get(row, col);
            // Set height based on the altitude.
            // The height maps altitude [0,1] to height [0,max_height], but not linearly.
            float vert_height = max_height * pow(altitude, 1.5f);
            heightmap.set(row, col, altitude * max_height);

            // Assign colour based on the biome.
            Biome biome = assign_biome(altitude, moisture);
            glm::vec3 colour = biome_colours[int(biome)];
            set_biome(row, col, biome);
            set_colour(row, col, colour);
        }
    }
    
    sealevel = 0.05f * max_height;
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
    fatal("Deprecated function generate_materials");
}

// Stage 5: Generate indices.
void TerrainGenerator::generate_indices()
{
    // Build indices.
    // For each vertex except the lowest row and furthest left column,
    // connect positions into two triangles:
    // current vertex r, c -> a___b <- r, c+1
    //                        |  /|
    //                        | / |
    //                        |/  |
    //               r+1,c -> c___d <- r+1, c+1
    for (int row = 0; row < size - 1; row += 1)
    {
        for (int col = 0; col < size - 1; col += 1)
        {
            const unsigned int a = ( row      * size + col);
            const unsigned int b = ( row      * size + col + 1);
            const unsigned int c = ((row + 1) * size + col);
            const unsigned int d = (unsigned int)((row + 1) * size + col + 1);
            const float a_height = get_position(row, col).y;
            const float b_height = get_position(row, col + 1).y;
            const float c_height = get_position(row + 1, col).y;
            const float d_height = get_position(row + 1, col + 1).y;

            // Only render a triangle if at least one vert is above sea level.
            // First triangle (upper-left on diagram).
            const float cull_height = sealevel * 0.5;
            if (a_height    >= cull_height
                || b_height >= cull_height
                || c_height >= cull_height)
            {
                indices.push_back({{a, b, c}});
            }
            // Second triangle (lower-right on diagram).
            if (c_height    >= cull_height
                || b_height >= cull_height
                || d_height >= cull_height)
            {
                indices.push_back({{c, b, d}});
            }
        }
    }
}


void TerrainGenerator::populate()
{
    // The objects required are:
    Mesh* pine01        = resources->get_mesh("Pine01");
    Mesh* pine02        = resources->get_mesh("Pine02");
    Mesh* stump         = resources->get_mesh("Stump");
    Shader* tex_shader  = resources->get_shader("texture");
    /*new Object(
        resources.get_mesh("Pine01"),     // mesh
        glm::vec3(-20.0f, 20.0f, 0.0f),     // position
        resources.get_shader("texture")   // shader
    )*/

    // To place objects, we evaluate some noise function and
    // select the local maxima, and place an object there with some
    // optional probability. The density of the placed objects
    // depends on the period of the noise function.

    // Add pine trees to PineForest biome.
    {
        std::vector<std::vector<bool>> locations;
        object_position_map(1, 1, locations);
        for (int row = 0; row < size; row += 1)
        {
            for (int col = 0; col < size; col += 1)
            {
                if (locations[row][col] &&
                        (get_biome(row, col) == Woodland
                        || get_biome(row, col) == Forest
                        || get_biome(row, col) == PineForest))
                {
                    float r = randf();
                    Mesh* model;
                    if      (r < 0.45f) model = pine01;
                    else if (r < 0.90f) model = pine02;
                    else                model = stump;

                    Object* obj = new Object(
                        model,
                        get_position(row, col)
                            - glm::vec3(0.1f * randf(),
                                        -0.3f, 
                                        0.1f * randf()), 
                        tex_shader);

                    obj->scale = glm::vec3(1.0f, 1.0f, 1.0f) + 0.1f * randf();
                    objects.push_back(obj);
                }
            }
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

// Populate
static float seed = 0.0f;
void TerrainGenerator::object_position_map(
    int radius,     // The search radius for maxima, or how
                    // dense the generated objects should be.
    int div,        // Each square in the heightmap will be divided
                    // into div*div regions, each of which can have an
                    // object.
    std::vector<std::vector<bool>>& objects) // The output.
{
    objects.resize(size * div);
    for (auto& vec: objects)
    {
        vec.resize(size * div);
        std::fill(vec.begin(), vec.end(), false);
    }

    // Until I properly implement this.
    assert(div == 1);

    ValueMap noise(size * div);

    for (int row = 0; row < size * div; row += 1)
    {
        for (int col = 0; col < size * div; col += 1)
        {
            float value = stb_perlin_noise3(
                float(row) / 8.0f,
                float(col) / 8.0f,
                seed,
                0,0,0);
            noise.set(row, col, value);
        }
    }

    for (int row = 0; row < size * div; row += 1)
    {
        for (int col = 0; col < size * div; col += 1)
        {
            float max = std::numeric_limits<float>::lowest();
            int xmax, ymax;
            for (int i = -radius; i <= radius; i += 1)
            {
                for (int j = -radius; j <= radius; j += 1)
                {
                    int x = row + i, y = col + j;
                    if (x < 0 || y < 0 || x >= size*div || y >= size*div) continue;
                    if (noise.get(x, y) > max) 
                    {
                        max = noise.get(x, y);
                        xmax = x; ymax = y;
                    }
                }
            }
            if (xmax == row && ymax == col) objects[row][col] = true;
        }
    }
    seed += 1.0f;
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
            if (this->min >= this->max)
            {
                map[size * row + col] = norm_min;
            } else
            {
                float value = map[size * row + col];
                float prop = (value - min) / (max - min);
                float norm_value = norm_min + prop * (norm_max - norm_min);
                map[size * row + col] = norm_value;
            }
        }
    }

    this->min = norm_min;
    this->max = norm_max;
}

