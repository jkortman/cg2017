// Main function for CG 2017 final project.

#include "core.hpp"
#include "Mesh.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Shader.hpp"

int main(int argc, char** argv)
{
    Renderer renderer;
    Scene scene;

    // Scene setup
    scene.camera = Camera(
        glm::vec4(0.0, 0.0, 3.0, 1.0),          // position
        glm::vec4(0.0, 0.0, 0.0, 1.0),          // target
        DEFAULT_FOV,                            // fov
        float(window_width) / window_height,    // aspect
        0.05f,                                  // near
        10.0f);                                 // far

    scene.give_shader("test1", new Shader("shaders/test1.vert", "shaders/test1.frag"));
    scene.give_mesh("Barrel", Mesh::load_obj("models/Barrel/", "Barrel02.obj"));
    scene.objects.push_back(Object(
        scene.get_mesh("Barrel"),               // mesh
        glm::vec3(0.0f, 0.0f, 0.0f),            // position
        scene.get_shader("test1")->program_id   // shader id
    ));

    // Renderer setup
    renderer.initialize();

    // Rendering loop
    while (!renderer.should_end())
    {
        renderer.render(scene);
        renderer.postrender();
    }

    renderer.cleanup();
    return EXIT_SUCCESS;
}