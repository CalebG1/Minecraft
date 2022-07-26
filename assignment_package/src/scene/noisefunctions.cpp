///
/// Noise Functions
///
/// Perlin is based on Perlin Simplex noise
///   with testing this was by far the quickest compared to
///   lecture notes code and other online Perlin code
/// Both Perlin and Fractal Perlin based on
///   https://github.com/SRombauts/SimplexNoise
///
/// Worley and FBM based on lecture notes
///

#include "noisefunctions.h"

#include <cstdint>  // int32_t/uint8_t


static inline int32_t fastfloor(float fp) {
    int32_t i = static_cast<int32_t>(fp);
    return (fp < i) ? (i - 1) : (i);
}

static const uint8_t perm[256] = {
    151, 160, 137, 91, 90, 15,
    131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
    190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
    88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
    77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
    102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
    135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
    5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
    223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
    251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
    49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
    138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};


static inline uint8_t hash(int32_t i) {
    return perm[static_cast<uint8_t>(i)];
}

static float grad(int32_t hash, float x) {
    const int32_t h = hash & 0x0F;  // Convert low 4 bits of hash code
    float grad = 1.0f + (h & 7);    // Gradient value 1.0, 2.0, ..., 8.0
    if ((h & 8) != 0) grad = -grad; // Set a random sign for the gradient
//  float grad = gradients1D[h];    // NOTE : Test of Gradient look-up table instead of the above
    return (grad * x);              // Multiply the gradient with the distance
}

static float grad(int32_t hash, float x, float y) {
    const int32_t h = hash & 0x3F;  // Convert low 3 bits of hash code
    const float u = h < 4 ? x : y;  // into 8 simple gradient directions,
    const float v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v); // and compute the dot product with (x,y).
}

static float grad(int32_t hash, float x, float y, float z) {
    int h = hash & 15;     // Convert low 4 bits of hash code into 12 simple
    float u = h < 8 ? x : y; // gradient directions, and compute dot product.
    float v = h < 4 ? y : h == 12 || h == 14 ? x : z; // Fix repeats at h = 12 to 15
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}


float NoiseFunctions::Perlin(float x) {
    float n0, n1;   // Noise contributions from the two "corners"

    // No need to skew the input space in 1D

    // Corners coordinates (nearest integer values):
    int32_t i0 = fastfloor(x);
    int32_t i1 = i0 + 1;
    // Distances to corners (between 0 and 1):
    float x0 = x - i0;
    float x1 = x0 - 1.0f;

    // Calculate the contribution from the first corner
    float t0 = 1.0f - x0*x0;
//  if(t0 < 0.0f) t0 = 0.0f; // not possible
    t0 *= t0;
    n0 = t0 * t0 * grad(hash(i0), x0);

    // Calculate the contribution from the second corner
    float t1 = 1.0f - x1*x1;
//  if(t1 < 0.0f) t1 = 0.0f; // not possible
    t1 *= t1;
    n1 = t1 * t1 * grad(hash(i1), x1);

    // The maximum value of this noise is 8*(3/4)^4 = 2.53125
    // A factor of 0.395 scales to fit exactly within [-1,1]
    return 0.395f * (n0 + n1);
}

float NoiseFunctions::Perlin(float x, float y) {
    float n0, n1, n2;   // Noise contributions from the three corners

    // Skewing/Unskewing factors for 2D
    static const float F2 = 0.366025403f;  // F2 = (sqrt(3) - 1) / 2
    static const float G2 = 0.211324865f;  // G2 = (3 - sqrt(3)) / 6   = F2 / (1 + 2 * K)

    // Skew the input space to determine which simplex cell we're in
    const float s = (x + y) * F2;  // Hairy factor for 2D
    const float xs = x + s;
    const float ys = y + s;
    const int32_t i = fastfloor(xs);
    const int32_t j = fastfloor(ys);

    // Unskew the cell origin back to (x,y) space
    const float t = static_cast<float>(i + j) * G2;
    const float X0 = i - t;
    const float Y0 = j - t;
    const float x0 = x - X0;  // The x,y distances from the cell origin
    const float y0 = y - Y0;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int32_t i1, j1;  // Offsets for second (middle) corner of simplex in (i,j) coords
    if (x0 > y0) {   // lower triangle, XY order: (0,0)->(1,0)->(1,1)
        i1 = 1;
        j1 = 0;
    } else {   // upper triangle, YX order: (0,0)->(0,1)->(1,1)
        i1 = 0;
        j1 = 1;
    }

    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6

    const float x1 = x0 - i1 + G2;            // Offsets for middle corner in (x,y) unskewed coords
    const float y1 = y0 - j1 + G2;
    const float x2 = x0 - 1.0f + 2.0f * G2;   // Offsets for last corner in (x,y) unskewed coords
    const float y2 = y0 - 1.0f + 2.0f * G2;

    // Work out the hashed gradient indices of the three simplex corners
    const int gi0 = hash(i + hash(j));
    const int gi1 = hash(i + i1 + hash(j + j1));
    const int gi2 = hash(i + 1 + hash(j + 1));

    // Calculate the contribution from the first corner
    float t0 = 0.5f - x0*x0 - y0*y0;
    if (t0 < 0.0f) {
        n0 = 0.0f;
    } else {
        t0 *= t0;
        n0 = t0 * t0 * grad(gi0, x0, y0);
    }

    // Calculate the contribution from the second corner
    float t1 = 0.5f - x1*x1 - y1*y1;
    if (t1 < 0.0f) {
        n1 = 0.0f;
    } else {
        t1 *= t1;
        n1 = t1 * t1 * grad(gi1, x1, y1);
    }

    // Calculate the contribution from the third corner
    float t2 = 0.5f - x2*x2 - y2*y2;
    if (t2 < 0.0f) {
        n2 = 0.0f;
    } else {
        t2 *= t2;
        n2 = t2 * t2 * grad(gi2, x2, y2);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 45.23065f * (n0 + n1 + n2);
}

glm::vec3 random3(glm::vec3 p) {
//    float x = glm::sin(glm::dot(p,glm::vec3(127.1, 311.7, 152.35))) * 43758.5453;
//    float y = glm::sin(glm::dot(p,glm::vec3(269.5, 183.3, 163.94))) * 43758.5453;
//    float z =  glm::sin(glm::dot(p,glm::vec3(420.6, 631.2, 203.25))) * 43758.5453;
    float x = glm::sin(glm::dot(p,glm::vec3(127.1, 311.7, 365))) * 43758.5453;
    float y = glm::sin(glm::dot(p,glm::vec3(269.5, 183.3, 150))) * 43758.5453;
    float z =  glm::sin(glm::dot(p,glm::vec3(420.6, 631.2, 132.25))) * 43758.5453;
    return glm::fract(glm::vec3(x,y,z));
}

float NoiseFunctions::surflet(glm::vec3 p, glm::vec3 gridPoint)
{
    // Compute the distance between p and the grid point along each axis, and warp it with a
    // quintic function so we can smooth our cells
    glm::vec3 t2 = glm::abs(p - gridPoint);
    glm::vec3 t = glm::vec3(1.f - 6.f * pow(t2.x, 5.f) + 15.f * pow(t2.x, 4.f) - 10.f * pow(t2.x, 3.f),
                             1.f - 6.f * pow(t2.x, 5.f) + 15.f * pow(t2.x, 4.f) - 10.f * pow(t2.x, 3.f),
                             1.f - 6.f * pow(t2.x, 5.f) + 15.f * pow(t2.x, 4.f) - 10.f * pow(t2.x, 3.f));
    // Get the random vector for the grid point (assume we wrote a function random2
    // that returns a vec2 in the range [0, 1])
    glm::vec3 grad = random3(gridPoint);
    glm::vec3 gradient = glm::vec3(grad.x * 2, grad.y * 2, grad.z * 2) - glm::vec3(1., 1., 1.);
    // ADDED
    gradient = glm::normalize(gradient);
    // Get the vector from the grid point to P
    glm::vec3 diff = p - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = glm::dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * t.x * t.y * t.z;
}

float NoiseFunctions::classPerlinNoise3D(float x, float y, float z) {
//    x/= 4;
//    y/=6;
//    z/=10;
    float surfletSum = 0.f;
    // Iterate over the four integer corners surrounding uv
    for(int dx = 0; dx <= 1; ++dx) {
        for(int dy = 0; dy <= 1; ++dy) {
            for(int dz = 0; dz <= 1; ++dz) {
                surfletSum += surflet(glm::vec3(x,y,z), glm::floor(glm::vec3(x,y,z)) + glm::vec3(dx, dy, dz));
            }
        }
    }
    return (surfletSum * 2) + 0.5;
}


float NoiseFunctions::Perlin(float x, float y, float z) {
    float n0, n1, n2, n3; // Noise contributions from the four corners

    // Skewing/Unskewing factors for 3D
    static const float F3 = 1.0f / 3.0f;
    static const float G3 = 1.0f / 6.0f;

    // Skew the input space to determine which simplex cell we're in
    float s = (x + y + z) * F3; // Very nice and simple skew factor for 3D
    int i = fastfloor(x + s);
    int j = fastfloor(y + s);
    int k = fastfloor(z + s);
    float t = (i + j + k) * G3;
    float X0 = i - t; // Unskew the cell origin back to (x,y,z) space
    float Y0 = j - t;
    float Z0 = k - t;
    float x0 = x - X0; // The x,y,z distances from the cell origin
    float y0 = y - Y0;
    float z0 = z - Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
    if (x0 >= y0) {
        if (y0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; // X Y Z order
        } else if (x0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; // X Z Y order
        } else {
            i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; // Z X Y order
        }
    } else { // x0<y0
        if (y0 < z0) {
            i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; // Z Y X order
        } else if (x0 < z0) {
            i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; // Y Z X order
        } else {
            i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; // Y X Z order
        }
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    float y1 = y0 - j1 + G3;
    float z1 = z0 - k1 + G3;
    float x2 = x0 - i2 + 2.0f * G3; // Offsets for third corner in (x,y,z) coords
    float y2 = y0 - j2 + 2.0f * G3;
    float z2 = z0 - k2 + 2.0f * G3;
    float x3 = x0 - 1.0f + 3.0f * G3; // Offsets for last corner in (x,y,z) coords
    float y3 = y0 - 1.0f + 3.0f * G3;
    float z3 = z0 - 1.0f + 3.0f * G3;

    // Work out the hashed gradient indices of the four simplex corners
    int gi0 = hash(i + hash(j + hash(k)));
    int gi1 = hash(i + i1 + hash(j + j1 + hash(k + k1)));
    int gi2 = hash(i + i2 + hash(j + j2 + hash(k + k2)));
    int gi3 = hash(i + 1 + hash(j + 1 + hash(k + 1)));

    // Calculate the contribution from the four corners
    float t0 = 0.6f - x0*x0 - y0*y0 - z0*z0;
    if (t0 < 0) {
        n0 = 0.0;
    } else {
        t0 *= t0;
        n0 = t0 * t0 * grad(gi0, x0, y0, z0);
    }
    float t1 = 0.6f - x1*x1 - y1*y1 - z1*z1;
    if (t1 < 0) {
        n1 = 0.0;
    } else {
        t1 *= t1;
        n1 = t1 * t1 * grad(gi1, x1, y1, z1);
    }
    float t2 = 0.6f - x2*x2 - y2*y2 - z2*z2;
    if (t2 < 0) {
        n2 = 0.0;
    } else {
        t2 *= t2;
        n2 = t2 * t2 * grad(gi2, x2, y2, z2);
    }
    float t3 = 0.6f - x3*x3 - y3*y3 - z3*z3;
    if (t3 < 0) {
        n3 = 0.0;
    } else {
        t3 *= t3;
        n3 = t3 * t3 * grad(gi3, x3, y3, z3);
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0f*(n0 + n1 + n2 + n3);
}

float NoiseFunctions::fractalPerlin(size_t octaves, float x) const {
    float output    = 0.f;
    float denom     = 0.f;
    float frequency = mFrequency;
    float amplitude = mAmplitude;

    for (size_t i = 0; i < octaves; i++) {
        output += (amplitude * Perlin(x * frequency));
        denom += amplitude;

        frequency *= mLacunarity;
        amplitude *= mPersistence;
    }

    return (output / denom);
}

float NoiseFunctions::fractalPerlin(size_t octaves, float x, float y) const {
    float output = 0.f;
    float denom  = 0.f;
    float frequency = mFrequency;
    float amplitude = mAmplitude;

    for (size_t i = 0; i < octaves; i++) {
        output += (amplitude * Perlin(x * frequency, y * frequency));
        denom += amplitude;

        frequency *= mLacunarity;
        amplitude *= mPersistence;
    }

    return (output / denom);
}

float NoiseFunctions::fractalPerlin(size_t octaves, float x, float y, float z) const {
    float output = 0.f;
    float denom  = 0.f;
    float frequency = mFrequency;
    float amplitude = mAmplitude;

    for (size_t i = 0; i < octaves; i++) {
        output += (amplitude * Perlin(x * frequency, y * frequency, z * frequency));
        denom += amplitude;

        frequency *= mLacunarity;
        amplitude *= mPersistence;
    }

    return (output / denom);
}

float NoiseFunctions::PerlinSmoothStep(float x, float y)
{
    float n = (this->Perlin(x,y) / 2.f) + 0.5f; // converts to range [0,1]
    return float( glm::smoothstep(0.25f,0.75f, n) );
}

float NoiseFunctions::PerlinSmoothStep(float x, float y, float z)
{
    float n = (this->Perlin(x,y,z) / 2.f) + 0.5f; // converts to range [0,1]
    return float( glm::smoothstep(0.25f,0.75f, n) );
}

float NoiseFunctions::WorleyNoise(float x, float y)
{
    glm::vec2 uvin = glm::vec2(x,y);
    glm::vec2 uv = uvin * 10.f;
    // Tile the space
    glm::vec2 uvInt = glm::floor(uv);
    glm::vec2 uvFract = glm::fract(uv);
    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int y = -1; y <= 1; y++) {
        for(int x = -1; x <= 1; x++) {
            glm::vec2 neighbor = glm::vec2(float(x), float(y));

            // Random point inside current neighboring cell
            glm::vec2 point = random2(uvInt + neighbor);

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            glm::vec2 diff = neighbor + point - uvFract;
            float dist = glm::length(diff);
            minDist = glm::min(minDist, dist);
        }
    }
    return minDist;
}

float NoiseFunctions::fBm(size_t octaves, float x)
{
    float output    = 0.f;
    float denom     = 0.f;
    float frequency = mFrequency;
    float amplitude = mAmplitude;

    for (size_t i = 0; i < octaves; i++) {
        output += (amplitude * interpNoise1D(x * frequency));
        denom += amplitude;

        frequency *= mLacunarity;
        amplitude *= mPersistence;
    }

    return (output / denom);
}

float NoiseFunctions::fBm(size_t octaves, float x, float y)
{
    float output = 0.f;
    float denom  = 0.f;
    float frequency = mFrequency;
    float amplitude = mAmplitude;

    for (size_t i = 0; i < octaves; i++) {
        output += (amplitude * bilerpNoise(glm::vec2(x * frequency, y * frequency)));
        denom += amplitude;

        frequency *= mLacunarity;
        amplitude *= mPersistence;
    }

    return (output / denom);
}

float NoiseFunctions::random1(glm::vec2 p)
{
    return glm::fract(sin(glm::dot(p,glm::vec2(127.1,311.7)))*43758.5453);
}

float NoiseFunctions::random1(glm::vec2 p) const
{
    return glm::fract(sin(glm::dot(p,glm::vec2(127.1,311.7)))*43758.5453);
}


glm::vec2 NoiseFunctions::random2(glm::vec2 p)
{
    return glm::fract(glm::sin(glm::vec2(glm::dot(p,glm::vec2(127.1,311.7)),glm::dot(p,glm::vec2(269.5,183.3))))*43758.5453f);
}

glm::vec3 NoiseFunctions::random3from2(glm::vec2 p)
{
    return glm::fract(glm::sin(glm::vec3(glm::dot(p,glm::vec2(127.1,311.7)),
                                         glm::dot(p,glm::vec2(269.5,183.3)),
                                         glm::dot(p,glm::vec2(923.5,656.2))
                                         ))*43758.5453f);
}


float NoiseFunctions::mySmootherStep(float a, float b, float t)
{
    t = t*t*t*(t*(t*6.0 - 15.0) + 10.0);
    return glm::mix(a, b, t);
}

glm::vec2 NoiseFunctions::rotate(glm::vec2 p, float deg)
{
    float rad = deg * 3.14159 / 180.0;
    return glm::vec2(cos(rad) * p.x - sin(rad) * p.y,
                     sin(rad) * p.x + cos(rad) * p.y);
}

float NoiseFunctions::bilerpNoise(glm::vec2 uv)
{
    glm::vec2 uvFract = glm::fract(uv);
    float ll = random1(glm::floor(uv));
    float lr = random1(glm::floor(uv) + glm::vec2(1,0));
    float ul = random1(glm::floor(uv) + glm::vec2(0,1));
    float ur = random1(glm::floor(uv) + glm::vec2(1,1));

    float lerpXL = mySmootherStep(ll, lr, uvFract.x);
    float lerpXU = mySmootherStep(ul, ur, uvFract.x);

    return mySmootherStep(lerpXL, lerpXU, uvFract.y);
}

float NoiseFunctions::interpNoise1D(float x)
{
    int intX = int(floor(x));
    float fractX = glm::fract(x);

    float v1 = random1(glm::vec2(intX,intX));
    float v2 = random1(glm::vec2(intX + 1,intX + 1));
    return glm::mix(v1, v2, fractX);
}

