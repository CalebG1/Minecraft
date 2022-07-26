#pragma once
#include "glm_includes.h"
#include "entity.h"
#include "drawable.h"
#include "noisefunctions.h"
#include "shaderprogram.h"
#include "terrain.h"
#include "tiny_obj_loader.h"
#include "bombs.h"

class npc : public Drawable
{
public:
    glm::vec3 m_position;
    float m_velocity, m_acceleration;
    glm::vec3 fwdVec;
    glm::vec3 playPos;
    NoiseFunctions noise;
    float distance;
    bool createdVBO;
    Bomb bomb;
    Terrain &mcr_terrain;
    float startPoint;
    float lastHeight;


    npc(glm::vec3 pos, OpenGLContext* mp_context, float init_vel, float dist, Terrain &terrain);

    void tick(float dt, float accel_rate, glm::vec3 m_pos, float time);
    void createCubeVertexPositions(glm::vec4 (&cub_vert_pos)[CUB_VERT_COUNT]);
    void createCubeVertexNormals(glm::vec4 (&cub_vert_nor)[CUB_VERT_COUNT]);
    void createCubeIndices(GLuint (&cub_idx)[CUB_IDX_COUNT]);
    void createVBOdataFromOBJ(const char* filename);
    void createVBOdata() override;
    void npcPhysics();
    void draw(ShaderProgram *shaderProgram, ShaderProgram *forBomb);

virtual ~npc(){}
};
