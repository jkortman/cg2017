// Core typedefs and constants

#ifndef CORE_H
#define CORE_H

#include <cstdio>
#include <glm/glm.hpp>
#include <string>

using ShaderID = unsigned int;
const ShaderID SHADER_NONE = -1;

const glm::vec3 AXIS_X = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 AXIS_Y = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 AXIS_Z = glm::vec3(0.0f, 0.0f, 1.0f);

FILE* logfile = stderr;

// -- Warning and error functions --

// Warn the user.
static void warn(const std::string& msg)
{
    std::fprintf(logfile, "Warning: %s\n", msg.c_str());
}

// Warn if a test fails.
static void warn_if(bool test, const std::string& msg)
{
    if (!test) warn(msg);
}

// Fatal error - cleanup and exit.
using fatal_cleanup_fn = void(*)();   // no arguments, void return value
static void fatal(const std::string& msg, fatal_cleanup_fn cleanup=nullptr)
{
    std::fprintf(logfile, "Error: %s\n", msg.c_str());
    std::exit(EXIT_FAILURE);
}

// Fatal error if a test fails.
static void fatal_if(bool test, std::string msg, fatal_cleanup_fn cleanup=nullptr)
{
    if (!test)
    {
        // Cleanup and exit if test fails.
        if (cleanup != nullptr) cleanup();
        fatal(msg, cleanup);
    }
}

#endif // CORE_H