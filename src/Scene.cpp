// Implementation of Scene class member functions.

#include "Scene.hpp"

#include <cstdio>
#include <GL/glew.h>

Scene::Scene()
{}

void Scene::update(float dt)
{

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