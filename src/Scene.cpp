// Implementation of Scene class member functions.

#include "Scene.hpp"

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