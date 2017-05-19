// InputHandler class
// Provides static functions to be used as input callbacks,
// and informs objects of input when necessary.

#ifndef INPUTHANDLER_HPP
#define INPUTHANDLER_HPP

#include <array>
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Scene.hpp"

class InputHandler
{
public:
    static void initialize();
    static void key_callback(
        GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_button_callback(
        GLFWwindow* window, int button, int action, int mods);
    static void mouse_motion_callback(
        GLFWwindow* window, double xpos, double ypos);

    // Flags for keys and mouse buttons.
    static std::array<bool, 1024> keys; 
    static std::array<bool, 8>    mouse_buttons;
    // The current location of the mouse.
    static double                 mouse_xpos;
    static double                 mouse_ypos;
    // The previous location of the mouse (one cycle previously)
    static double                 mouse_xpos_prev;
    static double                 mouse_ypos_prev;
    // The change in position of the mouse.
    static double                 mouse_dx;
    static double                 mouse_dy;
};

#endif // INPUTHANDLER_HPP