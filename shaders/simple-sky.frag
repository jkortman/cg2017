#version 330

in vec3 FragPos;

out vec4 FragColour;

void main() {
    FragColour = vec4((0.5 + FragPos.y / 1200.0), 0.0, 0.0, 1.0);
}