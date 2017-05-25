#version 330

out float FragDepth;

float linearize(float z)
{
    const float near = 0.05;
    const float far = 1000.0;
    return (2.0 * near) / (far + near - z * (far - near));
}

void main() {
    FragDepth = 0.5;//gl_FragCoord.z;
}