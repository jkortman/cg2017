#version 330

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosDeviceSpace;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

uniform sampler2D DepthMap;

layout (location = 0) out vec3 FragColour;

// The positions of the samples to take for ambient occlusion.
// Each element in SSAOSamples is a 3D point relative to the fragment.
uniform vec3 SSAOSamples[64];
uniform int SSAONumSamples;

float discretize(float value)
{
    // We map a continuous value to one of N
    // fixed values between 0 and 1+W, where W is some factor to force white
    // highlights on the landscape.
    // For N = 5 this is equivalent to:
    //      if      (value < 0.25)      return 0.00;
    //      else if (value < 0.50)      return 0.33;
    //      else if (value < 0.75)      return 0.66;
    //      else if (value < 1.00)      return 1.00;
    //      else                        return 1.33;
    /*
    if      (value < 0.25)      return 0.00;
    else if (value < 0.50)      return 0.33;
    else if (value < 0.75)      return 0.66;
    else if (value < 1.00)      return 1.00;
    else                        return 1.33;
    */
    const float N = 5.0;
    return floor(N * value) / (N - 1.0);
}

float ambient_occlusion(vec3 pos)
{
    // Create the TBN matrix, which rotates the sample kernel up against
    // the normal vector.
    mat3 tbn = create_tbn(pos, Normal);

    float radius = 50.0;
    float bias = 0.001;
    float occlusion = 0.0;
    // The fragment position in view space.
    vec4 frag_view = ViewMatrix * vec4(pos, 1.0);
    for (int i = 0; i < SSAONumSamples; i += 1)
    {
        // Get view-space position of sample, relative to the fragment.
        vec3 sample = tbn * SSAOSamples[i];
        sample = sample * radius + frag_view.xyz;

        // Get the depth of the sample according to the depth map.
        vec4 proj = ProjectionMatrix * vec4(sample, 1.0);
        proj.xyz = 0.5 + 0.5 * proj.xyz / proj.w;
        float sample_depth = texture(DepthMap, proj.xy).r;
        
        float in_range = (abs(proj.z - sample_depth) < radius) ? 1.0 : 0.0;
        occlusion += 
            in_range * ((sample_depth <= proj.z) ? 1.0 : 0.0);
    }
    return 1.0 - occlusion / SSAONumSamples;
}

void main()
{
    float ssao = ambient_occlusion(FragPos);
    FragColour = vec3(ssao);
    //FragColour = vec3(gl_Position.z);
}

