// Implementation of Scene class member functions.

#include "Scene.hpp"

#include <cstdio>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/gtx/rotate_vector.hpp>
#include <stdexcept>

Scene::Scene()
{}

void Scene::update(float dt)
{
    const float rotate_factor = 0.006f;
    const float move_speed = 0.13f;

    glm::vec3 right_direction = glm::cross(player.direction, AXIS_Y);

    if (InputHandler::keys[GLFW_KEY_W])
    {
        player.position.x += move_speed * player.direction.x;
        //player.position.y += move_speed * player.direction.y;
        player.position.z += move_speed * player.direction.z;
    }
    if (InputHandler::keys[GLFW_KEY_S])
    {
        player.position.x -= move_speed * player.direction.x;
        //player.position.y -= move_speed * player.direction.y;
        player.position.z -= move_speed * player.direction.z;
    }
    if (InputHandler::keys[GLFW_KEY_A])
    {
        player.position.x -= move_speed * right_direction.x;
        //player.position.y -= move_speed * right_direction.y;
        player.position.z -= move_speed * right_direction.z;
    }
    if (InputHandler::keys[GLFW_KEY_D])
    {
        player.position.x += move_speed * right_direction.x;
        //player.position.y += move_speed * right_direction.y;
        player.position.z += move_speed * right_direction.z;
    }
    if (InputHandler::keys[GLFW_KEY_SPACE])
    {
        player.position.y += move_speed;
    }
    if (InputHandler::keys[GLFW_KEY_LEFT_SHIFT])
    {
        player.position.y -= move_speed;
    }

    // Rotations
    player.direction = glm::rotate(
        player.direction,
        rotate_factor * float(-InputHandler::mouse_dx),
        AXIS_Y);
    player.direction = glm::rotate(
        player.direction,
        rotate_factor * float(-InputHandler::mouse_dy),
        right_direction);

    // Match camera to player.
    camera.position = player.position;
    camera.direction = player.direction;
    camera.update_view();

    // Rotate the day lighting.
    const float day_cycle_factor = 10.0f;
    auto daylight_dir = glm::vec3(world_light_day.position);
    daylight_dir = glm::rotate(
        daylight_dir,
        rotate_factor * float(dt * day_cycle_factor),
        AXIS_X),
    world_light_day.position.x = daylight_dir.x;
    world_light_day.position.y = daylight_dir.y;
    world_light_day.position.z = daylight_dir.z;

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
}

Landscape* Scene::get_landscape()
{
    return landscape.get();
}

