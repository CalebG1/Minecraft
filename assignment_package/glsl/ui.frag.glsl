#version 330
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

uniform sampler2D textureSampler;

in vec4 fs_Pos;
in vec2 fs_UVs;

out vec4 out_Col;

void main()
{

    vec4 diffuseColor = texture(textureSampler, fs_UVs);

    // Copy the color; there is no shading.
    out_Col = diffuseColor;
}
