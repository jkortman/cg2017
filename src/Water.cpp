// Implementation of Water struct member functions.

#include "Water.hpp"

Water::Water(
    int size, float edge, float level, Landscape* landscape,
    glm::vec2 min, glm::vec2 max)
    : model_matrix(glm::mat4(1.0f)), 
      size(size),
      edge(edge),
      level(level),
      min(min),
      max(max)
{
    normal_matrix = glm::mat3(
        glm::transpose(glm::inverse(model_matrix)));

    positions.resize(size * size);
    normals.resize(size * size);
    colours.resize(size * size);
    material.shininess = 20.0f;

    // Generate palette.
    base_colour = glm::vec3(0.30f, 0.30f, 1.00f);
    palette = {{
        glm::vec3(0.90f, 0.90f, 1.00f),
        glm::vec3(0.75f, 0.75f, 1.00f),
        glm::vec3(0.60f, 0.60f, 1.00f),
        glm::vec3(0.30f, 0.30f, 1.00f),
        glm::vec3(0.23f, 0.24f, 0.75f),
        glm::vec3(0.16f, 0.16f, 0.5f),
        glm::vec3(0.10f, 0.10f, 0.33f),
    }};
    initialize(landscape);
}

float Water::vert_dist()
{
    return edge / size;
}

void Water::initialize(Landscape* landscape)
{
    auto radius = [](float x, float y) { return std::sqrt(x*x + y*y); };
    // Initialize positions and colours.
    // Set the alpha on edge values
    // Everything outside the edge radius has height 0.
    const float edge_radius = 0.5 * edge - 0.5 * edge / size;

    for (int row = 0; row < size; row += 1)
    {
        for (int col = 0; col < size; col += 1)
        {
            float height = level;
            float x = -edge / 2.0f + edge * float(row) / (size - 1);
            float z = -edge / 2.0f + edge * float(col) / (size - 1);
            glm::vec3 position(
                x,
                (radius(x, z) > edge_radius) ? 0.0f : height,
                z);
            set_position(row, col, position);

            set_colour(row, col, base_colour);
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
    const float max_radius = 0.5 * edge;
    for (int row = 0; row < size - 1; row += 1)
    {
        for (int col = 0; col < size - 1; col += 1)
        {
            // Count the number of verts outside of the max_radius provided.
            int num_outside_radius = 0;
            if (radius(get_position(row, col).x,
                       get_position(row, col).z)
                > max_radius) num_outside_radius += 1;
            if (radius(get_position(row, col + 1).x,
                       get_position(row, col + 1).z)
                > max_radius) num_outside_radius += 1;
            if (radius(get_position(row + 1, col).x,
                       get_position(row + 1, col).z)
                > max_radius) num_outside_radius += 1;
            if (radius(get_position(row + 1, col + 1).x,
                       get_position(row + 1, col + 1).z)
                > max_radius) num_outside_radius += 1;

            // Check if any verts are outside of the bounds provided.
            auto in_bounds = [=](float x, float z)
            {
                return x >= min[0] && z >= min[1] && x <= max[0] && z <= max[1];
            };
            int num_outside_bounds = 0;
            if (!in_bounds(get_position(    row,     col).x,
                           get_position(    row,     col).z)) num_outside_bounds += 1;
            if (!in_bounds(get_position(    row, col + 1).x,
                           get_position(    row, col + 1).z)) num_outside_bounds += 1;
            if (!in_bounds(get_position(row + 1,     col).x,
                           get_position(row + 1,     col).z)) num_outside_bounds += 1;
            if (!in_bounds(get_position(row + 1, col + 1).x,
                           get_position(row + 1, col + 1).z)) num_outside_bounds += 1;

            // Check if any verts are obscured by the landscape.
            auto is_obscured = [=](int row, int col) {
                float x = get_position(row, col).x;
                float z = get_position(row, col).z;
                // check if x,z is witho bounds for the landscape.
                if (x <= -landscape->edge / 2.0 
                    || x >= landscape->edge / 2.0
                    || z <= -landscape->edge / 2.0 
                    || z >= landscape->edge / 2.0) return false;
                return landscape->get_pos_at(get_position(row, col)).y
                        > (level / 2.0) + get_position(row, col).y;
            };
            int num_obscured = 0;
            if (landscape != nullptr)
            {
                if (is_obscured(    row,     col)) num_obscured += 1;
                if (is_obscured(    row, col + 1)) num_obscured += 1;
                if (is_obscured(row + 1,     col)) num_obscured += 1;
                if (is_obscured(row + 1, col + 1)) num_obscured += 1;
            }

            // First triangle (upper-left on diagram).
            if (num_outside_radius      < 4
                && num_outside_bounds   < 4
                && num_obscured         < 4)
            {
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
    }
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
