#version 330

//in vec2 TexCoord;

out vec4 FragColour;

//uniform sampler2D SceneMap;

void main()
{
    //FragColour = texture(SceneMap, TexCoord);
    FragColour = vec4(1.0, 0.0, 0.0, 1.0);
}