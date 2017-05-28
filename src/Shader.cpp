// Implementation of Shader class member functions

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"

// Forward declaration of shader loading function
GLuint LoadShaders(
    const char * vertex_file_path,
    const char * fragment_file_path);

Shader::Shader()
    : program_id(SHADER_NONE)
{}

Shader::Shader(
    const std::string& vertex_file_path,
    const std::string& fragment_file_path)
    : program_id(SHADER_NONE)
{
    load(vertex_file_path, fragment_file_path);
}

void Shader::load(
    const std::string& vertex_file_path,
    const std::string& fragment_file_path)
{
    std::string vertname = vertex_file_path.substr(0, vertex_file_path.length() - 5);
    std::string fragname = vertex_file_path.substr(0, fragment_file_path.length() - 5);
    if (vertname == fragname) name = vertname;
    else                      name = vertname + "|" + fragname;
    
    printf("Compiling shader %s...\n", name.c_str());
    program_id = LoadShaders(
        vertex_file_path.c_str(),
        fragment_file_path.c_str());
    
    fatal_if(
        program_id == -1,
        "Loading shaders '" + vertex_file_path
        + "'/'" + fragment_file_path + "' failed");
}

bool Shader::exists(const std::string& uniform)
{
    return (glGetUniformLocation(program_id, uniform.c_str()) != -1);
}

void Shader::assert_existence(const std::string& uniform)
{
    if (glGetUniformLocation(program_id, uniform.c_str()) == -1)
    {
        fatal("Uniform '" + uniform + "' does not exist");
    }
}

void Shader::set_palette(const std::vector<glm::vec3>& palette, int offset)
{
    glUseProgram(program_id);
    int palette_size = palette.size() - offset;
    glUniform1i(
        glGetUniformLocation(program_id, "PaletteSize"),
        palette_size);

    for (int i = 0; i < palette_size; i += 1)
    {
        // copy palette[i + offset] into shader program.
        std::string uniform_name = "Palette[" + std::to_string(i) + "]";
        glUniform3fv(
            glGetUniformLocation(program_id, uniform_name.c_str()),
            1, glm::value_ptr(palette[i + offset]));
    }
}

// TODO: Cite where this comes from!
int CompileShader(const char *ShaderPath, const GLuint ShaderID)
{
    // Read shader code from file
    std::string ShaderCode;
    std::ifstream ShaderStream (ShaderPath, std::ios::in);
    if (ShaderStream.is_open()) {
        std::string Line = "";
        while (getline(ShaderStream, Line)) {
            ShaderCode += "\n" + Line;
        }
        ShaderStream.close();
    }
    else {
        std::cerr << "Cannot open " << ShaderPath << ". Are you in the right directory?" << std::endl;
        return 0;
    }

    // Compile Shader
    char const *SourcePointer = ShaderCode.c_str();
    glShaderSource(ShaderID, 1, &SourcePointer , NULL);
    glCompileShader(ShaderID);

    // Check Shader
    GLint Result = GL_FALSE;
    int InfoLogLength;

    glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    printf("    compiled shader %d %d\n", Result, InfoLogLength);
    if ( InfoLogLength > 1 ) {
        char ShaderErrorMessage[InfoLogLength+1];
        glGetShaderInfoLog( ShaderID,
                            InfoLogLength,
                            NULL,
                            &ShaderErrorMessage[0]);
        std::cerr << &ShaderErrorMessage[0] << std::endl;
        return 0;
    }
    return 1;
}

GLuint LoadShaders(const char * vertex_file_path,
                   const char * fragment_file_path )
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Compile both shaders. Exit if compile errors.
    if ( !CompileShader(vertex_file_path, VertexShaderID)
         || !CompileShader(fragment_file_path, FragmentShaderID) ) {
        return 0;
    }
    
    // Link the program
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ) {
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        std::cerr << &ProgramErrorMessage[0] << std::endl;
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}


