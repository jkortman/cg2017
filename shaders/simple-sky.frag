#version 330

in vec3 FragPos;

out vec4 FragColour;

void main() {
    // TODO: We probably want to be passing in the skybox size as a uniform eventually.
    FragColour = vec4(0.7, 0.7, 0.8 + FragPos.y / 1200.0, 1.0);
}