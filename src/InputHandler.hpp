// InputHandler class
// Provides static functions to be used as input callbacks,
// and informs objects of input when necessary.

#ifndef INPUTHANDLER_HPP
#define INPUTHANDLER_HPP

#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Scene.hpp"

class InputHandler
{
public:
    static void set_scene(Scene& scene_ref);
    static void key_callback(
        GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_button_callback(
        GLFWwindow* window, int button, int action, int mods);
    static void mouse_motion_callback(
        GLFWwindow* window, double xpos, double ypos);

    static Scene* scene;
};

#endif // INPUTHANDLER_HPP