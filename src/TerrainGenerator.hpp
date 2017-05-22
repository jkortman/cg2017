// TerrainGenerator class
// Provides functions to generate Landscape. objects

#ifndef TERRAINGENERATOR_HPP
#define TERRAINGENERATOR_HPP

#include <array>
#include <vector>
#include <glm/glm.hpp>

#include "Mesh.hpp"
#include "Landscape.hpp"

#include "stb_perlin.h"

class TerrainGenerator
{
public:
    // Create a new TerrainGenerator.
    // The terrain will consist of size*size vertices, and will have
    // dimensions edge*edge.
    TerrainGenerator(int size, float edge);
    TerrainGenerator() = delete;

    // Convert the contained terrain data into a landscape object.
    Landscape* landscape();
private:
    // --------------------
    // -- Internal types --
    // --------------------
    // A heightmap is a matrix of heights (floats).
    // width is leftwards, breadth is downwards.
    struct Heightmap {
        Heightmap();
        Heightmap(int size);
        float get(int row, int col);
        void set(int row, int col, float height);
        int size;
        std::vector<float> map;
    };

    // ------------------
    // -- Terrain data -- 
    // ------------------
    // The number of vertices along each edge.
    int size;
    // The length of each edge.
    float edge;
    // The heightmap for the terrain.
    Heightmap heightmap;
    // The vertex data.
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> colours;
    std::vector<std::array<unsigned int, 3>> indices;

    // ---------------------------
    // -- Data access functions --
    // ---------------------------
    glm::vec3   get_position    (int row, int col);
    glm::vec3   get_normal      (int row, int col);
    glm::vec3   get_colour      (int row, int col);
    void        set_position    (int row, int col, glm::vec3 pos);
    void        set_normal      (int row, int col, glm::vec3 norm);
    void        set_colour      (int row, int col, glm::vec3 colour);

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
    void generate();
    // Stage 1: Use noise functions to generate a heightmap according to some
    // noise function(s). Heights will be normalized to range <0, max_height>.
    // scale is the distance it takes for the noise function to take on a unique
    // point. (these are interpolated between)
    void generate_heightmap(float height_limit, float scale);
    // Stage 2: Convert the heightmap into positions.
    void generate_positions();
    // Stage 3: Generate normals.
    void generate_normals();
    // Stage 4: Generate materials accociated with each position.
    void generate_materials();
    // Stage 5: Generate indices.
    void generate_indices();
    // Stage 6: Object population.
    // TODO.

    // ----------------------------------
    // -- Processing utility functions --
    // ----------------------------------
    enum Property { Positions, Normals, Colours };
    // Blur a property of a vertex on the map by some ammount.
    // (1.0f = full blur, 0.0f = no blur)
    glm::vec3 blur_value(
        Property property, int row, int col, float amt, int kernel_size=1);
};

#endif // TERRAINGENERATOR_HPP