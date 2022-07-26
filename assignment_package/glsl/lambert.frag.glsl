#version 330
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform sampler2D textureSampler;
uniform int u_Time;
uniform ivec2 u_ScreenDimensions;

uniform vec3 u_PlayerPos;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;
in vec2 fs_UVs;
in float fs_Flag;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

float mySmoothStep(float a, float b, float t) {
    t = smoothstep(0, 1, t);
    return mix(a, b, t);
}

float cubicTriMix(vec3 p) {
    vec3 pFract = fract(p);
    float llb = random1(floor(p) + vec3(0,0,0));
    float lrb = random1(floor(p) + vec3(1,0,0));
    float ulb = random1(floor(p) + vec3(0,1,0));
    float urb = random1(floor(p) + vec3(1,1,0));

    float llf = random1(floor(p) + vec3(0,0,1));
    float lrf = random1(floor(p) + vec3(1,0,1));
    float ulf = random1(floor(p) + vec3(0,1,1));
    float urf = random1(floor(p) + vec3(1,1,1));

    float mixLoBack = mySmoothStep(llb, lrb, pFract.x);
    float mixHiBack = mySmoothStep(ulb, urb, pFract.x);
    float mixLoFront = mySmoothStep(llf, lrf, pFract.x);
    float mixHiFront = mySmoothStep(ulf, urf, pFract.x);

    float mixLo = mySmoothStep(mixLoBack, mixLoFront, pFract.z);
    float mixHi = mySmoothStep(mixHiBack, mixHiFront, pFract.z);

    return mySmoothStep(mixLo, mixHi, pFract.y);
}

float fbm(vec3 p) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = 0.0;
    for(int i = 0; i < 8; i++) {
        sum += cubicTriMix(p * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

void main()
{
    // Material base color (before shading)

        vec2 uv = fs_UVs;
        vec4 colorMod = vec4(1.f);

        if(fs_Flag == 1.0f){
            float textureOffset = fract(u_Time / 10000.f) * 0.0625;
            uv.x += textureOffset;
            uv.y -= textureOffset;
        } else if (fs_Flag == 2.0f) {
            colorMod = vec4(0.4f, 0.f, 0.f, 1.f);
        } else if (fs_Flag == 3.0f) {
            colorMod = vec4(0.9f, 0.2f, 0.2f, 1.f);
}

        vec4 diffuseColor = texture(textureSampler, uv);

        for (int i = 0; i < 4; i++) {
            diffuseColor[i] *= colorMod[i];
        }

//        vec4 diffuseColor = vec4(fract(u_Time / 10000.f), fs_UVs.y, 1.f, 1.f);
//        out_Col = vec4(fs_UVs, 1.0f, 1.0f); return;

//

        float alpha = diffuseColor.w;

        if (alpha == 0.f) {
            discard;
        }
//        diffuseColor = diffuseColor * (0.5 * fbm(fs_Pos.xyz) + 0.5);

        // Calculate the diffuse term for Lambert shading
        float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
        // Avoid negative lighting values
        diffuseTerm = clamp(diffuseTerm, 0, 1);

        float ambientTerm = 0.2;

        float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
                                                            //to simulate ambient lighting. This ensures that faces that are not
                                                            //lit by our point light are not completely black.

        // Compute final shaded color
//        out_Col = fs_Col
//        out_Col = diffuseColor;
//        vec3 texture_Col = texture(textureSampler, fs_UVs).xyz;
//        float FOG_DISANCE = 492.f;
        float FOG_DISANCE = 492.f;
        vec2 playerPos = vec2(250.f,10.f);
        float l = length(playerPos / FOG_DISANCE);
        float fog_t = smoothstep(0.9,1.0,min(1.0,l));
//        float fog_t = smoothstep(0.9,1.0,FOG_DISTANCE);
        vec2 screenSpaceUVs = gl_FragCoord.xy / vec2(u_ScreenDimensions);
//        vec4 skyTextureColor = vec4(texture(u_skyTexture, screenSpaceUVs).rgb, 1.f);
//        diffuseColor = mix(diffuseColor,skyTextureColor, fog_t);
//        vec2 test = toPlayer.xz;
        diffuseColor = mix(diffuseColor, vec4(0.5,0.5,1,0), fog_t);
        out_Col = vec4(diffuseColor.xyz * lightIntensity, alpha);

//        out_Col = vec4(1.0f, 0, 0, 1.0f);
//        if (u_Time < 1000) {
//             out_Col = vec4(1,0,0,1);
//        }
////        else {
////             out_Col = vec4(0,0,1,1);
////        }
}
