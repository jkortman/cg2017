// Implementation of InputHandler class member functions.

#include "InputHandler.hpp"
Scene* InputHandler::scene;

void InputHandler::set_scene(Scene& scene_ref)
{
    InputHandler::scene = &scene_ref;
}

void InputHandler::key_callback(
    GLFWwindow* window, int key, int scancode, int action, int mods)
{
    fprintf(stderr, "key callback\n");
    scene->inform_key(key, action);
}

void InputHandler::mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods)
{
    scene->inform_mouse_button(button, action);
}

void InputHandler::mouse_motion_callback(
    GLFWwindow* window, double xpos, double ypos)
{
    scene->inform_mouse_motion(xpos, ypos);
}