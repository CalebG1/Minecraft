#include "npc.h"
#include <cmath>
#include <iostream>

//npc::npc(glm::vec3 pos, OpenGLContext *mp_context, float init_vel, float dist)
//    : Drawable(mp_context), m_position(glm::vec3(pos.x+dist,pos.z,pos.y+dist)),
//      m_velocity(init_vel), m_acceleration(0), playPos(pos), noise(), distance(dist),
//      createdVBO(false), bomb(glm::vec3(pos.x+dist,pos.z,pos.y+dist),mp_context, 0.f,0.f)
//{}
npc::npc(glm::vec3 pos, OpenGLContext *mp_context, float init_vel, float dist, Terrain &terrain)
    : Drawable(mp_context), m_position(glm::vec3(pos.x,pos.z,pos.y)),
      m_velocity(init_vel), m_acceleration(0), playPos(pos), noise(), distance(dist),
      createdVBO(false), bomb(glm::vec3(pos.x,pos.z,pos.y),mp_context, 0.f,0.f, terrain), mcr_terrain(terrain),
      startPoint(180.f), lastHeight(-1)
{}

void npc::tick(float dT, float accel_rate, glm::vec3 m_pos, float time)
{
    // Note this is [-1,1]
    // float acceleration = noise.Perlin(dT);
    glm::vec3 oldPos = m_position;
    time = time * 0.0001;
    float acceleration = 0.1;
    m_velocity += acceleration *accel_rate;
    if (m_velocity < 0) m_velocity = 0;
    float newX = sin(time)*distance + m_pos.x;
    float newZ = sin(time)*cos(time)*distance + m_pos.z;
    if (lastHeight == -1) {
        m_position = glm::vec3(newX, startPoint, newZ);
    } else {
        if (m_position.y - 0.25 >= startPoint) {
            m_position = glm::vec3(newX, m_position.y - 0.25, newZ);
        } else {
            m_position = glm::vec3(newX, startPoint, newZ);
        }
    }
    bool hasMoved = true;
    float change = 0;
    do {


        m_position = glm::vec3(newX, m_position.y + change, newZ);
        change = change + 0.15;
        hasMoved = true;
        for (int i = -15; i < 15; i++) {
            if (hasMoved == true) {
                for (int j = -15; j < 15; j++) {
                    for (int k = -1; k < 1; k++) {
                        if (mcr_terrain.hasChunkAt(m_position.x + i,m_position.z + k)) {
                            if (mcr_terrain.getBlockAt(glm::vec3(m_position.x + i, m_position.y + j, m_position.z + k)) != EMPTY) {
                                //                             m_position = glm::vec3(oldX, oldY, oldY); // Should be abble to comment out
                                hasMoved = false;
                            }
                        }
                    }
                }
            }
        }
    } while (hasMoved == false);
//    glm::vec3 toPass =  glm::vec3(m_position.x + 10 , 228.f, m_position.z - 10);
    glm::vec3 toPass =  glm::vec3(m_position.x , m_position.y, m_position.z);
    bomb.tick(toPass);
    lastHeight = m_position.y;
    fwdVec = glm::vec3(newX, m_position.y, newZ) - oldPos;

}

//These are functions that are only defined in this cpp file. They're used for organizational purposes
//when filling the arrays used to hold the vertex and index data.
void npc::createCubeVertexPositions(glm::vec4 (&cub_vert_pos)[CUB_VERT_COUNT])
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

void npc::createCubeVertexNormals(glm::vec4 (&cub_vert_nor)[CUB_VERT_COUNT])
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

void npc::createCubeIndices(GLuint (&cub_idx)[CUB_IDX_COUNT])
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

void npc::createVBOdataFromOBJ(const char* filename)
{
    std::vector<tinyobj::shape_t> shapes; std::vector<tinyobj::material_t> materials;
    std::string errors = tinyobj::QLoadObj(shapes, materials, filename);
    if(errors.size() == 0)
    {
        int count = 0;
        //Read the information from the vector of shape_ts
        for(unsigned int i = 0; i < shapes.size(); i++)
        {
            std::vector<float> &positions = shapes[i].mesh.positions;
            std::vector<float> &normals = shapes[i].mesh.normals;
            std::vector<float> &uvs = shapes[i].mesh.texcoords;
            std::vector<unsigned int> &indices = shapes[i].mesh.indices;

            bool normalsExist = normals.size() > 0;

            std::vector<GLuint> glIndices;
            for(unsigned int ui : indices)
            {
                glIndices.push_back(ui);
            }
            std::vector<glm::vec4> glPos;
            std::vector<glm::vec4> glNor;

            for(int x = 0; x < positions.size(); x += 3)
            {
                glPos.push_back(glm::vec4(positions[x], positions[x + 1], positions[x + 2], 1.f));
                if(normalsExist)
                {
                    glNor.push_back(glm::vec4(normals[x], normals[x + 1], normals[x + 2], 1.f));
                }
            }

            this->m_opaqueCount = glPos.size();

            generateIdxOpaque();
            mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpaque);
            mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, glIndices.size() * sizeof(GLuint), glIndices.data(), GL_STATIC_DRAW);

            generateOpaque();
            mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufOpaque);
            mp_context->glBufferData(GL_ARRAY_BUFFER, glPos.size() * sizeof(glm::vec4), glPos.data(), GL_STATIC_DRAW);

            generateNor();
            mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
            mp_context->glBufferData(GL_ARRAY_BUFFER, glNor.size() * sizeof(glm::vec4), glNor.data(), GL_STATIC_DRAW);

            count += indices.size();
        }
    }

//    mp_texture = std::unique_ptr<Texture>(new Texture(context));
//    mp_texture->create(textureFile);

//    mp_bgTexture = std::unique_ptr<Texture>(new Texture(context));
//    mp_bgTexture->create(bgTextureFile);
}

void npc::createVBOdata()
{
    createdVBO = true;
    createVBOdataFromOBJ(":/SwordMinecraft.obj"); // This will hopefully replace what's below
}

void npc::npcPhysics()
{
    int checks = 20;
    glm::vec3 vertices[12] = { glm::vec3(0.3, 0, 0.3),
                               glm::vec3(-0.3, 0, 0.3),
                               glm::vec3(0.3, 0, -0.3),
                               glm::vec3(-0.3, 0, -0.3),
                               glm::vec3(0.3, 0.9, 0.3),
                               glm::vec3(-0.3, 0.9, 0.3),
                               glm::vec3(0.3, 0.9, -0.3),
                               glm::vec3(-0.3, 0.9, -0.3),
                               glm::vec3(0.3, 1.8, 0.3),
                               glm::vec3(-0.3, 1.8, 0.3),
                               glm::vec3(0.3, 1.8, -0.3),
                               glm::vec3(-0.3, 1.8, -0.3) };

//    glm::vec3 movement = m_velocity * dT;
//    float newX = sin(m_velocity)*distance + m_pos.x;
//    float newZ = sin(m_velocity)*cos(m_velocity)*distance + m_pos.z;

//    for (const auto& v : vertices) {
//        for (int i = 1; i <= checks+1; i++) {
//            for (int dir = 0; dir < 3; dir++) {
//                // Reduces velocity in a particular axis to 0 if there is a block in that direction
//                glm::vec3 axis = glm::vec3(0.f);
//                axis[dir] = movement[dir];
//                glm::vec3 checkPoint = m_position + v + float(i) * axis / float(checks);
//                if ((terrain.getBlockAt(checkPoint) != EMPTY)) {
//                    // Check this
//                }
//            }
//        }
//    }



    // Extra collision test with final computed velocity
//    for (const auto& v : vertices) {
//        glm::vec3 moved_vertex = m_position + v + movement;
//        if ((terrain.getBlockAt(moved_vertex) != EMPTY)  && (terrain.getBlockAt(moved_vertex) != WATER) && (terrain.getBlockAt(moved_vertex) != LAVA)) {
//            return;
//        }
//    }

}

void npc::draw(ShaderProgram *shaderProgram, ShaderProgram *forBomb) {
    if (!createdVBO) {
        createVBOdata();
    }

    float rot = atan2(fwdVec.x, fwdVec.z);
    glm::mat4 toSetMatrix = glm::mat4(cos(rot), 0.f, -sin(rot), 0.f,
                                      0.f, 1.0f, 0.f, 0.f,
                                      sin(rot), 0.f, cos(rot), 0.f,
                                      m_position.x, m_position.y, m_position.z, 1.f);

    shaderProgram->setModelMatrix(toSetMatrix);
    shaderProgram->draw(*this);

    bomb.draw(forBomb);

//    toSetMatrix = glm::mat4(1.f, 0.f, 0.f, 0.f,
//                                                0.f, 1.0f, 0.f, 0.f,
//                                                0.f, 0.f, 1.f, 0.f,
//                                                m_position.x + 10 , m_position.y + 10, m_position.z + 10, 1.f);

//    shaderProgram->setModelMatrix(toSetMatrix);
//    shaderProgram->draw(*this);


//    toSetMatrix = glm::mat4(1.f, 0.f, 0.f, 0.f,
//                                                0.f, 1.0f, 0.f, 0.f,
//                                                0.f, 0.f, 1.f, 0.f,
//                                                m_position.x - 10 , m_position.y + 10, m_position.z - 10, 1.f);

//    shaderProgram->setModelMatrix(toSetMatrix);
//    shaderProgram->draw(*this);

}


