// Implementation of LightSource member functions.

#include "LightSource.hpp"

LightSource::LightSource(
    glm::vec4 position,
    glm::vec3 ambient,
    glm::vec3 diffuse,
    glm::vec3 specular)
    : position(position),
      ambient(ambient),
      diffuse(diffuse),
      specular(specular)
{}