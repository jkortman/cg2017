// Main function for CG 2017 final project.

#include <cassert>

#define MAIN_FILE
#include "core.hpp"
#include "Mesh.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "Scene.hpp"
#include "Shader.hpp"
#include "InputHandler.hpp"
#include "Landscape.hpp"
#include "TerrainGenerator.hpp"

int main(int argc, char** argv)
{
    std::printf("starting\n");

    Renderer renderer;
    Scene scene;

    // Renderer setup
    renderer.initialize();
    renderer.set_callbacks();
    
    InputHandler::initialize();

    // Scene setup
    scene.player.position = glm::vec3(0.0, 0.0, 3.0);
    scene.player.direction = glm::vec3(0.0, 0.0, -1.0);
    scene.camera = Camera(
        glm::vec3(0.0, 0.0, 3.0),          // position
        scene.player.direction, // target
        DEFAULT_FOV,                            // fov
        DEFAULT_ASPECT,                         // aspect
        0.05f,                                  // near
        1000.0f);                               // far

    // Create resources.
    ResourceManager resources;

    // Create shaders.
    resources.give_shader(
        "texture", new Shader("shaders/texture.vert", "shaders/texture.frag"));
    resources.give_shader(
        "landscape", new Shader("shaders/landscape.vert", "shaders/landscape.frag"));
    
    // Create meshes.
    resources.give_mesh(
        "Pine01",
        renderer.create_materials(
            renderer.assign_vao(
                Mesh::load_obj("models/tree/", "PineTree03.obj"))));
    resources.give_mesh(
        "Pine02",
        renderer.create_materials(
            renderer.assign_vao(
                Mesh::load_obj("models/pine/", "PineTransp.obj"))));

    // Generate landscape.
    TerrainGenerator tg;
    Landscape* landscape = tg.generate();
    landscape = renderer.assign_vao(landscape);
    scene.give_landscape(landscape, resources.get_shader("landscape"));

    // Create objects.
    scene.give_object(new Object(
        resources.get_mesh("Pine01"),                    // mesh
        glm::vec3(0.0f, 0.0f, 0.0f),                // position
        resources.get_shader("texture")     // shader id
    ));
    scene.give_object(new Object(
        resources.get_mesh("Pine02"),                    // mesh
        glm::vec3(2.0f, 0.0f, 1.0f),                // position
        resources.get_shader("texture")     // shader id
    ));

    // Rendering loop
    float dt = 0.0f;
    while (!renderer.should_end())
    {
        InputHandler::update();
        scene.update(dt);
        renderer.render(scene);
        renderer.postrender();
    }

    renderer.cleanup();
    return EXIT_SUCCESS;
}