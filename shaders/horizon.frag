#version 330
// Authorship: James Kortman (a1648090)

out vec4 FragColour;

void main() {
    // We want the horizon blocker to not be visible during the visible
    // rendering step (when this shader is used).
    discard;
}