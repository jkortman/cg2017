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
#include "Shader.hpp"

class Renderer
{
public:
    Renderer();
    // Setup the OpenGL environment and settings.
    void initialize(bool wf = false, unsigned int aa_samples = 1);
    // Register InputHandler functions as callbacks.
    void set_callbacks();
    // Functions to generate and assign VAOs to renderables.
    Landscape* assign_vao(Landscape* landscape);
    Water* assign_vao(Water* water);
    Skybox* assign_vao(Skybox* skybox);
    Mesh* assign_vao(Mesh* mesh);
    // Read and load mesh textures onto the GPU.
    Mesh* create_materials(Mesh* mesh);
    void setup_skybox(Skybox* skybox);
    // Render a scene.
    void render(const Scene& scene);
    // Cleanup after a single render cycle
    void postrender();
    // Cleanup OpenGL environent.
    void cleanup();
    // Check if the rendering process should end.
    bool should_end();

private:
    enum class RenderMode { Scene, Shadow, Depth, Reflect, SSAO };
    void draw_scene(const Scene& scene, RenderMode render_mode);
    void init_shader(
        const Scene& scene, Shader* shader, RenderMode render_mode);

    // Internally, textures are assigned the following numbers:
    //  Num Name              Usage
    //  0   DepthMap          The view-perspective depth map.
    //  1   ShadowDepthMap    The light-perspective depth map.
    //  2   Texture           A texture for a shape in a Mesh object.
    //  3   ReflectMap        The color map for a top-down ortho view of the scene.

    // The FBO and texture for light-perspective depth map (for shadow mapping).
    GLuint shadow_buffer;
    GLuint shadow_texture;
    const unsigned int shadow_texture_size = 1024;
    // The FBO and texture for camera-perspective depth map.
    GLuint depth_buffer;
    GLuint depth_texture;
    const std::array<unsigned int, 2> depth_texture_size =
        {{ DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT }};
    // The FBO and texture to render the landscape from a top-down perspective.
    GLuint reflect_buffer;
    GLuint reflect_texture;
    const unsigned int reflect_texture_size = 1024;
    // The FBO and texture to render the SSAO mapping.
    GLuint ssao_buffer;
    GLuint ssao_texture;
    const std::array<unsigned int, 2> ssao_texture_size = 
        {{ DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT }};
    // The FBO and texture to render the scene to, for postprocessing.
    GLuint scene_buffer;
    GLuint scene_texture;
    const std::array<unsigned int, 2> scene_texture_size = 
        {{ DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT }};
    // The quad to draw on, for postprocessing.
    GLuint quad_vao;
    unsigned int quad_size;
    bool wireframe;
};

#endif // RENDERER_HPP