//#version 330
#version 150
out vec4 color;

uniform sampler2D u_RenderedTexture;

void main()
{
    // vec4 c = texelFetch(u_RenderedTexture, gl_FragColor.xy, 0);
//    color = vec4(1, 0, 1, 1);

//    vec4 col = texelFetch(u_RenderedTexture, ivec2(gl_FragColor.xy), 0);

//    color = vec4(col.rgb * vec3(0.5, 0.5, 1), 1);
    vec4 col = texelFetch(u_RenderedTexture, ivec2(gl_FragCoord.xy), 0);

    color = vec4(col.rgb * vec3(1., 0.19, 0.14), 1);
}
