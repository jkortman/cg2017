#version 330

in vec2 TexCoord;

uniform sampler2D Texture;

out vec4 FragColour;

void main() {
    vec4 texel = texture(Texture, vec2(TexCoord.x, 1.0 - TexCoord.y));
    if (texel.a < 0.5)  discard;
    else                FragColour = texel;
}