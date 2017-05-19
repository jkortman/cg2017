// Shader class
// Provides tools for loading shader programs.

#ifndef SHADER_H
#define SHADER_H

class Shader
{
public:
    Shader();
    Shader(
        const std::string& vertex_file_path,
        const std::string& fragment_file_path);
    void Shader::load(
        const std::string& vertex_file_path,
        const std::string& fragment_file_path)
    bool exists(const std::string& uniform);
    void assert(const std::string& uniform);
    ShaderID program_id;
};

#endif // SHADER_H