// Implementation of Scene class member functions.

#include "Scene.hpp"

#include <cstdio>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Scene::Scene()
{}

void Scene::update(float dt)
{
    if (InputHandler::mouse_buttons[GLFW_MOUSE_BUTTON_LEFT])
    {
        objects[0].x_rotation += 0.3 * InputHandler::mouse_dy;
        objects[0].y_rotation += 0.3 * InputHandler::mouse_dx;
    }

    // Update the model and normal matrices for each object.
    for (auto& object : objects)
    {
        object.update_model_matrix();
        object.update_normal_matrix();
    }
}

void Scene::give_mesh(const std::string& name, Mesh* mesh)
{
    meshes[name] = std::unique_ptr<Mesh>(mesh);
}

Mesh* Scene::get_mesh(const std::string& name)
{
    if (meshes.find(name) == meshes.end())
    {
        // 'name' not in meshes
        return nullptr;
    }
    return meshes[name].get();
}

void Scene::give_shader(const std::string& name, Shader* shader) {
    owned_shaders[name] = std::unique_ptr<Shader>(shader);
    shaders.push_back(shader);
    
    glUseProgram(shader->program_id);
    assert(glGetUniformLocation(shader->program_id, "ProjectionMatrix") != -1);
}

void Scene::give_shader(Shader* shader) {
    give_shader(shader->name, shader);
}

Shader* Scene::get_shader(const std::string& name) {
    if (owned_shaders.find(name) == owned_shaders.end())
    {
        // 'name' not in meshes
        return nullptr;
    }
    return owned_shaders[name].get();
}


