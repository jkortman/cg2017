// Implementation of Scene class member functions.

#include "Scene.hpp"

#include <cstdio>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/gtx/rotate_vector.hpp>
#include <stdexcept>

#include "Console.hpp"

Scene::Scene()
    : time_elapsed(0.0f)
{
    no_clip = true;
    console->register_var(
        "player.pos",
        Float,
        &player.position[0],
        3,
        "The player position");
    console->register_var(
        "player.dir",
        Float,
        &player.direction[0],
        3,
        "The player direction");
    console->register_var(
        "noclip",
        Bool,
        &no_clip,
        1,
        "Toggles noclip");
}

void Scene::update(float dt)
{
    time_elapsed += dt;
    const float rotate_factor = 0.006f;
    const float move_speed = 0.13f;

    glm::vec3 right_direction = glm::cross(player.direction, AXIS_Y);


    glm::vec3 movement = player.position;

    if (InputHandler::keys[GLFW_KEY_W])
    {
        movement.x += move_speed * player.direction.x;
        //player.position.y += move_speed * player.direction.y;
        movement.z += move_speed * player.direction.z;
    }
    if (InputHandler::keys[GLFW_KEY_S])
    {
        movement.x -= move_speed * player.direction.x;
        //player.position.y -= move_speed * player.direction.y;
        movement.z -= move_speed * player.direction.z;
    }
    if (InputHandler::keys[GLFW_KEY_A])
    {
        movement.x -= move_speed * right_direction.x;
        //player.position.y -= move_speed * right_direction.y;
        movement.z -= move_speed * right_direction.z;
    }
    if (InputHandler::keys[GLFW_KEY_D])
    {
        movement.x += move_speed * right_direction.x;
        //player.position.y += move_speed * right_direction.y;
        movement.z += move_speed * right_direction.z;
    }
    if (InputHandler::keys[GLFW_KEY_SPACE])
    {
        movement.y += move_speed;
    }
    if (InputHandler::keys[GLFW_KEY_LEFT_SHIFT])
    {
        movement.y -= move_speed; 
    }

    // Toggle off for noclip / camera modes
    if (!no_clip) movement.y +=
        (landscape->get_pos_at(movement)).y
        - (landscape->get_pos_at(player.position)).y;
    player.position = check_collisions(player.position, movement);

    // Rotations
    player.direction = glm::rotate(
        player.direction,
        rotate_factor * float(-InputHandler::mouse_dx),
        AXIS_Y);
    player.direction = glm::rotate(
        player.direction,
        rotate_factor * float(-InputHandler::mouse_dy),
        right_direction);

    // Update skybox position.
    skybox->update_model_matrix(player.position);
    //skybox->update_model_matrix(glm::vec3(0.0, 30.0, 0.0));

    // Match camera to player.
    camera.position = player.position;
    camera.direction = player.direction;
    camera.update_view();

    // Rotate the day lighting.
    const float day_cycle_factor = 25.0f;
    auto daylight_dir = glm::vec3(world_light_day.position);
    daylight_dir = glm::rotate(
        daylight_dir,
        rotate_factor * float(dt * day_cycle_factor),
        AXIS_X);
    world_light_day.position.x = daylight_dir.x;
    world_light_day.position.y = daylight_dir.y;
    world_light_day.position.z = daylight_dir.z;
    world_light_day.update_view();

    // Update the model and normal matrices for each object.
    for (auto& object : objects)
    {
        object->update_model_matrix();
        object->update_normal_matrix();
    }
}

void Scene::give_object(Object* object)
{
    objects.push_back(object);
    owned_objects.push_back(std::unique_ptr<Object>(object));
}

void Scene::give_landscape(Landscape* landscape, Shader* shader)
{
    this->landscape.reset(landscape);
    this->landscape_shader = shader;

    // Give the Landscape's objects to the Scene.
    for (auto object: landscape->objects)
    {
        this->give_object(object);
    }

    // Register landscape variables.
    console->register_var(
        "landscape_shine",
        Float,
        &landscape->material.shininess,
        1, 
        "The shininess of the landscape");
}

Water* Scene::get_water()
{
    return water.get();
}

void Scene::give_water(Water* water, Shader* shader)
{
    this->water.reset(water);
    this->water_shader = shader;

    // Register landscape variables.
    console->register_var(
        "water_shine",
        Float,
        &water->material.shininess,
        1, 
        "The shininess of the ocean");
}

Skybox* Scene::get_skybox()
{
    return skybox.get();
}

void Scene::give_skybox(Skybox* skybox, Shader* shader)
{
    this->skybox.reset(skybox);
    this->skybox_shader = shader;
}

Landscape* Scene::get_landscape()
{
    return landscape.get();
}


glm::vec3 Scene::check_collisions(glm::vec3 current, glm::vec3 proposed)
{
    if (no_clip) return proposed;
    float radius = 2;
    // Check terrain
    if ( (landscape->get_pos_at(proposed)).y + player.height > current.y) 
    {
        current.y *= 1.05;
        return current;
    }    

    // Check objects
    for (auto& object : objects)
    {
        if ( glm::length(object->position - glm::vec4(proposed, 0.0f)) < radius) return current;
    }
    return proposed;
}