// Shader class
// Provides tools for loading shader programs.

#ifndef SHADER_H
#define SHADER_H

#include <string>

#include "core.hpp"

class Shader
{
public:
    Shader();
    // Create a shader program from a vertex/fragment shader combination.
    Shader(
        const std::string& vertex_file_path,
        const std::string& fragment_file_path);
    // Load a shader program into an existing shader object.
    void load(
        const std::string& vertex_file_path,
        const std::string& fragment_file_path);
    // Check if a uniform exists in the shader.
    bool exists(const std::string& uniform);
    // Fail if a uniform does not exist.
    void assert_existence(const std::string& uniform);

    // The id of the shader program.
    ShaderID program_id;
    // The name of the shader program.
    // warning: this is not necessarily consistent with the name used by a Scene
    std::string name;
};

#endif // SHADER_H