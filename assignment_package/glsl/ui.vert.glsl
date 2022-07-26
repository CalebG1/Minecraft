#version 330
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

//Added
uniform int u_Time;

in vec2 vs_Pos;

in vec2 vs_UVs;

out vec4 fs_Pos;
out vec2 fs_UVs;

void main()
{
    fs_Pos = vec4(vs_Pos.x, vs_Pos.y, 1.0f, 1.0f);
    fs_UVs = vs_UVs;
    //built-in things to pass down the pipeline
    gl_Position = vec4(vs_Pos.x, vs_Pos.y, 1.0f, 1.f);

}
