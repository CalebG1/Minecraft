#pragma once
#include "glm_includes.h"
#include "entity.h"
#include "drawable.h"
#include "noisefunctions.h"
#include "shaderprogram.h"
#include "terrain.h"

static const int CUB_IDX_COUNT = 36;
static const int CUB_VERT_COUNT = 24;

class Bomb  : public Drawable
{
public:
    glm::vec3 m_position;
    float m_velocity, m_acceleration;
    glm::vec3 playPos;
    NoiseFunctions noise;
    float distance;
    bool createdVBO;
    bool stopMoving;
    Terrain &terrain;
    bool justBroke;
    bool soundOn;


    Bomb(glm::vec3 pos, OpenGLContext* mp_context, float init_vel, float dist, Terrain &terrain);

    void tick(glm::vec3 m_pos);
    void createCubeVertexPositions(glm::vec4 (&cub_vert_pos)[CUB_VERT_COUNT]);
    void createCubeVertexNormals(glm::vec4 (&cub_vert_nor)[CUB_VERT_COUNT]);
    void createCubeIndices(GLuint (&cub_idx)[CUB_IDX_COUNT]);
    void createVBOdata() override;
    void draw(ShaderProgram *shaderProgram);
virtual ~Bomb(){}
};
