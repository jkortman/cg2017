// Implementation of Renderer class member functions.

#include "Renderer.hpp"

#include <array>
#include <cstdio>
#include <unordered_map>
#include <iostream>
#include <vector>

#include "Console.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Renderer::Renderer()
{}

// Callback for GLFW errors.
static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

// Setup the OpenGL environment and settings.
void Renderer::initialize(bool wf, unsigned int aa_samples)
{
    wireframe = wf;
    console->register_var("wf", Bool, &wireframe, 1, "the rendering mode (fill or wireframe)");

    glfwSetErrorCallback(error_callback);
    fatal_if(!glfwInit(), "Failed to initialise GLFW");

    // Load OpenGL 3.3 Core Profile.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, aa_samples);

    // Setup the window.
    window_width  = DEFAULT_WINDOW_WIDTH;
    window_height = DEFAULT_WINDOW_HEIGHT;
    window = glfwCreateWindow(
        window_width, window_height, "!", nullptr, nullptr);
    fatal_if(window == nullptr, "Failed to create window", glfwTerminate);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW.
    glewExperimental = true;
    fatal_if(glewInit() != GLEW_OK, "Failed to initialise GLEW");

    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Setup OpenGL.
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    // -----------------------------------------
    // -- FBO initialization for depth buffer --
    // -----------------------------------------
    glGenFramebuffers(1, &depth_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_buffer);

    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_2D, depth_texture);

    // Generate an empty image for OpenGL.
    glTexImage2D(
        GL_TEXTURE_2D,
        0, GL_DEPTH_COMPONENT32, depth_tex_size, depth_tex_size,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    #define WRAP_BEHAVIOUR GL_CLAMP_TO_EDGE // GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WRAP_BEHAVIOUR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WRAP_BEHAVIOUR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  

    // Attach depth_texture as depth attachment
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        depth_texture, 0);

    // Instruct openGL that we won't bind a color texture with the current FBO
    glDrawBuffer(GL_NONE); // default here would be GL_FRONT
    glReadBuffer(GL_NONE); // default here would be GL_BACK

    // Check that the framebuffer generated correctly
    GLenum fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    fatal_if(
        fb_status != GL_FRAMEBUFFER_COMPLETE,
        "Frame buffer error, status: " + std::to_string(fb_status));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Callback for window resize
void reshape(int width, int height) 
{
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    window_width = width;
    window_height = height;

    // TODO: Update projection matrix.
}

// Register InputHandler functions as callbacks.
void Renderer::set_callbacks()
{
    glfwSetKeyCallback              (window, InputHandler::key_callback);
    glfwSetCursorPosCallback        (window, InputHandler::mouse_motion_callback);
    glfwSetMouseButtonCallback      (window, InputHandler::mouse_button_callback);
    //glfwSetFramebufferSizeCallback  (window, reshape_callback);
}

void assign_generic_vao(
    unsigned int vao, // A generated VAO.
    const std::vector<float>        positions,
    const std::vector<float>        normals,
    const std::vector<float>        texcoords,
    const std::vector<unsigned int> indices)
{
    glBindVertexArray(vao);

    // Create buffers for positions, normals, texcoords, indices
    unsigned int buffer[4];
    glGenBuffers(4, buffer);

    // Set vertex position attribute
    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * positions.size(),
        positions.data(),
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, VALS_PER_VERT, GL_FLOAT, GL_FALSE, 0, 0);

    // Set normal attribute
    glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * normals.size(),
        normals.data(),
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, VALS_PER_NORMAL, GL_FLOAT, GL_FALSE, 0, 0);

    // Set texcoord attribute
    if (texcoords.size() != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(float) * texcoords.size(),
            texcoords.data(),
            GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, VALS_PER_TEX, GL_FLOAT, GL_FALSE, 0, 0);
    }

    // Set vertex indices attrib
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[3]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(float) * indices.size(),
        indices.data(),
        GL_STATIC_DRAW);
}

// Generate and assign a VAO to a landscape object.
Landscape* Renderer::assign_vao(Landscape* landscape)
{
    glGenVertexArrays(1, &landscape->vao);

    glBindVertexArray(landscape->vao);

    // Create buffers for positions, normals, texcoords, indices
    unsigned int buffer[4];
    glGenBuffers(4, buffer);

    // Set vertex position attribute
    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 3 * landscape->positions.size(),
        landscape->positions.data(),
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, VALS_PER_VERT, GL_FLOAT, GL_FALSE, 0, 0);

    // Set normal attribute
    glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 3 * landscape->normals.size(),
        landscape->normals.data(),
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, VALS_PER_NORMAL, GL_FLOAT, GL_FALSE, 0, 0);

    // Set colour attribute
    glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 3 * landscape->colours.size(),
        landscape->colours.data(),
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, VALS_PER_COLOUR, GL_FLOAT, GL_FALSE, 0, 0);\

    // Set vertex indices attrib
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[3]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(float) * landscape->indices.size(),
        landscape->indices.data(),
        GL_STATIC_DRAW);

    return landscape;
}

Water* Renderer::assign_vao(Water* water)
{
    glGenVertexArrays(1, &water->vao);

    glBindVertexArray(water->vao);

    // Create buffers for positions, normals, texcoords, indices
    unsigned int buffer[4];
    glGenBuffers(4, buffer);

    // Set vertex position attribute
    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 3 * water->positions.size(),
        &water->positions[0].x,
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, VALS_PER_VERT, GL_FLOAT, GL_FALSE, 0, 0);

    // Set normal attribute
    glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 3* water->normals.size(),
        &water->normals[0].x,
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, VALS_PER_NORMAL, GL_FLOAT, GL_FALSE, 0, 0);

    // Set colour attribute
    glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 3 * water->colours.size(),
        &water->colours[0].x,
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, VALS_PER_COLOUR, GL_FLOAT, GL_FALSE, 0, 0);

    // Set vertex indices attrib
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[3]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(float) * 3 * water->indices.size(),
        water->indices.data(),
        GL_STATIC_DRAW);

    return water;
}

Skybox* Renderer::assign_vao(Skybox* skybox)
{
    glGenVertexArrays(1, &skybox->vao);

    glBindVertexArray(skybox->vao);

    // Create buffers for positions, normals, indices
    unsigned int buffer[3];
    glGenBuffers(3, buffer);

    // Set vertex position attribute
    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 3 * skybox->positions.size(),
        &skybox->positions[0].x,
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, VALS_PER_VERT, GL_FLOAT, GL_FALSE, 0, 0);

    // Set normal attribute
    glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 3* skybox->normals.size(),
        &skybox->normals[0].x,
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, VALS_PER_NORMAL, GL_FLOAT, GL_FALSE, 0, 0);

    // Set vertex indices attrib
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[2]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(float) * 3 * skybox->indices.size(),
        skybox->indices.data(),
        GL_STATIC_DRAW);

    return skybox;
}

// Generate and assign a VAO to a mesh object.
Mesh* Renderer::assign_vao(Mesh* mesh)
{
    mesh->vaos.resize(mesh->num_shapes);
    glGenVertexArrays(mesh->num_shapes, mesh->vaos.data());

    // Create a VAO for each shape in the object.
    for (int i = 0; i < mesh->num_shapes; i += 1) {
        assign_generic_vao(
            mesh->vaos[i],
            mesh->shapes[i].mesh.positions,
            mesh->shapes[i].mesh.normals,
            mesh->shapes[i].mesh.texcoords,
            mesh->shapes[i].mesh.indices);
    }

    return mesh;
}

// Forward declare helper functions for material loading.
enum ImageFormat {JPEG, PNG, UNKNOWN};
static ImageFormat get_image_type(const std::string& path);
static void load_texture(const std::string& path);
static void load_texture_skybox(std::vector<std::string> paths);

// Read and load mesh textures onto the GPU.
Mesh* Renderer::create_materials(Mesh* mesh)
{
    glActiveTexture(GL_TEXTURE0);

    // Maps filenames of already loaded textures to texture IDs.
    std::unordered_map<std::string, unsigned int> loaded_textures;

    for (auto& shape : mesh->shapes) {
        // Get the material ID for this shape.
        const size_t face = 0;  // The face to check for material ID.
                                // Assuming that shapes have constant material.
        int matID = shape.mesh.material_ids[face];

        // Retrieve the texture name.
        std::string texname = mesh->materials[matID].diffuse_texname;

        #ifdef REPORT_SHAPE_DETAILS
            std::fprintf(stderr, "Shape %s:\n", shape.name.c_str());
            std::fprintf(stderr, "  material:    %s\n",
                mesh->materials[matID].name.c_str());
            std::fprintf(stderr, "  material id: %d\n", matID);
            std::fprintf(stderr, "  tex:         %s\n", texname.c_str());
            std::fprintf(stderr, "  #vertices:   %lu\n",
                shape.mesh.positions.size());
            std::fprintf(stderr, "  #normals:    %lu\n",
                shape.mesh.normals.size());
            std::fprintf(stderr, "  #texcoords:  %lu\n",
                shape.mesh.texcoords.size());
            std::fprintf(stderr, "  #indices:    %lu\n",
                shape.mesh.indices.size());
            std::fprintf(stderr, "  shininess:   %f\n",
                mesh->materials[matID].shininess);
        #endif

        // Check if the texture is already loaded.
        GLuint texID = -1;
        if (loaded_textures.find(texname) == loaded_textures.end()) {
            // If not, load the texture onto the GPU.
            glGenTextures(1, &texID);
            glBindTexture(GL_TEXTURE_2D, texID);
            // Set texture wrap behaviour to repeat
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            load_texture(mesh->dir + texname);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
        } else {
            texID = loaded_textures[texname];
        }
        mesh->textureIDs.push_back(texID);
    }

    return mesh;
}

void Renderer::setup_skybox(Skybox* skybox)
{
    glActiveTexture(GL_TEXTURE0);

    std::vector<std::string> paths;
    paths.push_back("skybox/sky_right.jpg");
    paths.push_back("skybox/sky_left.jpg");
    paths.push_back("skybox/sky_top.jpg");
    paths.push_back("skybox/sky_bottom.jpg");
    paths.push_back("skybox/sky_back.jpg");
    paths.push_back("skybox/sky_front.jpg");

    

    
        
    // Check if the texture is already loaded.
    GLuint texID = -1;
    // If not, load the texture onto the GPU.
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    // Set texture wrap behaviour to repeat
    load_texture_skybox(paths);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    //mesh->textureIDs.push_back(texID);
    
    skybox->texID =texID;
    //return mesh;
}



static void draw_object(const RenderUnit& ru, const unsigned int current_program)
{
    // Draw each shape in the object.
    for (int i = 0; i < ru.mesh->shapes.size(); i += 1) {
        auto& shape = ru.mesh->shapes[i];
        int matID = shape.mesh.material_ids[0];

        // Load the shape material properties into the shader.
        glUniform3fv(
            glGetUniformLocation(current_program, "MtlAmbient"),
            1, ru.mesh->materials[matID].ambient);
        glUniform3fv(
            glGetUniformLocation(current_program, "MtlDiffuse"),
            1, ru.mesh->materials[matID].diffuse);
        glUniform3fv(
            glGetUniformLocation(current_program, "MtlSpecular"),
            1, ru.mesh->materials[matID].specular);
        glUniform1f(
            glGetUniformLocation(current_program, "MtlShininess"),
            ru.mesh->materials[matID].shininess);

        // Load the shape material texture into the shader.
        GLuint texID = ru.mesh->textureIDs[i];
        glBindTexture(GL_TEXTURE_2D, texID);

        // Render the shape.
        glBindVertexArray(ru.mesh->vaos[i]);
        glDrawElements(
            GL_TRIANGLES,
            shape.mesh.indices.size(),
            GL_UNSIGNED_INT,
            0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Renderer::init_shader(
    const Scene& scene, Shader* shader, RenderMode render_mode)
{
    glUseProgram(shader->program_id);

    // Load projection matrix
    if (render_mode == RenderMode::Depth)
    {
        shader->assert_existence("LightSpaceMatrix");
        glm::mat4 ls = scene.camera.projection * scene.camera.view;
        glUniformMatrix4fv(
            glGetUniformLocation(shader->program_id, "LightSpaceMatrix"),
            1, false, glm::value_ptr(scene.world_light_day.light_space));
            //1, false, glm::value_ptr(ls));
    }
    else
    {
        shader->assert_existence("ProjectionMatrix");
        shader->assert_existence("ViewMatrix");
        glUniformMatrix4fv(
            glGetUniformLocation(shader->program_id, "ProjectionMatrix"),
            1, false, glm::value_ptr(scene.camera.projection));
        glUniformMatrix4fv(
            glGetUniformLocation(shader->program_id, "ViewMatrix"),
            1, false, glm::value_ptr(scene.camera.view));
        glUniformMatrix4fv(
            glGetUniformLocation(shader->program_id, "LightSpaceMatrix"),
            1, false, glm::value_ptr(scene.world_light_day.light_space));
    }

    // Load light sources.
    glUniform4fv(
        glGetUniformLocation(shader->program_id, "LightDay.position"),
        1, glm::value_ptr(scene.world_light_day.position));
    glUniform1f(
        glGetUniformLocation(shader->program_id, "LightDay.ambient"),
        scene.world_light_day.ambient);
    glUniform1f(
        glGetUniformLocation(shader->program_id, "LightDay.diffuse"),
        scene.world_light_day.diffuse);
    glUniform1f(
        glGetUniformLocation(shader->program_id, "LightDay.specular"),
        scene.world_light_day.specular);

    glUniform1i(
        glGetUniformLocation(shader->program_id, "NumLights"),
        int(scene.lights.size()));

    for (int i = 0; i < scene.lights.size(); i += 1)
    {
        const LightSource& ls = scene.lights[i];
        glUniform4fv(
            glGetUniformLocation(
                shader->program_id,
                ("Lights[" + std::to_string(i) + "].position").c_str()),
            1, glm::value_ptr(scene.lights[i].position));
        glUniform1f(
            glGetUniformLocation(
                shader->program_id,
                ("Lights[" + std::to_string(i) + "].ambient").c_str()),
            scene.lights[i].ambient);
        glUniform1f(
            glGetUniformLocation(
                shader->program_id,
                ("Lights[" + std::to_string(i) + "].diffuse").c_str()),
            scene.lights[i].diffuse);
        glUniform1f(
            glGetUniformLocation(
                shader->program_id,
                ("Lights[" + std::to_string(i) + "].specular").c_str()),
            scene.lights[i].specular);
        glUniform1f(
            glGetUniformLocation(
                shader->program_id,
                ("Lights[" + std::to_string(i) + "].K_constant").c_str()),
            scene.lights[i].K_constant);
        glUniform1f(
            glGetUniformLocation(
                shader->program_id,
                ("Lights[" + std::to_string(i) + "].K_linear").c_str()),
            scene.lights[i].K_linear);
        glUniform1f(
            glGetUniformLocation(
                shader->program_id,
                ("Lights[" + std::to_string(i) + "].K_quadratic").c_str()),
            scene.lights[i].K_quadratic);
        glUniform3fv(
            glGetUniformLocation(
                shader->program_id,
                ("Lights[" + std::to_string(i) + "].spot_direction").c_str()),
            1, glm::value_ptr(scene.lights[i].spot_direction));
        glUniform1f(
            glGetUniformLocation(
                shader->program_id,
                ("Lights[" + std::to_string(i) + "].spot_cos_angle").c_str()),
            glm::cos(glm::radians(scene.lights[i].spot_angle)));

    }

    // Load view position.
    glUniform3fv(
        glGetUniformLocation(shader->program_id, "ViewPos"),
        1, glm::value_ptr(scene.camera.position));
}

void Renderer::draw_scene(const Scene& scene, RenderMode render_mode)
{
    unsigned int current_program;

    // NOTE: Landscape and Water expect the single bound texture to be a depth map.
    // Binding another texture before those are rendered will break the lighting!

    if (render_mode == RenderMode::Scene)
    {
        reshape(window_width, window_height);

        // The default shadow state is no shadows when the sun is above
        // the horizon, and shadows when the sun is below the horizon.
        // This is important for below the horizon.
        // Warning: this seems a litte slow - it's likely glTexParameterfv()
        // are a little too performance-heavy to call per-frame.
        #if WRAP_BEHAVIOUR == GL_CLAMP_TO_BORDER
            glBindTexture(GL_TEXTURE_2D, depth_texture);
            std::array<float, 4> border_colour_night = {{ 1.0f, 0.0f, 0.0f, 1.0f }};
            std::array<float, 4> border_colour_day   = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
            if (glm::dot(glm::vec3(scene.world_light_day.position), AXIS_Y) >= 0.0f)
            {
                glTexParameterfv(
                GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_colour_day.data());
            }
            else
            {
                glTexParameterfv(
                GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_colour_night.data());
            }
        #endif
    }
    else if (render_mode == RenderMode::Depth)
    {
        current_program = scene.depth_shader->program_id;
        glUseProgram(current_program);
        init_shader(scene, scene.depth_shader, render_mode);
    }

    // Render the landscape.
    Landscape* landscape = scene.landscape.get();
    if (landscape != nullptr)
    {
        if (render_mode == RenderMode::Scene)
        {
            current_program = scene.landscape_shader->program_id;
            glUseProgram(current_program);
            init_shader(scene, scene.landscape_shader, render_mode);
        }

        glUniform1f(
            glGetUniformLocation(current_program, "Time"),
            scene.time_elapsed);

        // Load model and normal matrices.
        glUniformMatrix4fv(
            glGetUniformLocation(current_program, "ModelMatrix"),
            1, false, glm::value_ptr(landscape->model_matrix));
        glUniformMatrix3fv(
            glGetUniformLocation(current_program, "NormalMatrix"),
            1, false, glm::value_ptr(landscape->normal_matrix));
        // Load the shape material properties into the shader.
        glUniform3fv(
            glGetUniformLocation(current_program, "MtlAmbient"),
            1, glm::value_ptr(landscape->material.ambient));
        glUniform3fv(
            glGetUniformLocation(current_program, "MtlDiffuse"),
            1, glm::value_ptr(landscape->material.diffuse));
        glUniform3fv(
            glGetUniformLocation(current_program, "MtlSpecular"),
            1, glm::value_ptr(landscape->material.specular));
        glUniform1f(
            glGetUniformLocation(current_program, "MtlShininess"),
            landscape->material.shininess);

        glBindVertexArray(landscape->vao);
        glDrawElements(
            GL_TRIANGLES,
            landscape->indices.size(),
            GL_UNSIGNED_INT,
            0);
        glBindVertexArray(0);
    }

    // Render ocean.
    Water* water = scene.water.get();
    if (water != nullptr)
    {
        if (render_mode == RenderMode::Scene)
        {
            current_program = scene.water_shader->program_id;
            glUseProgram(current_program);
            init_shader(scene, scene.water_shader, render_mode);
        }

        // Load model and normal matrices.
        glUniformMatrix4fv(
            glGetUniformLocation(current_program, "ModelMatrix"),
            1, false, glm::value_ptr(water->model_matrix));
        glUniformMatrix3fv(
            glGetUniformLocation(current_program, "NormalMatrix"),
            1, false, glm::value_ptr(water->normal_matrix));
        // Load the shape material properties into the shader.
        glUniform1f(
            glGetUniformLocation(current_program, "MtlShininess"),
            water->material.shininess);
        // Load time elapsed into the shader.
        glUniform1f(
            glGetUniformLocation(current_program, "Time"),
            scene.time_elapsed);
        // Load the distance between vertices into the shader.
        glUniform1f(
            glGetUniformLocation(current_program, "VertDist"),
            water->vert_dist());

        glBindVertexArray(water->vao);
        glDrawElements(
            GL_TRIANGLES,
            3 * water->indices.size(),
            GL_UNSIGNED_INT,
            0);
        glBindVertexArray(0);
    }

    // Render skybox.
    // Skybox doesn't casr any shadows, so it's unnecessary when rendering
    // the depth map.
    if (render_mode == RenderMode::Scene)
    {
        // render skybox.
        Skybox* skybox = scene.skybox.get();
        if (skybox != nullptr)
        {
            current_program = scene.skybox_shader->program_id;
            glUseProgram(current_program);
            init_shader(scene, scene.skybox_shader, render_mode);

            glUniform1f(
                glGetUniformLocation(current_program, "Time"),
                scene.time_elapsed);

            // Load model and normal matrices.
            glUniformMatrix4fv(
                glGetUniformLocation(current_program, "ModelMatrix"),
                1, false, glm::value_ptr(skybox->model_matrix));

            glBindTexture(GL_TEXTURE_2D, skybox->texID);

            glBindVertexArray(skybox->vao);
            glDrawElements(
                GL_TRIANGLES,
                3 * skybox->indices.size(),
                GL_UNSIGNED_INT,
                0);
            glBindVertexArray(0);
        }
    }

    // Load and draw each object in the scene.
    for (const auto& object : scene.objects)
    {
        const RenderUnit& render_unit = object->render_unit;
        if (render_mode == RenderMode::Scene)
        {
            current_program = render_unit.program_id;
            glUseProgram(current_program);
            init_shader(scene, object->shader, render_mode);
            object->shader->set_palette(render_unit.mesh->palette);
        }

        // Load model and normal matrices.
        glUniformMatrix4fv(
            glGetUniformLocation(current_program, "ModelMatrix"),
            1, false, glm::value_ptr(render_unit.model_matrix));
        glUniformMatrix3fv(
            glGetUniformLocation(current_program, "NormalMatrix"),
            1, false, glm::value_ptr(render_unit.normal_matrix));

        // Render the object.
        draw_object(render_unit, current_program);
    }
}

// Render a scene.
void Renderer::render(const Scene& scene)
{
    // Initial setup.
    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else           glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // ----------------------------------
    // -- Pass 1: Render depth buffer. --
    // ----------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, depth_buffer);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, depth_tex_size, depth_tex_size);
    draw_scene(scene, RenderMode::Depth);

    GLenum fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    fatal_if(
        fb_status != GL_FRAMEBUFFER_COMPLETE,
        "Frame buffer error, status: " + std::to_string(fb_status));

    // ---------------------------
    // -- Pass 2: Render scene. --
    // ---------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.75f, 0.85f, 1.0f, 1.0f);   // Sky blue
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_width, window_height);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    draw_scene(scene, RenderMode::Scene);   
}

// Cleanup after a single render
void Renderer::postrender()
{
    glBindVertexArray(0);
    glfwSwapBuffers(window);
    glfwPollEvents();
    glFlush();
}

// Cleanup OpenGL environent.
void Renderer::cleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
// Check if the rendering process should end.
bool Renderer::should_end()
{
    return glfwWindowShouldClose(window);
}

// ----------------------
// -- Helper functions --
// ----------------------
// Get the format of an image (supported: PNG or JPEG).
static ImageFormat get_image_type(const std::string& path) {
    if (path.substr(path.length() - 4) == std::string(".jpg")
        || path.substr(path.length() - 5) == std::string(".jpeg"))
        return JPEG;
    if (path.substr(path.length() - 4) == std::string(".png"))
        return PNG;
    warn("Unknown image format for image '" + path + "'");
    return UNKNOWN;
}

// Load a texture from a given path to an image
static void load_texture(const std::string& path)
{
    ImageFormat image_fmt = get_image_type(path);
    int x, y, n;
    unsigned char* data = nullptr;
    
    switch (image_fmt) {
        case JPEG: data = stbi_load(path.c_str(), &x, &y, &n, 3); break;
        case PNG:  data = stbi_load(path.c_str(), &x, &y, &n, 4); break;
        default: break;
    }

    if (data == nullptr || image_fmt == UNKNOWN) {
        warn("No path to image '" + path  + "'");

        // A red texture to be used when no texture is provided.
        const std::array<unsigned char, 3> error_texture({{255, 0, 0}});

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB,
            GL_UNSIGNED_BYTE, &error_texture[0]);
        return;
    }

    switch (image_fmt) {
        case JPEG:
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB,
                GL_UNSIGNED_BYTE, data);
            break;
        case PNG:
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, data);
            break;
        default:
            fatal("Unknown image type");
    }
    stbi_image_free(data);
    glGenerateMipmap(GL_TEXTURE_2D); 
}

static void load_texture_skybox(std::vector<std::string> path)
{
    for (int i = 0; i < path.size(); i++)
    {
        ImageFormat image_fmt = get_image_type(path.at(i));
        int x, y, n;
        unsigned char* data = nullptr;
        
        switch (image_fmt) {
            case JPEG: data = stbi_load(path.at(i).c_str(), &x, &y, &n, 3); break;
            case PNG:  data = stbi_load(path.at(i).c_str(), &x, &y, &n, 4); break;
            default: break;
        }

        if (data == nullptr || image_fmt == UNKNOWN) {
            warn("No path to image '" + path.at(i)  + "'");

            // A red texture to be used when no texture is provided.
            const std::array<unsigned char, 3> error_texture({{255, 0, 0}});

            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGB, 1, 1, 0, GL_RGB,
                GL_UNSIGNED_BYTE, &error_texture[0]);
            return;
        }

        switch (image_fmt) {
            case JPEG:
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGB, x, y, 0, GL_RGB,
                    GL_UNSIGNED_BYTE, data);
                break;
            case PNG:
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA, x, y, 0, GL_RGBA,
                    GL_UNSIGNED_BYTE, data);
                break;
            default:
                fatal("Unknown image type");
        }
        stbi_image_free(data);
        //glGenerateMipmap(GL_TEXTURE_2D); 
    }
    
}