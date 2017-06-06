// Implementation of Scene class member functions.

#include "Scene.hpp"

#include <iostream>
#include <cstdio>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/gtx/rotate_vector.hpp>
#include <stdexcept>

#include "Console.hpp"

Scene::Scene()
    : world_light_night_index(-1),
      time_elapsed(0.0f)
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
    float move_speed;
    if (InputHandler::keys[GLFW_KEY_LEFT_ALT])
    {
        move_speed = 0.6f;
    }
    else 
    {
        move_speed = 0.3f;
    }
    glm::vec3 right_direction = glm::cross(player.direction, AXIS_Y);

    glm::vec3 movement = player.position;
    glm::vec3 dir = glm::vec3(0.0,0.0,0.0);

    if (InputHandler::keys[GLFW_KEY_W])
    {
        movement.x += move_speed * player.direction.x;
        //player.position.y += move_speed * player.direction.y;
        movement.z += move_speed * player.direction.z;
        dir.x += player.direction.x;
        dir.z += player.direction.z;
    }
    if (InputHandler::keys[GLFW_KEY_S])
    {
        movement.x -= move_speed * player.direction.x;
        //player.position.y -= move_speed * player.direction.y;
        movement.z -= move_speed * player.direction.z;
        dir.x -= player.direction.x;
        dir.z -= player.direction.z;
    }
    if (InputHandler::keys[GLFW_KEY_A])
    {
        movement.x -= move_speed * right_direction.x;
        //player.position.y -= move_speed * right_direction.y;
        movement.z -= move_speed * right_direction.z;
        dir.x -= right_direction.x;
        dir.z -= right_direction.z;
    }
    if (InputHandler::keys[GLFW_KEY_D])
    {
        movement.x += move_speed * right_direction.x;
        //player.position.y += move_speed * right_direction.y;
        movement.z += move_speed * right_direction.z;
        dir.x += right_direction.x;
        dir.z += right_direction.z;
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
    if (!no_clip) 
    {

        if (length(movement - player.position) > 0)
        {
            movement.y = landscape->get_pos_at(movement).y;
            //movement.y = landscape->get_height_at(movement.x, movement.z);
            movement = player.position + move_speed*normalize(movement-player.position);

        }
        
    }

    // TO TRY: Use Bezier curves
    // > Compare proposed point to terrain point and find proportion of distance to proposed point from currrent
    // 


    /*movement.y +=
        (landscape->get_pos_at(movement)).y
        - (landscape->get_pos_at(player.position)).y;*/
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
    
    
    if (demo->demo_mode)
    {
        node cam_pos = demo->update_pos(dt);
    
        camera.position = cam_pos.pos;
        camera.direction = cam_pos.dir;
    }
    else
    {
        // Match camera to player.
        camera.position = player.position;
        camera.direction = player.direction;
    }


    // Update skybox position.
    skybox->update_model_matrix(camera.position);
    //skybox->update_model_matrix(glm::vec3(0.0, 30.0, 0.0));

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

    if (world_light_night_index != -1)
    {
        lights[world_light_night_index].position.x = -daylight_dir.x;
        lights[world_light_night_index].position.y = -daylight_dir.y;
        lights[world_light_night_index].position.z = -daylight_dir.z;
    }

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


void Scene::give_demo(Demo* demo)
{
    this->demo = std::unique_ptr<Demo>(demo);

    console->register_var(
        "demo",
        Bool,
        &demo->demo_mode,
        1,
        "Toggles if demo is run");
    
}



glm::vec3 Scene::check_collisions(glm::vec3 current, glm::vec3 proposed)
{
    if (no_clip) return proposed;
    float radius = 2;
    
    /*glm::vec3 prop = current + proposed;
    prop.y += get_pos_at(prop)-current.y;
*/


    // Check terrain
    if ( current.y <(landscape->get_pos_at(proposed)).y + player.height ) 
    {
  /*      proposed.y = landscape->get_pos_at(proposed).y
        proposed = current + move_speed*normalize(proposed-current);
*/
        //current.y *= 1.05;
        std::cout << "STUCK!" << "\n";
        current.y = (landscape->get_pos_at(proposed)).y + player.height;
        return current;
    }
    else if (current.y == (landscape->get_pos_at(proposed)).y + player.height )
    {

    }

    // Check objects
    for (auto& object : objects)
    {
        if ( glm::length(object->position - glm::vec4(proposed, 0.0f)) < radius) return current;
    }
    return proposed;
}