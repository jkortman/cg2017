// Object class
// A physical object in the world with some shape, position, orientation.

#ifndef OBJECT_H
#define OBJECT_H

#include <glm/glm.hpp>

#include "Entity.hpp"
#include "core.hpp"
#include "RenderUnit.hpp"

class Object: public Entity
{
public:
    Object() = delete;
    Object(
        Mesh* mesh,
        const glm::vec3& position,
        ShaderID program_id=SHADER_NONE);
    Object(
        Mesh* mesh,
        const glm::vec4& position,
        ShaderID program_id=SHADER_NONE);

    // Functions to update and retrieve data.
    const RenderUnit& get_render_unit();
    const glm::mat4&  update_model_matrix();
    const glm::mat3&  update_normal_matrix();
    const ShaderID    get_program_id();
    void              set_program_id(ShaderID program_id);

    // Data
    glm::vec4   position;
    glm::vec3   scale;
    float       x_rotation;
    float       y_rotation;
    float       z_rotation;
    RenderUnit  render_unit;
private:
    void initialize(Mesh* mesh, const glm::vec4& position, ShaderID program_id);
};

#endif