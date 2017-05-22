// Scene class
// Contains the entire state of the world:
// objects, lights, camera.

#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "core.hpp"
#include "Object.hpp"
#include "LightSource.hpp"
#include "Camera.hpp"
#include "Player.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "InputHandler.hpp"
#include "Landscape.hpp"

class Scene
{
public:
    Scene();

    // The player.
    Player player;
    Camera camera;
    // The objects present in the scene.
    std::vector<Object*> objects;
    // Directional lights for day and night, which cast shadows.
    LightSource world_light_day;
    LightSource world_light_night;
    // Point light sources.
    std::vector<LightSource> lights;
    // The landscape.
    std::unique_ptr<Landscape> landscape;
    Shader* landscape_shader;

    // Update the scene after given an elapsed amount of time.
    void update(float dt);

    // Give the scene an object to own.
    void give_object(Object* object);

    // Give the scene a landscape to own.
    void give_landscape(Landscape* landscape, Shader* shader);
    // Get the owned landscape.
    Landscape* get_landscape();

private:
    // The meshes, stored as owning pointers hashed by name.
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
    // The owned shaders.
    std::unordered_map<std::string, std::unique_ptr<Shader>> owned_shaders;
    std::vector<std::unique_ptr<Object>> owned_objects;
};

#endif // SCENE_H