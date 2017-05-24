// Implementation of Water struct member functions.

#include "Water.hpp"

Water::Water(int size, float edge, float level)
    : model_matrix(glm::mat4(1.0f)), 
      size(size),
      edge(edge),
      level(level)
{
    normal_matrix = glm::mat3(
        glm::transpose(glm::inverse(model_matrix)));

    positions.resize(size * size);
    normals.resize(size * size);
    colours.resize(size * size);
    material.shininess = 20.0f;
    initialize();
}

void Water::update(float dt)
{
    //calculate_normals();
}

float Water::vert_dist()
{
    return edge / size;
}

void Water::initialize()
{ 
    // Initialize positions and colours.
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            float height = level;
            glm::vec3 position(
                -edge / 2.0f + edge * float(row) / (size - 1),
                height,
                -edge / 2.0f + edge * float(col) / (size - 1));
            set_position(row, col, position);

            glm::vec3 colour(0.30f, 0.30f, 1.00f);
            set_colour(row, col, colour);
        }
    }

    // Initialize normals.
    calculate_normals();

    // Initialize indices.
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
            indices.push_back(std::array<unsigned int, 3>({{
                (unsigned int)( row      * size + col),
                (unsigned int)( row      * size + col + 1),
                (unsigned int)((row + 1) * size + col),
            }}));
            // Second triangle (lower-right on diagram).
            indices.push_back({{
                (unsigned int)((row + 1) * size + col),
                (unsigned int)( row      * size + col + 1),
                (unsigned int)((row + 1) * size + col + 1),
            }});
        }
    }

    fprintf(stderr, "positions: %zu, indices: %zu\n",
        positions.size(), indices.size());

}

void Water::calculate_normals()
{
    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            set_normal(row, col, glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }
}

// -- Data access functions --
// ---------------------------
glm::vec3 Water::get_position(int row, int col)
{
    return positions[size * row + col];
}

glm::vec3 Water::get_normal(int row, int col)
{
    return normals[size * row + col];
}

glm::vec3 Water::get_colour(int row, int col)
{
    return colours[size * row + col];
}

void Water::set_position(int row, int col, glm::vec3 pos)
{
    positions[size * row + col] = pos;
}

void Water::set_normal(int row, int col, glm::vec3 norm)
{
    normals[size * row + col] = norm;
}

void Water::set_colour(int row, int col, glm::vec3 colour)
{
    colours[size * row + col] = colour;
}
