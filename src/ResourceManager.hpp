// Authorship: James Kortman (a1648090)
// ResourceManager class
// Provides named access to resources needed by the program:
//  - meshes
//  - shaders

#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include "Mesh.hpp"
#include "Shader.hpp"

#include <memory>
#include <string>
#include <unordered_map>

class ResourceManager
{
public:
    ResourceManager();

    // All the shaders owned by the ResourceManager,
    // which can be ietrated over.
    std::vector<Shader*> shaders;

    // Give the manager a mesh to own.
    void give_mesh(const std::string& name, Mesh* mesh);
    // Get a mesh owned by the scene by name.
    // Throws std::runtime_error on failure.
    Mesh* get_mesh(const std::string& name);

    // Give the manager a shader to own.
    void give_shader(const std::string& name, Shader* shader);
    void give_shader(Shader* shader);
    // Get a shader owned by the manager by name.
    // Throws std::runtime_error on failure.
    Shader* get_shader(const std::string& name);

private:
    // The meshes, stored as owning pointers hashed by name.
    std::unordered_map<std::string, std::unique_ptr<Mesh>> owned_meshes;
    // The owned shaders.
    std::unordered_map<std::string, std::unique_ptr<Shader>> owned_shaders;
};

#endif // RESOURCEMANAGER_HPP