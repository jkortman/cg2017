// Renderer class
// Encapsulates OpenGL initialization and rendering code.

#ifndef RENDERER_HPP
#define RENDERER_HPP

#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "core.hpp"
#include "Scene.hpp"

class Renderer
{
public:
    Renderer();
    // Setup the OpenGL environment and settings.
    void initialize();
    // Generate and assign a VAO to a mesh object.
    bool assign_vao(Mesh* mesh);
    // Read and load mesh textures onto the GPU.
    bool create_materials(Mesh* mesh);
    // Render a scene.
    void render(const Scene& scene);
};

#endif // RENDERER_HPP