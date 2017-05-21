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
    //player.direction = glm::normalize(player.direction);

    camera.position = player.position;
    camera.direction = player.direction;
    camera.update_view();

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

void Scene::give_mesh(const std::string& name, Mesh* mesh)
{
    meshes[name] = std::unique_ptr<Mesh>(mesh);
}

Mesh* Scene::get_mesh(const std::string& name)
{
    if (meshes.find(name) == meshes.end())
    {
        // 'name' not in meshes
        throw std::runtime_error("Invalid mesh name '" + name + "'.");
    }
    return meshes[name].get();
}

void Scene::give_shader(const std::string& name, Shader* shader) {
    owned_shaders[name] = std::unique_ptr<Shader>(shader);
    shaders.push_back(shader);
    
    glUseProgram(shader->program_id);
    warn_if(
        glGetUniformLocation(shader->program_id, "ProjectionMatrix") == -1,
        "Scene given invalid shader with name '" + name + "'");
}

void Scene::give_shader(Shader* shader) {
    give_shader(shader->name, shader);
}

Shader* Scene::get_shader(const std::string& name) {
    if (owned_shaders.find(name) == owned_shaders.end())
    {
        // 'name' not in meshes
        throw std::runtime_error("Invalid shader name '" + name + "'.");
    }
    return owned_shaders[name].get();
}


