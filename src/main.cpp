// Main function for CG 2017 final project.

#include <cassert>
#include <cmath>
#include <chrono>
#include <glm/gtc/constants.hpp>
//#include <thread>
#include <stdio.h>

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
#include "Demo.hpp"
#include "Sound.hpp"

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
    scene.player.position  = glm::vec3( 0.0f, 20.0f,  3.0f);
    scene.player.direction = glm::vec3( 0.0f,  0.0f, -1.0f);
    scene.player.height = 2.0f;
    scene.camera = Camera(
        glm::vec3(0.0, 0.0, 3.0),   // position
        scene.player.direction,     // target
        DEFAULT_FOV,                // fov
        DEFAULT_ASPECT,             // aspect
        1.0f,                      // near
        far_dist);                  // far

    Demo* demo = new Demo;
    demo->initialize();
    scene.give_demo(demo);

    // Create resources.
    ResourceManager resources;

    // Create shaders.
    // To load a shader into the resources, add the name here.
    const std::vector<std::string> shaders = {{
        "landscape-light",
        "water",
        "bplight",
        "depth",
        "shadow",
        "extract-brightness",
        "postprocess",
        "reflect",
        "ssao",
        "texture",
        "obj-cel",
        "simple-sky",
        "skybox",
        "horizon",
        "diffuse",
        "blur",
        "hdr",
    }};
    for (const auto& shname: shaders)
    {
        resources.give_shader(shname,
                              new Shader("shaders/" + shname + ".vert",
                                         "shaders/" + shname + ".frag"));
    }

    scene.shadow_shader             = resources.get_shader("shadow");
    scene.depth_shader              = resources.get_shader("depth");
    scene.render_tex_shader         = resources.get_shader("postprocess");
    scene.extract_brightness_shader = resources.get_shader("extract-brightness");
    scene.reflect_shader            = resources.get_shader("reflect");
    scene.ssao_shader               = resources.get_shader("ssao");
    scene.blur_shader               = resources.get_shader("blur");
    scene.hdr_shader                = resources.get_shader("hdr");


    resources.get_shader("ssao")->set_ssao(64);
    resources.get_shader("landscape-light")->set_ssao(64);
    
    // Create meshes.
    // Each mesh entry in meshes is a name, dir name, and filename.
    const std::vector<std::array<std::string, 3>> meshes = {{
        {{ "Cube",          "cube-simple",  "cube-simple"   }},
        {{ "Pine01",        "tree",         "PineTree03"    }},
        {{ "Pine02",        "pine",         "PineTransp"    }},
        {{ "Stump",         "TreeStump",    "TreeStump03"   }},
        {{ "Bonfire",       "bonfire",      "bonfire"       }},
        {{ "Lighthouse",    "lighthouse",   "lighthouse"    }},
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
        glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
        0.2f, 1.0f, 1.0f);

    #if 0
    scene.lights.push_back(LightSource(
        glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
        0.2f, 0.5f, 1.0f));
    scene.world_light_night_index = scene.lights.size() - 1;
    #endif


    // Create objects not generated by terrain
    {
        Object* lighthouse = new Object(
            resources.get_mesh("Lighthouse"),
            glm::vec3(110.330582, 10.8, 173.194229),
            resources.get_shader("texture"));
        lighthouse->scale = glm::vec3(1.7f);
        //lighthouse->y_rotation = glm::pi<float>();
        scene.give_object(lighthouse);
        // Spotlight for lighthouse.
        scene.lights.push_back(LightSource(
            glm::vec4(110.330582, 32.508884, 173.194229, 1.0f),
            0.1f, 1.0f, 0.0f,
            1.0f, 0.015f, 0.000007f,
            glm::vec3(0.0f, -0.3f, 1.0f),
            0.01f
        ));
        scene.lighthouse_light_index = scene.lights.size() - 1;
    }
    
    {
        Object* bonfire = new Object(
            resources.get_mesh("Bonfire"),
            glm::vec3(33.584152f, 7.408922f, -54.512539f),
            resources.get_shader("obj-cel"));
        scene.give_object(bonfire);
        // Campfire point light.
        scene.lights.push_back(LightSource(
            glm::vec4(33.584152f, 8.208922f, -54.512539f, 1.0f),
            0.8f, 1.0f, 0.0f,
            1.0f, 0.09f, 0.0032f
        ));
    }

    // Generate landscape.
    const float max_height = 128.0f;    // Needs to be consistent with water.vert.
    Landscape* landscape = nullptr;
    {
        // Note: The TerrainGenerator requires certain meshes and shaders
        // available with the correct name in resources.
        // See TerrainGenerator::populate().
        TerrainGenerator tg(0, 100, 400.0f, max_height, &resources);
        landscape = renderer.assign_vao(tg.landscape());
        scene.give_landscape(landscape, resources.get_shader("landscape-light"));
        resources.get_shader("landscape-light")->set_palette(landscape->palette);
        resources.get_shader("reflect")->set_palette(landscape->palette);
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

    // Create horizon.
    {
        Object* horizon = new Object(
            resources.get_mesh("Cube"),       // mesh
            glm::vec3(0.0f, 0.0f, 0.0f),      // position
            resources.get_shader("horizon")   // shader
        );
        horizon->scale = glm::vec3(600.0f, 0.0f, 600.0f);
        scene.give_object(horizon);
    }
    
    // Start background sounds
    Sound* sound = new Sound;
    sound->initialize();
    scene.give_sound(sound);

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

    // End Sounds
    if (sound->enabled) sound->cleanup();

    return EXIT_SUCCESS;
}