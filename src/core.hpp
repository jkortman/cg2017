// Core typedefs, constants, and global variables.

#ifndef CORE_H
#define CORE_H

#include <cstdio>
#include <cstdlib>
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

#ifndef MAIN_FILE
    #define EXTERN extern
#else
    #define EXTERN
#endif

// --------------------------
// -- Core type defintions --
// --------------------------
using ShaderID = unsigned int;

// ---------------------
// -- Constant values --
// ---------------------
const int       VALS_PER_VERT           = 3;
const int       VALS_PER_TEX            = 2;
const int       VALS_PER_NORMAL         = 3;
const ShaderID  SHADER_NONE             = -1;
const glm::vec3 AXIS_X                  = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 AXIS_Y                  = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 AXIS_Z                  = glm::vec3(0.0f, 0.0f, 1.0f);
const int       DEFAULT_WINDOW_HEIGHT   = 480;
const int       DEFAULT_WINDOW_WIDTH    = 640;
const float     DEFAULT_FOV             = 70.0f;
const float     DEFAULT_ASPECT          = float(DEFAULT_WINDOW_WIDTH)
                                            / DEFAULT_WINDOW_HEIGHT;
const float     DEFAULT_NEAR            = 0.05;
const float     DEFAULT_FAR             = 10.0;

// -------------
// -- Globals --
// -------------
// Window properties
// TODO: These can probably be refactored out to not be globals!
EXTERN GLFWwindow* window;
EXTERN int         window_width;
EXTERN int         window_height;

// ---------------------------------
// -- Warning and error functions --
// ---------------------------------
void warn(const std::string& msg);
void warn_if(bool test, const std::string& msg);
using fatal_cleanup_fn = void(*)();   // no arguments, void return value
void fatal(const std::string& msg, fatal_cleanup_fn cleanup=nullptr);
void fatal_if(bool test, std::string msg, fatal_cleanup_fn cleanup=nullptr);

#ifdef MAIN_FILE
// Warn the user.
void warn(const std::string& msg)
{
    std::fprintf(stderr, "Warning: %s\n", msg.c_str());
}

// Warn if a test fails.
void warn_if(bool test, const std::string& msg)
{
    if (!test) warn(msg);
}

// Fatal error - cleanup and exit.
void fatal(const std::string& msg, fatal_cleanup_fn cleanup)
{
    std::fprintf(stderr, "Error: %s\n", msg.c_str());
    std::exit(EXIT_FAILURE);
}

// Fatal error if a test fails.
void fatal_if(bool test, std::string msg, fatal_cleanup_fn cleanup)
{
    if (!test)
    {
        // Cleanup and exit if test fails.
        if (cleanup != nullptr) cleanup();
        fatal(msg, cleanup);
    }
}
#endif

#undef EXTERN
#endif // CORE_H