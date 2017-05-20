// Implementation of InputHandler class member functions.

#include "InputHandler.hpp"

// Declaration of statics.
std::array<bool, 1024> InputHandler::keys; 
std::array<bool, 8>    InputHandler::mouse_buttons;
double                 InputHandler::mouse_x;
double                 InputHandler::mouse_y;
double                 InputHandler::mouse_dx;
double                 InputHandler::mouse_dy;

void InputHandler::initialize()
{
    warn_if(window == nullptr, "InputHandler initialized before GLFW window");
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    mouse_dx = 0.0;
    mouse_dy = 0.0;
}

void InputHandler::update()
{
    double mouse_x_new, mouse_y_new;
    glfwGetCursorPos(window, &mouse_x_new, &mouse_y_new);
    mouse_dx = mouse_x_new - mouse_x;
    mouse_dy = mouse_y_new - mouse_y;
    mouse_x = mouse_x_new;
    mouse_y = mouse_y_new;
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
{}



