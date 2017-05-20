// Implementation of InputHandler class member functions.

#include "InputHandler.hpp"

// Declaration of statics.
std::array<bool, 1024> InputHandler::keys; 
std::array<bool, 8>    InputHandler::mouse_buttons;
double                 InputHandler::mouse_xpos;
double                 InputHandler::mouse_ypos;
double                 InputHandler::mouse_xpos_prev;
double                 InputHandler::mouse_ypos_prev;
double                 InputHandler::mouse_dx;
double                 InputHandler::mouse_dy;

void InputHandler::initialize()
{
    warn_if(window == nullptr, "InputHandler initialized before GLFW window");
    glfwGetCursorPos(window, &mouse_xpos_prev, &mouse_ypos_prev);
}

void InputHandler::key_callback(
    GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024)
    {
        keys[key] = (action == GLFW_PRESS);
    }
}

void InputHandler::mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods)
{
    if (button >= 0 && button < 8)
    {
        mouse_buttons[button] = (action == GLFW_PRESS);
    }
}

void InputHandler::mouse_motion_callback(
    GLFWwindow* window, double xpos, double ypos)
{
    // Update mouse position variables.
    mouse_xpos      = xpos;
    mouse_ypos      = ypos;
    mouse_dx        = xpos - mouse_xpos_prev;
    mouse_dy        = ypos - mouse_ypos_prev;
    mouse_xpos_prev = xpos;
    mouse_ypos_prev = ypos;
}