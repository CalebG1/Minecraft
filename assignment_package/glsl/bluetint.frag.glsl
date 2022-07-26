#version 330
out vec4 color;

uniform sampler2D u_RenderedTexture;
uniform int u_Time;


void main()
{
    // vec4 c = texelFetch(u_RenderedTexture, gl_FragColor.xy, 0);
    vec4 col = texelFetch(u_RenderedTexture, ivec2(gl_FragCoord.xy), 0);

    float holdTime = float(u_Time);
    float hold = sin(holdTime * 0.01);
//    if (u_Time == 0) {
//         color = vec4(col.rgb * vec3(1, 0, 0), 1);
//    } else {
//        color = vec4(col.rgb * vec3(0, 0, 1), 1);
//    }
//    color = vec4(col.rgb * vec3(0.5, 0.5, 1.), 1);
    color = vec4(col.rgb * vec3(0.5, 0.5, 1), 1);
}


// NEW

//#version 150
//uniform ivec2 u_Dimensions;
//uniform int u_Time;
//in vec2 fs_UV;
//out vec4 color;
//uniform sampler2D u_RenderedTexture;

//vec2 random2( vec2 p ) {
//    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
//}
//float random1( vec2 p ) {
//    return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);
//}

//vec3 getColorFromPixel(float i, float j)
//{
//    vec2 newPos = (gl_FragCoord.xy + vec2(i,j)) / u_Dimensions.xy ;
//    return vec3(texture(u_RenderedTexture, newPos));
//}

//void main()
//{
//    vec4 col = texelFetch(u_RenderedTexture, ivec2(gl_FragCoord.xy), 0);
//    color = vec4(col.rgb * vec3(0.5, 0.5, 1), 1);
//    float radians = random1(vec2(u_Time)) * 3.14159 / 180.0;
//    float xCords = cos(radians) * gl_FragCoord.x - sin(radians) * gl_FragCoord.y;
//    float yCords = sin(radians) * gl_FragCoord.x - cos(radians) * gl_FragCoord.y;
//    vec2 holdDimen = vec2(xCords,yCords);
//    vec3 horSpot11 = 2 * vec3(texture(u_RenderedTexture, (holdDimen + vec2(-1,-1)) / u_Dimensions.xy));
//    vec3 horSpot13 = -2 * vec3(texture(u_RenderedTexture, (holdDimen + vec2(1,-1)) / u_Dimensions.xy));
//    vec3 horSpot21 = 3 * vec3(texture(u_RenderedTexture, (holdDimen + vec2(-1,0)) / u_Dimensions.xy));
//    vec3 horSpot23 = -3 * vec3(texture(u_RenderedTexture, (holdDimen + vec2(1,0)) / u_Dimensions.xy));
//    vec3 horSpot31 = 2 * vec3(texture(u_RenderedTexture, (holdDimen + vec2(-1,1)) / u_Dimensions.xy));
//    vec3 horSpot33 = -2 * vec3(texture(u_RenderedTexture, (holdDimen + vec2(1,1)) / u_Dimensions.xy));
//    vec3 horAccum = horSpot11 + horSpot13 + horSpot21 + horSpot23 + horSpot31 + horSpot33;
//    horAccum = vec3(horAccum.r * horAccum.r, horAccum.g * horAccum.g, horAccum.b * horAccum.b);
//    vec3 vertSpot11 = 2 * vec3(texture(u_RenderedTexture, (holdDimen + vec2(-1,-1)) / u_Dimensions.xy));
//    vec3 vertSpot12 = 2 * vec3(texture(u_RenderedTexture, (holdDimen + vec2(0,-1)) / u_Dimensions.xy));
//    vec3 vertSpot13 = 3 * vec3(texture(u_RenderedTexture, (holdDimen + vec2(1,-1)) / u_Dimensions.xy));
//    vec3 vertSpot31 = -2 * vec3(texture(u_RenderedTexture, (holdDimen + vec2(-1,1)) / u_Dimensions.xy));
//    vec3 vertSpot32 = -2 * vec3(texture(u_RenderedTexture, (holdDimen + vec2(0,1)) / u_Dimensions.xy));
//    vec3 vertSpot33 = -3 * vec3(texture(u_RenderedTexture, (holdDimen + vec2(1,1)) / u_Dimensions.xy));
//    vec3 vertAccum = vertSpot11 + vertSpot12 + vertSpot13 + vertSpot31 + vertSpot32 + vertSpot33;
//    vertAccum = vec3(vertAccum.r * vertAccum.r, vertAccum.g * vertAccum.g, vertAccum.b * vertAccum.b);
//    vec3 combined = horAccum + vertAccum;
//    vec2 uvIn = fs_UV;
//    vec2 uv = uvIn * 15;
//    vec2 uvInt = floor(uv);
//    vec2 uvFract = fract(uv);
//    float minDist = 1.0;
//    for(int y = -1; y <= 1; y++) {
//        for(int x = -1; x <= 1; x++) {
//            vec2 neighbor = vec2(float(x), float(y));
//            vec2 point = random2(uvInt + neighbor);
//            point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point);
//            vec2 diff = neighbor + point - uvFract;
//            float dist = length(diff);
//            minDist = min(minDist, dist);
//        }
//    }
//    float h = 1 - minDist;

//    color = vec4(getColorFromPixel(0,0) * h + sqrt(combined),1);
//}

