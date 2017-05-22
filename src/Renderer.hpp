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

#include "core.hpp"
#include "Scene.hpp"
#include "InputHandler.hpp"

class Renderer
{
public:
    Renderer();
    // Setup the OpenGL environment and settings.
    void initialize();
    // Register InputHandler functions as callbacks.
    void set_callbacks();
    // Generate and assign a VAO to a landscape object.
    Landscape* assign_vao(Landscape* landscape);
    // Generate and assign a VAO to a mesh object.
    Mesh* assign_vao(Mesh* mesh);
    // Read and load mesh textures onto the GPU.
    Mesh* create_materials(Mesh* mesh);
    // Render a scene.
    void render(const Scene& scene);
    // Cleanup after a single render cycle
    void postrender();
    // Cleanup OpenGL environent.
    void cleanup();
    // Check if the rendering process should end.
    bool should_end();
};

#endif // RENDERER_HPP