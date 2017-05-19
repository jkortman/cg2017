// Scene class
// Contains the entire state of the world:
// objects, lights, camera.

#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "Object.hpp"
#include "LightSource.hpp"
#include "Camera.hpp"
#include "Player.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"

class Scene
{
public:
    // The player.
    Player player;
    Camera camera;
    // The objects present in the scene.
    std::vector<Object> objects;
    // Directional lights for day and night, which cast shadows.
    LightSource world_light_day;
    LightSource world_light_night;
    // Point light sources.
    std::vector<LightSource> lights;
    // Access for the shaders required by the scene.
    std::vector<Shader*> shaders;
    // Update the scene after given an elapsed amount of time.
    void update(float dt);
    // Give the scene a mesh to own.
    void give_mesh(const std::string& name, Mesh* mesh);
    // Get a mesh owned by the scene.
    Mesh* get_mesh(const std::string& name);
    // Give the scene a mesh to own.
    void give_shader(Shader* shader);
private:
    // The meshes, stored as owning pointers hashed by name.
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
    // The owned shaders.
    std::vector<std::unique_ptr<Shader>> owned_shaders;
};

#endif // SCENE_H