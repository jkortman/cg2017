// TerrainGenerator class
// Provides functions to generate Landscape. objects

#ifndef TERRAINGENERATOR_HPP
#define TERRAINGENERATOR_HPP

#include "Mesh.hpp"
#include "Landscape.hpp"

#include "stb_perlin.h"

class TerrainGenerator
{
public:
    Landscape* generate_flat();
    Landscape* generate();
private:
};

#endif // TERRAINGENERATOR_HPP