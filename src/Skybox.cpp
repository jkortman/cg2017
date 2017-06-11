// Authorship: Jeremy Hughes (a1646624)
// Implementation of Skybox struct member functions.

#include "Skybox.hpp"

#include <glm/gtc/matrix_transform.hpp>

Skybox::Skybox(float radius)
    : radius(radius)
{
    initialize();    
}

glm::mat4& Skybox::update_model_matrix(glm::vec3 position)
{
    model_matrix = glm::translate(glm::mat4(), position);
    return model_matrix;
}

void Skybox::initialize()
{ 
    // Defines the rendered cube
    positions = {{
        radius * glm::vec3(-1.0f, -1.0f, -1.0f), // left, bottom, far
        radius * glm::vec3(-1.0f, -1.0f,  1.0f), // left, bottom, near
        radius * glm::vec3(-1.0f,  1.0f, -1.0f), // left, top, far
        radius * glm::vec3(-1.0f,  1.0f,  1.0f), // left, top, near
        radius * glm::vec3( 1.0f, -1.0f, -1.0f), // right, bottom, far
        radius * glm::vec3( 1.0f, -1.0f,  1.0f), // right, bottom, near
        radius * glm::vec3( 1.0f,  1.0f, -1.0f), // right, top, far
        radius * glm::vec3( 1.0f,  1.0f,  1.0f), // right, top, near
    }};
    normals = {{
        radius * glm::vec3(  1.0f,  1.0f,  1.0f),
        radius * glm::vec3(  1.0f,  1.0f, -1.0f),
        radius * glm::vec3(  1.0f, -1.0f,  1.0f),
        radius * glm::vec3(  1.0f, -1.0f, -1.0f),
        radius * glm::vec3( -1.0f,  1.0f,  1.0f),
        radius * glm::vec3( -1.0f,  1.0f, -1.0f),
        radius * glm::vec3( -1.0f, -1.0f,  1.0f),
        radius * glm::vec3( -1.0f, -1.0f, -1.0f),  
    }};
    indices = {{
        // Far face
        {{ 0, 6, 2 }},
        {{ 0, 4, 6 }},
        // Near face
        {{ 1, 3, 7 }},
        {{ 1, 7, 5 }},
        // Left face
        {{ 0, 3, 1 }},
        {{ 0, 2, 3 }},
        // Right face
        {{ 4, 5, 6 }},
        {{ 5, 7, 6 }},
        // Top face
        {{ 2, 6, 7 }},
        {{ 7, 3, 2 }},
        // Bottom face
        {{ 0, 5, 4 }},
        {{ 5, 0, 1 }},
    }};
    
}