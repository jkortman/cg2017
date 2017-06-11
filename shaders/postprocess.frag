#version 330
// Authorship: James Kortman (a1648090)

in vec2 TexCoord;

out vec4 FragColour;

uniform sampler2D SceneMap;

void main()
{
    FragColour = texture(SceneMap, TexCoord);
}