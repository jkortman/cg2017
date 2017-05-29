// Main function for CG 2017 final project.

#include <cassert>
#include <cmath>
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

const bool          WIREFRAME_MODE = false;
const unsigned int  NUM_AA_SAMPLES = 4;

int main(int argc, char** argv)
{
    Console console;
    console.initialize();
    
    Renderer renderer;
    Scene scene;

    // Renderer setup
    renderer.initialize(WIREFRAME_MODE, NUM_AA_SAMPLES);
    renderer.set_callbacks();
    
    InputHandler::initialize();

    // Scene setup
    const float far_dist = 1200.0f;
    scene.player.position = glm::vec3(0.0, 20.0, 3.0);
    scene.player.direction = glm::vec3(0.0, 0.0, -1.0);
    scene.player.height = 2.0f;
    scene.camera = Camera(
        glm::vec3(0.0, 0.0, 3.0),   // position
        scene.player.direction,     // target
        DEFAULT_FOV,                // fov
        DEFAULT_ASPECT,             // aspect
        0.05f,                      // near
        far_dist);                  // far

    // Create resources.
    ResourceManager resources;

    // Create shaders.
    // To load a shader into the resources, add the name here.
    const std::vector<std::string> shaders = {{
        "landscape-light",
        "water",
        "bplight",
        "depth",
        "texture",
        "obj-cel",
        "simple-sky",
        "skybox",
        "horizon",
    }};
    for (const auto& shname: shaders)
    {
        resources.give_shader(
        shname,
        new Shader("shaders/" + shname + ".vert",
                   "shaders/" + shname + ".frag"));
    }

    scene.depth_shader = resources.get_shader("depth");
    
    // Create meshes.
    // Each mesh entry in meshes is a name, dir name, and filename.
    const std::vector<std::array<std::string, 3>> meshes = {{
        {{ "Cube",       "cube-simple",  "cube-simple"   }},
        {{ "Pine01",     "tree",         "PineTree03"    }},
        {{ "Pine02",     "pine",         "PineTransp"    }},
        {{ "Stump",      "TreeStump",    "TreeStump03"   }},
    }};
    for (const auto& meshinfo: meshes)
    {
        const std::string& name     = meshinfo[0];
        const std::string& dir      = meshinfo[1];
        const std::string& filename = meshinfo[2];
        resources.give_mesh(
            name,
            renderer.create_materials(renderer.assign_vao(
                Mesh::load_obj("models/" + dir + "/", filename + ".obj"))));
    }

    // Create lights.
    scene.world_light_day = LightSource(
        glm::vec4(0.0f, -1.0f, 0.0f, 0.0f),
        0.2f, 0.5f, 1.0f);

    // Test point light
    scene.lights.push_back(LightSource(
        glm::vec4(-20.518629f, 11.420021f, -0.268702f, 1.0f),
        1.0f, 1.0f, 1.0f,
        1.0f, 0.027f, 0.0028f
    ));

    // Test spot light.
    scene.lights.push_back(LightSource(
        glm::vec4(-41.486008f, 24.419973f, 41.395313f, 1.0f),
        1.0f, 1.0f, 1.0f,
        1.0f, 0.027f, 0.0028f,
        glm::vec3(0.017663f, -0.585867f, 0.810214f),
        45.0f
    ));

    // Generate landscape.
    const float max_height = 128.0f;    // Needs to be consistent with water.vert.
    Landscape* landscape = nullptr;
    {
        // Note: The TerrainGenerator requires certain meshes and shaders
        // available with the correct name in resources.
        // See TerrainGenerator::populate().
        TerrainGenerator tg(0, 600, 400.0f, max_height, &resources);
        landscape = renderer.assign_vao(tg.landscape());
        scene.give_landscape(landscape, resources.get_shader("landscape-light"));
        resources.get_shader("landscape-light")->set_palette(landscape->palette);
        scene.player.position =
            landscape->get_pos_at(glm::vec3(0.0f, 0.0f, 0.0f))
            + glm::vec3(0.0f, 1.0f, 0.0f);
    }

    // Create ocean.
    // We can pass the landscape to the water generator and have it cull hidden faces.
    Water* ocean = new Water(75, 1000.0f, 0.05f * max_height, landscape);
    ocean = renderer.assign_vao(ocean);
    scene.give_water(ocean, resources.get_shader("water"));
    resources.get_shader("water")->set_palette(ocean->palette);

    // Create skybox.
    // The skybox must be inside the far plane, meaning the corners
    // of the box must be slightly less than that distance.
    //           __     If we have a circle with radius d, we need
    //          |  --.  a square with radius d/sqrt(2) to stay within
    //        d |  .  \ the bounds of the circle.
    //          |.____| For a sphere and cube inside, we need to divide
    //             d    by sqrt(3).
    Skybox* skybox = renderer.assign_vao(
        new Skybox((far_dist - 1.0f) / std::sqrt(3)));
    scene.give_skybox(skybox, resources.get_shader("skybox"));

    // Create objects.
    scene.give_object(new Object(
        resources.get_mesh("Pine01"),     // mesh
        glm::vec3(-20.0f, 20.0f, 0.0f),     // position
        resources.get_shader("texture")   // shader
    ));

    // Create horizon.
    {
        Object* horizon = new Object(
            resources.get_mesh("Cube"),       // mesh
            glm::vec3(0.0f, 0.0f, 0.0f),   // position
            resources.get_shader("horizon")   // shader
        );
        horizon->scale = glm::vec3(600.0f, 0.0f, 600.0f);
        scene.give_object(horizon);
    }

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