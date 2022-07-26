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

#pragma once

#include <cstddef>  // size_t
#include "glm_includes.h"


/**
 * @brief A Perlin Simplex Noise C++ Implementation (1D, 2D, 3D, 4D).
 */
class NoiseFunctions {
public:
    float random1( glm::vec2 p );
    float random1( glm::vec2 p ) const;
    glm::vec2 random2( glm::vec2 p );
    glm::vec3 random3from2( glm::vec2 p );
    float mySmootherStep(float a, float b, float t);
    glm::vec2 rotate(glm::vec2 p, float deg);
    float bilerpNoise(glm::vec2 uv);
    float interpNoise1D(float x);
    float PerlinSmoothStep(float x, float y);
    float PerlinSmoothStep(float x, float y, float z);
    float WorleyNoise(float x, float y);
    float fBm(size_t octaves, float x);
    float fBm(size_t octaves, float x, float y);
    float classPerlinNoise3D(float x, float y, float z);
    float surflet(glm::vec3 p, glm::vec3 gridPoint);
    explicit NoiseFunctions(float frequency = 1.0f,
                          float amplitude = 1.0f,
                          float lacunarity = 2.0f,
                          float persistence = 0.5f) :
        mFrequency(frequency),
        mAmplitude(amplitude),
        mLacunarity(lacunarity),
        mPersistence(persistence) {
    }


  // Used versions from "https://github.com/SRombauts/SimplexNoise" instead of inclass
  // versions of the identical functions because these ran faster
  // Note: These are only REPLACEMENTS to in class code, not new functionality
    static float Perlin(float x);
    static float Perlin(float x, float y);
    static float Perlin(float x, float y, float z);
    float fractalPerlin(size_t octaves, float x) const;
    float fractalPerlin(size_t octaves, float x, float y) const;
    float fractalPerlin(size_t octaves, float x, float y, float z) const;

private:
    // Parameters of Fractional Brownian Motion (fBm) : sum of N "octaves" of noise
    float mFrequency;   ///< Frequency ("width") of the first octave of noise (default to 1.0)
    float mAmplitude;   ///< Amplitude ("height") of the first octave of noise (default to 1.0)
    float mLacunarity;  ///< Lacunarity specifies the frequency multiplier between successive octaves (default to 2.0).
    float mPersistence; ///< Persistence is the loss of amplitude between successive octaves (usually 1/lacunarity)
};
