#version 330

in vec4 Colour;
in vec3 Normal;

out vec4 FragColour;

void main() {
    FragColour = Colour;
}