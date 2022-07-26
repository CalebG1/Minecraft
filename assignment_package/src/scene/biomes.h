#pragma once


#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "noisefunctions.h"
#include "chunk.h"

// Biome types
enum BiomeType : unsigned char
{
    GRASSLAND, DESERTLAND, MOUNTAINLAND, FORRESTLAND
};


class Biomes
{
public:
    // Noise function
    NoiseFunctions noise;

    Biomes();

    float getHeight(float x, float z);
    float getMoisture(float x, float z);
    float getHeatNoise(float x, float z);
    BiomeType getBiomeType(float x, float z);
    void getBiomeandHeight(float x, float z, float& height, BiomeType& biometype);
    float woodlikePerlin(float x, float z);
    float getGrasslandHt(float x, float z);
    float getMountainlandHt(float x, float z);
    float getDesertlandHt(float x, float z);
    float getForrestlandHt(float x, float z);

    // Milstone 2
//    void getCaveSystem(float x, float z, bool solidOrNot [128]);
    void getCaveSystem(float x, float z, bool (&solidOrNot) [140]);
    // bool (&solidOrNot) [128]
};

