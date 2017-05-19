// Implementation of Object class member functions.

#include <glm/gtc/matrix_transform.hpp>

#include "Object.hpp"

Object::Object(Mesh* mesh, const glm::vec3& position, ShaderID program_id)
{
    initialize(mesh, glm::vec4(position, 1.0), program_id);
}

Object::Object(Mesh* mesh, const glm::vec4& position, ShaderID program_id)
{
    initialize(mesh, position, program_id);
}

void Object::initialize(Mesh* mesh, const glm::vec4& position, ShaderID program_id)
{
    this->render_unit.mesh          = mesh;
    this->position                  = position;
    this->scale                     = glm::vec3(1.0, 1.0, 1.0);
    this->x_rotation                = 0.0f;
    this->y_rotation                = 0.0f;
    this->z_rotation                = 0.0f;
    this->render_unit.program_id    = program_id;
    this->render_unit               = get_render_unit();
}


// Functions to update and retrieve data.
const RenderUnit& Object::get_render_unit()
{
    render_unit.model_matrix  = get_model_matrix();
    render_unit.normal_matrix = get_normal_matrix();
    return render_unit;
}

const glm::mat4& Object::get_model_matrix()
{
    glm::mat4& model_matrix = render_unit.model_matrix;
    model_matrix = glm::mat4();
    model_matrix = glm::scale(model_matrix, scale);
    model_matrix = glm::rotate(model_matrix, x_rotation, AXIS_X);
    model_matrix = glm::rotate(model_matrix, y_rotation, AXIS_Y);
    model_matrix = glm::rotate(model_matrix, z_rotation, AXIS_Z);
    model_matrix = glm::translate(model_matrix, glm::vec3(position));
    return model_matrix;
}

const glm::mat3& Object::get_normal_matrix()
{
    render_unit.normal_matrix = glm::mat3(
        glm::transpose(glm::inverse(render_unit.model_matrix)));
    return render_unit.normal_matrix;
}

const ShaderID Object::get_program_id()
{
    return render_unit.program_id;
}

void Object::set_program_id(ShaderID program_id)
{
    render_unit.program_id = program_id;
}

