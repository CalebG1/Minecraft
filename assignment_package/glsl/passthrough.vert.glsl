#version 150
//Added
uniform int u_Time;

in vec2 vs_UV;
in vec4 vs_Pos;

out vec2 fs_UV;

void main() {
    fs_UV = vs_UV;
    gl_Position = vs_Pos;
}
