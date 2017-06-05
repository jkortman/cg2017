#version 330

layout (location = 0) in vec3 a_Position;

//out vec2 TexCoord;

void main()
{
    //TexCoord = a_Position.xy * 0.5 + 0.5;
    gl_Position = vec4(a_Position.x, a_Position.y, a_Position.z, 1.0);
}
