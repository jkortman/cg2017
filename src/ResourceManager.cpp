// Implementation of ResourceManager class member functions.

#include "ResourceManager.hpp"

ResourceManager::ResourceManager()
{}

void ResourceManager::give_mesh(const std::string& name, Mesh* mesh)
{
    owned_meshes[name] = std::unique_ptr<Mesh>(mesh);
}

Mesh* ResourceManager::get_mesh(const std::string& name)
{
    if (owned_meshes.find(name) == owned_meshes.end())
    {
        // 'name' not in meshes
        throw std::runtime_error("Invalid mesh name '" + name + "'.");
    }
    return owned_meshes[name].get();
}

void ResourceManager::give_shader(const std::string& name, Shader* shader) {
    owned_shaders[name] = std::unique_ptr<Shader>(shader);
    //shaders.push_back(shader);
}

void ResourceManager::give_shader(Shader* shader) {
    give_shader(shader->name, shader);
}

Shader* ResourceManager::get_shader(const std::string& name) {
    if (owned_shaders.find(name) == owned_shaders.end())
    {
        // 'name' not in meshes
        throw std::runtime_error("Invalid shader name '" + name + "'.");
    }
    return owned_shaders[name].get();
}


