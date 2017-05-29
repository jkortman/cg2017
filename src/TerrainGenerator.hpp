// TerrainGenerator class
// Provides functions to generate Landscape. objects

#ifndef TERRAINGENERATOR_HPP
#define TERRAINGENERATOR_HPP

#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>

#include "Mesh.hpp"
#include "Landscape.hpp"
#include "ResourceManager.hpp"

#include "stb_perlin.h"

class TerrainGenerator
{
public:
    // Create a new TerrainGenerator.
    // The terrain will consist of size*size vertices, and will have
    // dimensions edge*edge.
    // The generator does not take ownership of 'resources', and will not
    // delete it.
    TerrainGenerator(
        int seed, int size, float edge, float max_height,
        ResourceManager* resources = nullptr);
    TerrainGenerator() = delete;

    // Convert the contained terrain data into a landscape object.
    Landscape* landscape();
private:
    // --------------------
    // -- Internal types --
    // --------------------
    // Biomes
    enum Biome {
        Error       = 0,    
        Ocean       = 1,    // The ocean.
        Beach       = 2,    // A sandy beach colour.
        Dunes       = 3,    // A dry region close to the beach.
        Veldt       = 4,    // A dry grassland with small shrubs.
        Grassland   = 5,    // A moderate-moisture grassland.
        Woodland    = 6,    // A sparse forest.
        Forest      = 7,    // A much denser forest.
        PineForest  = 8,    // A high-altitude alpine forest.
        Rock        = 9,    // Grey rock.
        Bare        = 10,   // Bare, rocky dirt.
        Moor        = 11,   // A bare high-altitude grassland.
        Tundra      = 12,   // High-altitude, light snow.
        LightSnow   = 13,
        HeavySnow   = 14,
    };
    static constexpr int num_biomes = 15;
    // A heightmap is a matrix of heights (floats).
    // width is leftwards, breadth is downwards.
    struct ValueMap {
        ValueMap();
        ValueMap(int size);
        float get(int row, int col);
        void set(int row, int col, float value);
        void normalize(float norm_min, float norm_max);
        int size;
        float min;
        float max;
        std::vector<float> map;
    };

    // ------------------
    // -- Terrain data -- 
    // ------------------
    // The number of vertices along each edge.
    int size;
    // The length of each edge.
    float edge;
    // The sealevel.
    float sealevel;
    // The heightmap for the terrain.
    ValueMap heightmap;
    // The maximum height of the landscape.
    float max_height;
    // The vertex data.
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> colours;
    std::vector<Biome> biomes;
    std::vector<std::array<unsigned int, 3>> indices;
    // The per-biome colours.
    std::vector<glm::vec3> biome_colours;
    // The object data.
    ResourceManager* resources;

    // ---------------------------
    // -- Data access functions --
    // ---------------------------
    glm::vec3   get_position    (int row, int col);
    glm::vec3   get_normal      (int row, int col);
    glm::vec3   get_colour      (int row, int col);
    Biome       get_biome       (int row, int col);
    void        set_position    (int row, int col, glm::vec3 pos);
    void        set_normal      (int row, int col, glm::vec3 norm);
    void        set_colour      (int row, int col, glm::vec3 colour);
    void        set_biome       (int row, int col, Biome biome);

    // ---------------------------------------
    // -- Core terrain generation functions --
    // ---------------------------------------
    // Note: The generation functions should be called in this order:
    //      1. Heightmap
    //      2. Positions
    //      3. Normals
    //      4. Materials
    //      5. Indices
    //      6. Object population
    // With any required processing functions (see below) called between stages.

    // Calls all of the core generator functions in order to create a terrain.
    void generate(int seed, float max_height);
    // Stage 1: Use noise functions to generate a heightmap according to some
    // noise function(s). Heights will be normalized to range <0, max_height>.
    // scale is the distance it takes for the noise function to take on a unique
    // point. (these are interpolated between)
    void generate_base_map(int seed, float max_height);
    // Stage 2: Convert the heightmap into positions.
    void generate_positions();
    // Stage 3: Generate normals.
    void generate_normals();
    // Stage 4: Generate materials accociated with each position.
    void generate_materials();
    // Stage 5: Generate indices.
    void generate_indices();
    // Stage 6: Object population.
    void populate();

    // ----------------------------------
    // -- Processing utility functions --
    // ----------------------------------
    enum Property { Positions, Normals, Colours };
    // Blur a property of a vertex on the map by some ammount.
    // (1.0f = full blur, 0.0f = no blur)
    glm::vec3 blur_value(
        Property property, int row, int col, float amt, int kernel_size=1);
    
    void object_position_map(
        int radius,     // The search radius for maxima, or how
                        // dense the generated objects should be.
        int div,        // Each square in the heightmap will be divided
                        // into div*div regions, each of which can have an
                        // object.
        std::vector<std::vector<bool>>& objects);

};

#endif // TERRAINGENERATOR_HPP