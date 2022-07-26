#include "biomes.h"

Biomes::Biomes()
    : noise()
{}

///
/// \brief Biomes::getHeight
/// \param x
/// \param z
/// \return
///  Calculates height use fractal Perlin noise
float Biomes::getHeight(float x, float z)
{
    float scale = 256.f;

    float n = noise.fractalPerlin(6,x/scale, z/scale, 0.8)/2.f + 0.5;
    float fudgeFactor = 1.2f;
    n = glm::pow(n*fudgeFactor,5.f)*125.f + 1.f;
    return n;
}

///
/// \brief Biomes::getMoisture
/// \param x
/// \param z
/// \return
/// Noise function to emulate moisture (wet/dry) for picking biomes
///    noise range [0,1]
float Biomes::getMoisture(float x, float z)
{
    // See screen captures
    // scale 150-200 decent distribution
    // scale 100 more frequent biome changes
    // scale 250 less frequent biome changes
    float scale = 200.f;
    float offset = 1024.f;
    x = (x + offset)/scale;
    z = (z + offset)/scale;
    //    float n = noise.PerlinSmoothStep(x, z, 0.8)/2.f + 0.5;
    float n = noise.Perlin(x, z, 0.8)/2.f + 0.5;
    return n;
}

///
/// \brief Biomes::getHeatNoise
/// \param x
/// \param z
/// \return
/// Noise function to emulate heat (hot/cold) for picking biomes
///    noise range [0,1]
float Biomes::getHeatNoise(float x, float z)
{
    // See screen captures
    // scale 150-200 decent distribution
    // scale 100 more frequent biome changes
    // scale 250 less frequent biome changes
    float scale = 200.f;
    float offset = 0.f;
    x = (x + offset)/scale;
    z = (z + offset)/scale;
    //    float n = noise.PerlinSmoothStep(x, z, 0.8)/2.f + 0.5;
    float n = noise.Perlin(x, z, 0.8)/2.f + 0.5;
    return n;

    /*
    // ORIGINAL VERSION
    float scale = 256.f;
    float offset = 0.f;
    x = (x + offset)/scale;
    z = (z + offset)/scale;
    //    float n = noise.PerlinSmoothStep(x, z, 0.8)/2.f + 0.5;
    float n = noise.Perlin(x, z, 0.8)/2.f + 0.5;
    return n;
    */
}

///
/// \brief Biomes::getBiomeType
/// \param x
/// \param z
/// \return
///  Returns biome type based on (x,z)
BiomeType Biomes::getBiomeType(float x, float z)
{
    BiomeType btype;

    // heat noise: if < .5 cold, > .5 hot
    float h = getHeatNoise(x,z);
    // moisture noise: if < .5 dry, > .5 wet
    float m = getMoisture(x,z);

    // use two perlin functions with offset origins to create a
    // 2x2 grid to determine between four biomes
    if (h >= 0.5) {  // hot
        if (m >= 0.5)  {  // wet
            btype = GRASSLAND;
        } else {  // dry
            btype =  DESERTLAND;
        }
    }  else {   // cold
        if (m >= 0.5)  {  // wet
            btype =  FORRESTLAND;
        } else {  // dry
            btype =  MOUNTAINLAND;
        }
    }
    return btype;
}

///
/// \brief Biomes::getBiomeandHeight
/// \param x
/// \param z
/// \param height - returns biome height
/// \param btype - returns biome type
///  Assigns a biome using two perlin noise functions offset from each other
///    modelled as temperature and moisture
void Biomes::getBiomeandHeight(float x, float z, float& height, BiomeType& btype)
{
    float temperature = getHeatNoise(x,z);
    float moisture = getMoisture(x,z);
    float h1 = 0.25;
    float h2 = 0.75;
    moisture = glm::smoothstep(h1, h2, moisture);
    temperature = glm::smoothstep(h1, h2, temperature);

    // use two perlin functions with offset origins to create a
    // 2x2 grid to determine between four biomes
    // Moisure and Temperature: [0,1]
    // all the ___Land: [0,128] (Maybe higher)
    if (moisture >= 0.5 && temperature >= 0.5) {
        btype = FORRESTLAND;
        float topPercent = moisture;
        float bottomPercent = (1 - moisture);
        float topRightPercent = topPercent * temperature;
        float topLeftPercent = topPercent * (1 - temperature);
        float bottomRightPercent = bottomPercent * temperature;
        float bottomLeftPercent = bottomPercent * (1 - temperature);
        height = topRightPercent * getForrestlandHt(x,z)
                + topLeftPercent * getGrasslandHt(x,z)
                + bottomRightPercent * getMountainlandHt(x,z)
                + bottomLeftPercent * getDesertlandHt(x,z) + 1;
    } else if (moisture >= 0.5 && temperature < 0.5) {
        btype = GRASSLAND;
        float topPercent = moisture;
        float bottomPercent = (1 - moisture);
        float topRightPercent = topPercent * temperature;
        float topLeftPercent = topPercent * (1 - temperature);
        float bottomRightPercent = bottomPercent * temperature;
        float bottomLeftPercent = bottomPercent * (1 - temperature);
        height = topRightPercent * getForrestlandHt(x,z)
                + topLeftPercent * getGrasslandHt(x,z)
                + bottomRightPercent * getMountainlandHt(x,z)
                + bottomLeftPercent * getDesertlandHt(x,z) + 1;
    } else if(moisture < 0.5 && temperature >= 0.5) {
        btype = MOUNTAINLAND;
        float topPercent = moisture;
        float bottomPercent = (1 - moisture);
        float topRightPercent = topPercent * temperature;
        float topLeftPercent = topPercent * (1 - temperature);
        float bottomRightPercent = bottomPercent * temperature;
        float bottomLeftPercent = bottomPercent * (1 - temperature);
        height = topRightPercent * getForrestlandHt(x,z)
                + topLeftPercent * getGrasslandHt(x,z)
                + bottomRightPercent * getMountainlandHt(x,z)
                + bottomLeftPercent * getDesertlandHt(x,z) + 1;
    } else if (moisture < 0.5 && temperature < 0.5) {
        btype = DESERTLAND;
        float topPercent = moisture;
        float bottomPercent = (1 - moisture);
        float topRightPercent = topPercent * temperature;
        float topLeftPercent = topPercent * (1 - temperature);
        float bottomRightPercent = bottomPercent * temperature;
        float bottomLeftPercent = bottomPercent * (1 - temperature);
        height = topRightPercent * getForrestlandHt(x,z)
                + topLeftPercent * getGrasslandHt(x,z)
                + bottomRightPercent * getMountainlandHt(x,z)
                + bottomLeftPercent * getDesertlandHt(x,z) + 3;
    }
}

void Biomes::getCaveSystem(float x, float z, bool (&solidOrNot) [140])
{
    // good: 32,8,8f;
    float scale = 32.f;
    float scale2 = 8.f;
    float scale3 = 8.f;

    x /= scale;
    z /= scale2;

    int startCaves = 4;
    for (int i = 0; i < 130; i++)
    {
        if (i < startCaves) {
            solidOrNot[i] = 1; // This should be a 1
        } else {
//            float holder = noise.Perlin(x,z,i/scale3);
            float holder = noise.classPerlinNoise3D(x,z,i/scale3);
            if (holder < 0) {
                solidOrNot[i] = 1;
            } else {
                solidOrNot[i] = 0;
            }
        }
    }
}


///
/// \brief Biomes::getGrasslandHt
/// \param x
/// \param z
/// \return
///
///
///
float Biomes::getGrasslandHt(float x, float z)
{
    float scale = 256.f;  //0.3f;
    x /= scale;
    z /= scale;
    float worley = noise.WorleyNoise(x,z);
    worley = glm::max(0.f, worley-0.1f);
    worley = glm::smoothstep(0.f,1.f,worley);
    float fbmNoise = noise.fractalPerlin(8,x,z)/2.f + 0.5f;  // [0,1]
    float h = worley*0.33f + fbmNoise*0.67f;                 // [0,1]
    h = h*32.f + 125.f;  //  h = h*30.f + 128.f;
    return (h);
}


///
/// \brief Biomes::getMountainlandHt
/// \param x
/// \param z
/// \return outputs world coord heights
///  Calculates height for the Mountain Biome
float Biomes::getMountainlandHt(float x, float z)
{
    float scale = 256.f;
    size_t octave = 8;
    float expo = 3.f;
    x /= scale;
    z /= scale;
    float n = noise.fractalPerlin(octave,x, z, 0.8)/2.f + 0.5; // range [0,1]
    float fudgeFactor = 1.2f;
    n = glm::pow(n*fudgeFactor,expo); // glm::pow(fudgeFactor,expo)

    n = n*128.f + 128.f; // Should be 128 and 120

    return glm::clamp(n,0.f,255.f);
}

///
/// \brief Biomes::getDesertlandHt
/// \param x
/// \param z
/// \return
///
float Biomes::getDesertlandHt(float x, float z)
{
    float scale = 256.f;
    size_t octave = 12.f;
    float expo = 5.f;
    x /= scale;
    z /= scale;

    // seems too flat
//    float pn = noise.fractalPerlin(4,x,z);
//    float height = glm::max( 0.05f*(1.0f-glm::pow(4.f*(pn-0.42f),2.0f)),0.0f) + glm::smoothstep(0.05f,0.2f,pn) * (glm::max(((0.4f-pn*pn)*0.2f),0.0f) + (step(pn,0.69f) + step(0.69f,pn) * (glm::abs(glm::mod(glm::floor(pn*100.f),2.0f))*0.1f+0.9f) * step(pn,0.8f)  + 1.0f - step(pn,0.8f)) * ((glm::pow(glm::smoothstep(0.f,0.9f, float(pow(glm::smoothstep(0.f,1.0f,pn),1.0f))),100.f) +  pn*0.1f) / 1.1f));
//    float h = (1.f - height)*50.f + 1.f;
//    return h;

    // maybe to artificial
//    float n = woodlikePerlin(x,z)*30 + 1.f;
//    return n;

    // possible
    float n = noise.fractalPerlin(octave,x, z, 0.8)/2.f + 0.5;
    float fudgeFactor = 1.25f;
    n = glm::pow(n*fudgeFactor,expo) / glm::pow(fudgeFactor,expo);

    n = n*50.f + 126.f;

    return glm::clamp(n,0.f,255.f);
}

///
/// \brief Biomes::getForrestlandHt
/// \param x
/// \param z
/// \return
///
float Biomes::getForrestlandHt(float x, float z)
{
    float scale = 256.f;  //0.3f;
    x /= scale;
    z /= scale;
    float worley = noise.WorleyNoise(x,z);
    worley = glm::max(0.f, worley-0.1f);
    worley = glm::smoothstep(0.f,1.f,worley);
    float fbmNoise = noise.fractalPerlin(8,x,z)/2.f + 0.5f;  // [0,1]
    float h = worley*0.33f + fbmNoise*0.67f;                 // [0,1]

    h = h*20.f + 138.f;  //  h = h*30.f + 128.f;

    return (h);
}















