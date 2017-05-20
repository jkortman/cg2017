// Implementation of Renderer class member functions.

#include "Renderer.hpp"

#include <array>
#include <cstdio>
#include <unordered_map>

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
void Renderer::initialize()
{
    glfwSetErrorCallback(error_callback);
    fatal_if(!glfwInit(), "Failed to initialise GLFW");

    // Load OpenGL 3.3 Core Profile.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    // Setup OpenGL.
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

// Callback for window resize
void reshape_callback(GLFWwindow *window, int x, int y) 
{
    window_width = x;
    window_height = y;
    glViewport(0, 0, x, y);
}

// Register InputHandler functions as callbacks.
void Renderer::set_callbacks()
{
    glfwSetKeyCallback              (window, InputHandler::key_callback);
    glfwSetCursorPosCallback        (window, InputHandler::mouse_motion_callback);
    glfwSetMouseButtonCallback      (window, InputHandler::mouse_button_callback);
    glfwSetFramebufferSizeCallback  (window, reshape_callback);
}

// Generate and assign a VAO to a mesh object.
bool Renderer::assign_vao(Mesh* mesh)
{
    mesh->vaos.resize(mesh->num_shapes);
    glGenVertexArrays(mesh->num_shapes, mesh->vaos.data());

    // Create a VAO for each shape in the object.
    for (int i = 0; i < mesh->num_shapes; i += 1) {
        glBindVertexArray(mesh->vaos[i]);

        // Create buffers for positions, normals, texcoords, indices
        unsigned int buffer[4];
        glGenBuffers(4, buffer);

        // Set vertex position attribute
        glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(float) * mesh->shapes[i].mesh.positions.size(),
            mesh->shapes[i].mesh.positions.data(),
            GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, VALS_PER_VERT, GL_FLOAT, GL_FALSE, 0, 0);

        // Set normal attribute
        glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(float) * mesh->shapes[i].mesh.normals.size(),
            mesh->shapes[i].mesh.normals.data(),
            GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, VALS_PER_NORMAL, GL_FLOAT, GL_FALSE, 0, 0);

        // Set texcoord attribute
        if (mesh->shapes[i].mesh.texcoords.size() != 0) {
            glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
            glBufferData(
                GL_ARRAY_BUFFER,
                sizeof(float) * mesh->shapes[i].mesh.texcoords.size(),
                mesh->shapes[i].mesh.texcoords.data(),
                GL_STATIC_DRAW);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, VALS_PER_TEX, GL_FLOAT, GL_FALSE, 0, 0);
        }

        // Set vertex indices attrib
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[3]);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            sizeof(float) * mesh->shapes[i].mesh.indices.size(),
            mesh->shapes[i].mesh.indices.data(),
            GL_STATIC_DRAW);
    }

    return true;
}

// Forward declare helper functions for material loading.
enum ImageFormat {JPEG, PNG, UNKNOWN};
static ImageFormat get_image_type(const std::string& path);
static void load_texture(const std::string& path);

// Read and load mesh textures onto the GPU.
bool Renderer::create_materials(Mesh* mesh)
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

    return 0;
}

static void draw_object(const RenderUnit& ru)
{
    // Draw each shape in the object.
    for (int i = 0; i < ru.mesh->shapes.size(); i += 1) {
        auto& shape = ru.mesh->shapes[i];
        int matID = shape.mesh.material_ids[0];

        // Load the shape material properties into the shader.
        glUniform3fv(
            glGetUniformLocation(ru.program_id, "MtlAmbient"),
            1, ru.mesh->materials[matID].ambient);
        glUniform3fv(
            glGetUniformLocation(ru.program_id, "MtlDiffuse"),
            1, ru.mesh->materials[matID].diffuse);
        glUniform3fv(
            glGetUniformLocation(ru.program_id, "MtlSpecular"),
            1, ru.mesh->materials[matID].specular);
        glUniform1f(
            glGetUniformLocation(ru.program_id, "MtlShininess"),
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

// Render a scene.
void Renderer::render(const Scene& scene)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Load uniforms that are constant for every object into each shader program.
    for (const auto& shader : scene.shaders)
    {
        glUseProgram(shader->program_id);

        // Load projection matrix
        // TODO - this doesn't need to happen every time!
        shader->assert_existence("ProjectionMatrix");
        glUniformMatrix4fv(
            glGetUniformLocation(shader->program_id, "ProjectionMatrix"),
            1, false, glm::value_ptr(scene.camera.projection));

        // Load view matrix
        shader->assert_existence("ViewMatrix");
        glUniformMatrix4fv(
            glGetUniformLocation(shader->program_id, "ViewMatrix"),
            1, false, glm::value_ptr(scene.camera.view));

        // Load light sources.
        // TODO

        // Load view position.
        glUniform4fv(
            glGetUniformLocation(shader->program_id, "ViewPos"),
            1, glm::value_ptr(scene.camera.position));
    }

    // Load and draw each object in the scene.
    for (const auto& object : scene.objects)
    {
        const RenderUnit& render_unit = object.render_unit;
        glUseProgram(render_unit.program_id);

        // Load model and normal matrices.
        glUniformMatrix4fv(
            glGetUniformLocation(render_unit.program_id, "ModelMatrix"),
            1, false, glm::value_ptr(render_unit.model_matrix));
        glUniformMatrix3fv(
            glGetUniformLocation(render_unit.program_id, "NormalMatrix"),
            1, false, glm::value_ptr(render_unit.normal_matrix));

        // Render the object.
        draw_object(render_unit);
    }
}

// Cleanup after a single render
void Renderer::postrender()
{
    glfwSwapBuffers(window);
    glfwPollEvents();
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
        const std::array<unsigned char, 3> error_texture({255, 0, 0});

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