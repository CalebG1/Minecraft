#include "bombs.h"
#include <cmath>

Bomb::Bomb(glm::vec3 pos, OpenGLContext *mp_context, float init_vel, float dist, Terrain &ter)
    : Drawable(mp_context), m_position(glm::vec3(pos.x,pos.z,pos.y)), m_velocity(init_vel), m_acceleration(0),
      playPos(pos), noise(), distance(dist), createdVBO(false), stopMoving(false), terrain(ter), justBroke(false),
      soundOn(false)
{}

void Bomb::tick(glm::vec3 m_pos)
{
    if (!stopMoving) {
        if (m_position.y < 120) {
            float x = m_pos.x;
            float y = m_pos.y;
            float z = m_pos.z;
            m_position = glm::vec3(x,y,z);
            glm::vec3 test = glm::vec3(x, y, z);
            justBroke = false;
        } else {
            float xDraw = m_position.x;
            float yDraw = m_position.y;
            float zDraw = m_position.z;
//            m_position = glm::vec3(xDraw, yDraw + 10, zDraw);
            if ((justBroke == false) && (terrain.hasChunkAt(xDraw, zDraw)) &&
                    (terrain.getBlockAt(xDraw, yDraw - 8, zDraw) != EMPTY)&&
                    (terrain.getBlockAt(xDraw, yDraw - 8, zDraw) != WATER)) {
                soundOn = true;
                if (terrain.hasChunkAt(xDraw, zDraw)) {
                    terrain.setBlockAt(xDraw, (yDraw - 8) - 1, zDraw, LAVA);
                }
                if (terrain.hasChunkAt(xDraw, zDraw)) {
                    terrain.setBlockAt(xDraw, (yDraw -8) +1, zDraw, LAVA);
                }
                if (terrain.hasChunkAt(xDraw - 1, zDraw -1)) {
                    terrain.setBlockAt(xDraw - 1, (yDraw -8), zDraw - 1, LAVA);
                }
                if (terrain.hasChunkAt(xDraw -1, zDraw)) {
                    terrain.setBlockAt(xDraw - 1, (yDraw - 8), zDraw, LAVA);
                }
                if (terrain.hasChunkAt(xDraw - 1,zDraw +1)) {
                    terrain.setBlockAt(xDraw -1 , (yDraw -8), zDraw + 1, LAVA);
                }
                if (terrain.hasChunkAt(xDraw, zDraw - 1)) {
                    terrain.setBlockAt(xDraw, (yDraw -8), zDraw - 1, LAVA);
                }
                if (terrain.hasChunkAt(xDraw, zDraw)) {
                    terrain.setBlockAt(xDraw, (yDraw -8), zDraw, LAVA);
                }
                if (terrain.hasChunkAt(xDraw,zDraw + 1)) {
                    terrain.setBlockAt(xDraw, (yDraw -8), zDraw + 1, LAVA);
                }
                if (terrain.hasChunkAt(xDraw + 1, zDraw -1)) {
                    terrain.setBlockAt(xDraw + 1, (yDraw -8), zDraw - 1, LAVA);
                }
                if (terrain.hasChunkAt(xDraw + 1, zDraw)) {
                    terrain.setBlockAt(xDraw + 1, (yDraw -8), zDraw, LAVA);
                }
                if (terrain.hasChunkAt(xDraw + 1, zDraw + 1)) {
                    terrain.setBlockAt(xDraw + 1, (yDraw -8), zDraw + 1, LAVA);
                }
                m_position = glm::vec3(xDraw, (yDraw) + 0.2,zDraw);
                justBroke = true;
            } else {
                m_position = glm::vec3(xDraw, (yDraw) - 0.2, zDraw);
                soundOn = false;
            }
        }
    }
}

//These are functions that are only defined in this cpp file. They're used for organizational purposes
//when filling the arrays used to hold the vertex and index data.
void Bomb::createCubeVertexPositions(glm::vec4 (&cub_vert_pos)[CUB_VERT_COUNT])
{
    int idx = 0;
    //Front face
    //UR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    //LR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    //LL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    //UL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);

    //Right face
    //UR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    //LR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    //LL
    cub_vert_pos[idx++] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    //UL
    cub_vert_pos[idx++] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    //Left face
    //UR
    cub_vert_pos[idx++] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    //LR
    cub_vert_pos[idx++] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    //LL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    //UL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

    //Back face
    //UR
    cub_vert_pos[idx++] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    //LR
    cub_vert_pos[idx++] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    //LL
    cub_vert_pos[idx++] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    //UL
    cub_vert_pos[idx++] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

    //Top face
    //UR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    //LR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    //LL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    //UL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

    //Bottom face
    //UR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    //LR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    //LL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    //UL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
}

void Bomb::createCubeVertexNormals(glm::vec4 (&cub_vert_nor)[CUB_VERT_COUNT])
{
    int idx = 0;
    //Front
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec4(0,0,1,0);
    }
    //Right
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec4(1,0,0,0);
    }
    //Left
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec4(-1,0,0,0);
    }
    //Back
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec4(0,0,-1,0);
    }
    //Top
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec4(0,1,0,0);
    }
    //Bottom
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec4(0,-1,0,0);
    }
}

void Bomb::createCubeIndices(GLuint (&cub_idx)[CUB_IDX_COUNT])
{
    int idx = 0;
    for(int i = 0; i < 6; i++){
        cub_idx[idx++] = i*4;
        cub_idx[idx++] = i*4+1;
        cub_idx[idx++] = i*4+2;
        cub_idx[idx++] = i*4;
        cub_idx[idx++] = i*4+2;
        cub_idx[idx++] = i*4+3;
    }
}

void Bomb::createVBOdata()
{
    createdVBO = true;

    GLuint sph_idx[CUB_IDX_COUNT];
    glm::vec4 sph_vert_pos[CUB_VERT_COUNT];
    glm::vec4 sph_vert_nor[CUB_VERT_COUNT];

    createCubeVertexPositions(sph_vert_pos);
    createCubeVertexNormals(sph_vert_nor);
    createCubeIndices(sph_idx);

    m_opaqueCount = CUB_IDX_COUNT;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdxOpaque();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpaque);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, CUB_IDX_COUNT * sizeof(GLuint), sph_idx, GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generateOpaque();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufOpaque);
    mp_context->glBufferData(GL_ARRAY_BUFFER, CUB_VERT_COUNT * sizeof(glm::vec4), sph_vert_pos, GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, CUB_VERT_COUNT * sizeof(glm::vec4), sph_vert_nor, GL_STATIC_DRAW);
}


void Bomb::draw(ShaderProgram *shaderProgram) {
    if (!createdVBO) {
        createVBOdata();
    }
    glm::mat4 toSetMatrix = glm::mat4(1.f, 0.f, 0.f, 0.f,
                                                0.f, 1.0f, 0.f, 0.f,
                                                0.f, 0.f, 1.f, 0.f,
                                                m_position.x, m_position.y, m_position.z, 1.f);

    shaderProgram->setModelMatrix(toSetMatrix);
    shaderProgram->draw(*this);
}
