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
#include "Landscape.hpp"
#include "Water.hpp"
#include "Mesh.hpp"
#include "Skybox.hpp"

class Renderer
{
public:
    Renderer();
    // Setup the OpenGL environment and settings.
    void initialize(bool wireframe = false, unsigned int aa_samples = 1);
    // Register InputHandler functions as callbacks.
    void set_callbacks();
    // Functions to generate and assign VAOs to renderables.
    Landscape* assign_vao(Landscape* landscape);
    Water* assign_vao(Water* water);
    Skybox* assign_vao(Skybox* skybox);
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
private:
    enum class RenderMode { Scene, Depth };
    void draw_scene(const Scene& scene, RenderMode render_mode);
    void init_shader(
        const Scene& scene, Shader* shader, RenderMode render_mode);

    GLuint depth_buffer;
    GLuint depth_texture;
    const unsigned int depth_tex_size = 2048;
};

#endif // RENDERER_HPP