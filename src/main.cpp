// Main function for CG 2017 final project.

#include <cassert>
#include <chrono>

#define MAIN_FILE
#include "core.hpp"
#include "LightSource.hpp"
#include "Console.hpp"
#include "Mesh.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "Scene.hpp"
#include "Shader.hpp"
#include "InputHandler.hpp"
#include "Landscape.hpp"
#include "TerrainGenerator.hpp"
#include "Water.hpp"

const bool WIREFRAME_MODE = false;

int main(int argc, char** argv)
{
    Console console;
    console.initialize();
    
    Renderer renderer;
    Scene scene;

    // Renderer setup
    renderer.initialize(WIREFRAME_MODE);
    renderer.set_callbacks();
    
    InputHandler::initialize();

    // Scene setup
    scene.player.position = glm::vec3(0.0, 20.0, 3.0);
    scene.player.direction = glm::vec3(0.0, 0.0, -1.0);
    scene.player.height = 2.0f;
    scene.camera = Camera(
        glm::vec3(0.0, 0.0, 3.0),   // position
        scene.player.direction,     // target
        DEFAULT_FOV,                // fov
        DEFAULT_ASPECT,             // aspect
        0.05f,                      // near
        1000.0f);                   // far

    // Create resources.
    ResourceManager resources;

    // Create shaders.
    resources.give_shader(
        "texture",
        new Shader("shaders/texture.vert", "shaders/texture.frag"));
    resources.give_shader(
        "landscape",
        new Shader("shaders/landscape.vert", "shaders/landscape.frag"));
    resources.give_shader(
        "landscape-light",
        new Shader("shaders/landscape-light.vert", "shaders/landscape-light.frag"));
    resources.give_shader(
        "water",
        new Shader("shaders/water.vert", "shaders/water.frag"));
    resources.give_shader(
        "bplight",
        new Shader("shaders/bplight.vert", "shaders/bplight.frag"));
    
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

    // Create lights.
    scene.world_light_day = LightSource(
        glm::vec4(0.0f, -1.0f, 0.0f, 0.0f),
        glm::vec3(0.2f, 0.2f, 0.2f),
        glm::vec3(0.5f, 0.5f, 0.48f),
        glm::vec3(1.0f, 1.0f, 1.0f));

    // Generate landscape.
    TerrainGenerator tg(600, 400.0f);
    Landscape* landscape = renderer.assign_vao(tg.landscape());
    scene.give_landscape(landscape, resources.get_shader("landscape-light"));

    // Create ocean.
    Water* ocean = new Water(2, 1000.0f, 0.05f * 128.0f);
    ocean = renderer.assign_vao(ocean);
    scene.give_water(ocean, resources.get_shader("water"));

    // Create objects.
    scene.give_object(new Object(
        resources.get_mesh("Pine01"),     // mesh
        glm::vec3(0.0f, 20.0f, 0.0f),      // position
        resources.get_shader("bplight")   // shader
    ));

    // Rendering loop
    auto current_time = std::chrono::steady_clock::now();
    while (!renderer.should_end())
    {
        // Calculate time difference between now and prev. frame
        // dt is in seconds ???
        auto frame_start_time = std::chrono::steady_clock::now();
        float dt = std::chrono::duration_cast
            <std::chrono::duration<float>>
            (frame_start_time - current_time).count();
        current_time = frame_start_time;

        InputHandler::update();
        scene.update(dt);
        renderer.render(scene);
        renderer.postrender();
    }

    renderer.cleanup();
    return EXIT_SUCCESS;
}