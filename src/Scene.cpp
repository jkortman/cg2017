// Implementation of Scene class member functions.

#include "Scene.hpp"

#include <cstdio>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Scene::Scene()
{}

void Scene::update(float dt)
{
    for (auto& object : objects)
    {
        // Update the matrices for the object.
        object.get_model_matrix();
        object.get_normal_matrix();
    }
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

void Scene::give_shader(const std::string& name, Shader* shader) {
    owned_shaders[name] = std::unique_ptr<Shader>(shader);
    shaders.push_back(shader);
    
    glUseProgram(shader->program_id);
    assert(glGetUniformLocation(shader->program_id, "ProjectionMatrix") != -1);
}

void Scene::give_shader(Shader* shader) {
    give_shader(shader->name, shader);
}

Shader* Scene::get_shader(const std::string& name) {
    if (owned_shaders.find(name) == owned_shaders.end())
    {
        // 'name' not in meshes
        return nullptr;
    }
    return owned_shaders[name].get();
}


void Scene::inform_key(int key, int action)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

bool mouse_left_drag = false;
double last_xpos;   // The last x-position of the mouse.
double last_ypos;   // The last x-position of the mouse.
void Scene::inform_mouse_button(int button, int action)
{
    fprintf(stderr, "mouse button callback\n");
    // Update last position of mouse
    glfwGetCursorPos(window, &last_xpos, &last_ypos);
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        mouse_left_drag = (action == GLFW_PRESS);
    }
}

void Scene::inform_mouse_motion(double xpos, double ypos)
{
    if (mouse_left_drag)
    {
        double dx = xpos - last_xpos;
        double dy = ypos - last_ypos;
        last_xpos = xpos;
        last_ypos = ypos;
        objects[0].x_rotation += 0.5 * dy;
        objects[0].y_rotation += 0.5 * dx;
    }
}


