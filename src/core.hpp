// Core typedefs and constants

#ifndef CORE_H
#define CORE_H

#include <glm/glm.hpp>

using ShaderID = unsigned int;
const ShaderID SHADER_NONE = -1;

const glm::vec3 AXIS_X = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 AXIS_Y = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 AXIS_Z = glm::vec3(0.0f, 0.0f, 1.0f);

#endif // CORE_H