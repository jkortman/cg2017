// Shader class
// Provides tools for loading shader programs.

#ifndef SHADER_H
#define SHADER_H

#include "core.hpp"

class Shader
{
public:
    Shader();
    Shader(
        const std::string& vertex_file_path,
        const std::string& fragment_file_path);
    void load(
        const std::string& vertex_file_path,
        const std::string& fragment_file_path);
    bool exists(const std::string& uniform);
    void assert_existence(const std::string& uniform);
    ShaderID program_id;
};

#endif // SHADER_H