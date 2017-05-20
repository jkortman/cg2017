// Implementation of Scene class member functions.

#include "Scene.hpp"

#include <cstdio>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdexcept>

Scene::Scene()
{}

void Scene::update(float dt)
{
    if (InputHandler::keys[GLFW_KEY_W])
    {
        player.position.z += 0.2;
    }
    if (InputHandler::keys[GLFW_KEY_S])
    {
        player.position.z -= 0.2;
    }

    camera.position = player.position;
    camera.update_view();

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
        throw std::runtime_error("Invalid mesh name '" + name + "'.");
    }
    return meshes[name].get();
}

void Scene::give_shader(const std::string& name, Shader* shader) {
    owned_shaders[name] = std::unique_ptr<Shader>(shader);
    shaders.push_back(shader);
    
    glUseProgram(shader->program_id);
    warn_if(
        glGetUniformLocation(shader->program_id, "ProjectionMatrix") == -1,
        "Scene given invalid shader with name '" + name + "'");
}

void Scene::give_shader(Shader* shader) {
    give_shader(shader->name, shader);
}

Shader* Scene::get_shader(const std::string& name) {
    if (owned_shaders.find(name) == owned_shaders.end())
    {
        // 'name' not in meshes
        throw std::runtime_error("Invalid shader name '" + name + "'.");
    }
    return owned_shaders[name].get();
}


