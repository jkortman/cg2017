// Main function for CG 2017 final project.

#include <cassert>

#define MAIN_FILE
#include "core.hpp"
#include "Mesh.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Shader.hpp"
#include "InputHandler.hpp"

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

    Shader* shader = new Shader("shaders/texture.vert", "shaders/texture.frag");
    scene.give_shader("texture", shader);

    Mesh* mesh = Mesh::load_obj("models/tree/", "PineTree03.obj");
    renderer.assign_vao(mesh);
    renderer.create_materials(mesh);
    scene.give_mesh("PineTree03", mesh);

    scene.objects.push_back(Object(
        scene.get_mesh("PineTree03"),               // mesh
        glm::vec3(0.0f, 0.0f, 0.0f),                // position
        scene.get_shader("texture")->program_id     // shader id
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