// Authorship: James Kortman (a1648090)
// Implementation of Shader class member functions

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <random>

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "LoadShaders.hpp"

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

// Source: https://learnopengl.com/#!Advanced-Lighting/SSAO
float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

void Shader::set_ssao(int num_samples)
{
    // Generate num_ssao_samples samples, each a vec3 with each value
    // varying between -1.0 and 1.0.
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::default_random_engine gen;
    std::vector<glm::vec3> samples;

    for (int i = 0; i < num_samples; i += 1)
    {
        glm::vec3 sample_pos(
            -1.0f + 2.0f * dist(gen),
            -1.0f + 2.0f * dist(gen),
            //-1.0f + 2.0f * dist(gen));  // spherical
            dist(gen));                 // hemispherical
        sample_pos = glm::normalize(sample_pos);
        sample_pos *= dist(gen);
        float scale = float(i) / num_samples;
        scale = lerp(0.1f, 1.0f, scale*scale);
        sample_pos *= scale;

        samples.push_back(sample_pos);
    }

    // Load into shader.
    glUseProgram(program_id);
    //assert_existence("SSAONumSamples");
    glUniform1i(
        glGetUniformLocation(program_id, "SSAONumSamples"),
        num_samples);

    for (int i = 0; i < num_samples; i += 1)
    {
        // copy samples[i] into shader program.
        std::string uniform_name = "SSAOSamples[" + std::to_string(i) + "]";
        //assert_existence(uniform_name);
        glUniform3fv(
            glGetUniformLocation(program_id, uniform_name.c_str()),
            1, glm::value_ptr(samples[i]));
    }
}
