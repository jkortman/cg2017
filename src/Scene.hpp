// Authorship: James Kortman (a1648090) & Jeremy Hughes (a1646624)
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
#include "Water.hpp"
#include "Skybox.hpp"
#include "Demo.hpp"
#include "Sound.hpp"

class Scene
{
public:
    Scene();

    // The player.
    Player player;
    Camera camera;
    std::unique_ptr<Demo> demo;
    std::unique_ptr<Sound> sound;
    // The objects present in the scene.
    std::vector<Object*> objects;
    // Directional lights for day and night, which cast shadows.
    LightSource world_light_day;
    int world_light_night_index;
    int lighthouse_light_index;
    // Point light sources.
    std::vector<LightSource> lights;
    // The landscape.
    std::unique_ptr<Landscape> landscape;
    Shader* landscape_shader;
    // The water.
    std::unique_ptr<Water> water;
    Shader* water_shader;
    // The skybox.
    std::unique_ptr<Skybox> skybox;
    Shader* skybox_shader;
    // The elapsed time.
    float time_elapsed;
    // The shader for the each specialized pass.
    Shader* shadow_shader;
    Shader* depth_shader;
    Shader* render_tex_shader;
    Shader* extract_brightness_shader;
    Shader* reflect_shader;
    Shader* ssao_shader;
    Shader* blur_shader;
    Shader* hdr_shader;
    
    // Update the scene after given an elapsed amount of time.
    void update(float dt);

    // Give the scene an object to own.
    void give_object(Object* object);

    // Give the scene a landscape to own.
    void give_landscape(Landscape* landscape, Shader* shader);
    // Get the owned landscape.
    Landscape* get_landscape();

    // Give the scene a body of water to own.
    void give_water(Water* water, Shader* shader);
    // Get the owned landscape.
    Water* get_water();

    // Give the scene a skybox to own.
    void give_skybox(Skybox* skybox, Shader* shader);
    // Get the owned landscape.
    Skybox* get_skybox();

    void give_demo(Demo* demo);
    void give_sound(Sound* sound);
    
    glm::vec3 check_collisions(glm::vec3 current, glm::vec3 proposed);

private:
    // The meshes, stored as owning pointers hashed by name.
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
    // The owned shaders.
    std::unordered_map<std::string, std::unique_ptr<Shader>> owned_shaders;
    std::vector<std::unique_ptr<Object>> owned_objects;
    bool no_clip;
    bool camera_mode;
    int frame = 0;
    bool prev_state = false;

};

#endif // SCENE_H