#include "chunk.h"
#include <iostream>


Chunk::Chunk(OpenGLContext* context, int x, int z) : Drawable(context), m_blocks(), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}, shouldUpdateVBO(false), x(x), z(z)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
RedstoneState Chunk::getRedstoneState(unsigned int x, unsigned int y, unsigned int z) const {
    if (m_blocks.at(x + 16 * y + 16 * 256 * z) != REDSTONE) {
        throw std::out_of_range("No redstone at " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z));
    }
    return m_redstone.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
RedstoneState Chunk::getRedstoneState(int x, int y, int z) const {
    return getRedstoneState(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setRedstoneState(unsigned int x, unsigned int y, unsigned int z, RedstoneState s) {
    if (m_blocks.at(x + 16 * y + 16 * 256 * z) != REDSTONE) {
        throw std::out_of_range("No redstone at " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z));
    }
    m_redstone.at(x + 16 * y + 16 * 256 * z) = s;
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    if(x == 0 && m_neighbors.at(XNEG) != nullptr) {
        m_neighbors.at(XNEG)->shouldUpdateVBO = true;

    } else if(x == 15  && m_neighbors.at(XPOS) != nullptr) {
        m_neighbors.at(XPOS)->shouldUpdateVBO = true;

    } else if(z == 0 && m_neighbors.at(ZNEG) != nullptr) {
        m_neighbors.at(ZNEG)->shouldUpdateVBO = true;
    } else if(z == 15  && m_neighbors.at(ZPOS) != nullptr) {
        m_neighbors.at(ZPOS)->shouldUpdateVBO = true;

    }

    shouldUpdateVBO = true;
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}

void Chunk::updateBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    if(x == 0 && m_neighbors.at(XNEG) != nullptr) {
        m_neighbors.at(XNEG)->shouldUpdateVBO = true;

    } else if(x == 15  && m_neighbors.at(XPOS) != nullptr) {
        m_neighbors.at(XPOS)->shouldUpdateVBO = true;

    } else if(z == 0 && m_neighbors.at(ZNEG) != nullptr) {
        m_neighbors.at(ZNEG)->shouldUpdateVBO = true;
    } else if(z == 15  && m_neighbors.at(ZPOS) != nullptr) {
        m_neighbors.at(ZPOS)->shouldUpdateVBO = true;

    }

    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}

void Chunk::placeBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    if (getBlockAt(x, y, z) == REDSTONE) {
        m_redstone.erase(x + 16 * y + 16 * 256 * z);
    }
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
    if (t == REDSTONE) {
        m_redstone.insert(std::pair<int, RedstoneState>(x + 16 * y + 16 * 256 * z, UNLIT_WIRE_SPOT));
    }
    if(x == 0 && m_neighbors.at(XNEG) != nullptr) {
        m_neighbors.at(XNEG)->createVBOdata();
    } else if(x == 15  && m_neighbors.at(XPOS) != nullptr) {
        m_neighbors.at(XPOS)->createVBOdata();
    } else if(z == 0 && m_neighbors.at(ZNEG) != nullptr) {
        m_neighbors.at(ZNEG)->createVBOdata();
    } else if(z == 15  && m_neighbors.at(ZPOS) != nullptr) {
        m_neighbors.at(ZPOS)->createVBOdata();
    }
    // VBO data is created in update redstone, so no need to do it here
    if (!(t == REDSTONE || t == LIT_REDSTONE_TORCH || t == UNLIT_REDSTONE_TORCH || t == LIT_REDSTONE_LAMP || t == UNLIT_REDSTONE_LAMP)) {
        createVBOdata();
    }
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

void Chunk::setBlockAtNeighbor(int x, int y, int z, BlockType t)
{
    if(x>=0 && x<16 && z>=0 && z<16) {
        setBlockAt(x,y,z,t);
        return;
    }

    int dx = (x%16);
    if(x<0) {
        dx = (x%16) + 16;
    }
    int dz = (z%16);
    if(z<0) {
        dz = (z%16) + 16;
    }

    if(x > 15 && dz == z && m_neighbors.at(XPOS)) {
        m_neighbors.at(XPOS)->setBlockAt(dx,y,z,t);
    }
    else if(x < 0 && dz == z && m_neighbors.at(XNEG)) {
        m_neighbors.at(XNEG)->setBlockAt(dx,y,z,t);
    }
    else if(z > 15 && dx == x && m_neighbors.at(ZPOS)) {
        m_neighbors.at(ZPOS)->setBlockAt(x,y,dz,t);
    }
    else if(z < 0 && dx == x && m_neighbors.at(ZNEG)) {
        m_neighbors.at(ZNEG)->setBlockAt(x,y,dz,t);
    }

}


//m_count = 6;




void Chunk::createVBOdata() {
    ChunkVBOData d = fillVBOdata();
    setupVBO(d.m_vbo, d.m_idx, d.m_tvbo, d.m_tidx);
}

ChunkVBOData Chunk::fillVBOdata() {
    std::vector<glm::vec4> vec;
    std::vector<GLuint> idx;

    std::vector<glm::vec4> transparentVec;
    std::vector<GLuint> transparentIdx;

    GLuint vertCount = 0;
    GLuint transparentVertCount = 0;

    for(int z = 0; z < 16; ++z){
        for(int y = 0; y < 256; ++y){
            for(int x = 0; x < 16; ++x){
                BlockType curr = getBlockAt(x,y,z);
                if(curr != EMPTY){
                    for(const BlockFace& neighborFace : adjacentFaces){
                        glm::vec3 p = neighborFace.directionVec + glm::vec3(x, y, z);
                            BlockType neighborType = EMPTY;

                            if (p.x >= 0 && p.x < 16 && p.z >= 0 && p.z < 16 && p.y >= 0 && p.y < 256){
                             neighborType = getBlockAt((unsigned) p.x, p.y, p.z);
                            } else if(p.x < 0 && m_neighbors.at(XNEG) != nullptr) {
                                neighborType = m_neighbors.at(XNEG)->getBlockAt(15, p.y, p.z);
                            } else if(p.x > 15  && m_neighbors.at(XPOS) != nullptr) {
                                neighborType = m_neighbors.at(XPOS)->getBlockAt(0, p.y, p.z);
                            } else if(p.z < 0 && m_neighbors.at(ZNEG) != nullptr) {
                                neighborType = m_neighbors.at(ZNEG)->getBlockAt(p.x, p.y, 15);
                            } else if(p.z > 15  && m_neighbors.at(ZPOS) != nullptr) {
                                neighborType = m_neighbors.at(ZPOS)->getBlockAt(p.x, p.y, 0);
                            }
                            if(((neighborType == EMPTY && curr != REDSTONE)
                                || neighborType == WATER
                                || (transparentBlocks.count(neighborType) == 1 && curr != EMPTY && curr != REDSTONE)
                                || (neighborType == LAVA && curr != LAVA))
                                    && curr != WATER && transparentBlocks.count(curr) == 0){
                                std::vector<GLuint> faceIdxs = std::vector<GLuint>();
                                for(const VertexData& dat : neighborFace.vertices){

                                    glm::vec4 pos = dat.pos + glm::vec4(x,y,z,0);
                                    vec.push_back(pos);
                                    faceIdxs.push_back(vertCount);
                                    vertCount++;

                                    glm::vec4 nor = glm::vec4(neighborFace.directionVec.x, neighborFace.directionVec.y, neighborFace.directionVec.z, 1.0f);

                                    vec.push_back(nor);

                                    glm::vec2 blockFaceUV;
                                    blockFaceUV = blockFaceUVs.at(curr).at(neighborFace.direction) + dat.uv;


                                    // third element is a flag: 0.0f = do not animate, 1.0f = do animate water/lava
                                    glm::vec4 uvAndFlags = glm::vec4(blockFaceUV.x, blockFaceUV.y, 0.0f, 1.0f);

                                    if (curr == LAVA){
                                        uvAndFlags[2] = 1.0f;
                                    }

                                    vec.push_back(uvAndFlags);

                                }

                                idx.push_back(faceIdxs[0]);
                                idx.push_back(faceIdxs[1]);
                                idx.push_back(faceIdxs[2]);
                                idx.push_back(faceIdxs[0]);
                                idx.push_back(faceIdxs[2]);
                                idx.push_back(faceIdxs[3]);



                            } else if((neighborType == EMPTY && (curr == WATER))
                                      || transparentBlocks.count(curr) == 1) {
                                std::vector<GLuint> faceIdxs = std::vector<GLuint>();
                                for(const VertexData& dat : neighborFace.vertices){
                                    glm::vec4 pos = dat.pos + glm::vec4(x,y,z,0);
                                    if (curr == UNLIT_REDSTONE_TORCH || curr == LIT_REDSTONE_TORCH) {
                                        if (neighborFace.direction == YPOS) {
                                            pos -= glm::vec4(neighborFace.directionVec, 0.f) * (6.f/16.f);
                                        } else {
                                            pos -= glm::vec4(neighborFace.directionVec, 0.f) * (7.f/16.f);
                                        }
                                    } else if(curr == REDSTONE){
                                        pos += glm::vec4(0, 0.002f, 0, 0);
                                    }
                                    transparentVec.push_back(pos);
                                    faceIdxs.push_back(transparentVertCount);
                                    transparentVertCount++;

                                    glm::vec4 nor = glm::vec4(neighborFace.directionVec.x, neighborFace.directionVec.y, neighborFace.directionVec.z, 1.0f);

                                    if(curr == REDSTONE){
                                        nor.y = -nor.y;
                                    }

                                    transparentVec.push_back(nor);

                                    glm::vec2 blockFaceUV;

                                    if (curr == REDSTONE) {
                                        blockFaceUV = redstoneUVs.at(getRedstoneState(x, y, z)).at(neighborFace.direction) + dat.uv;
                                    } else {
                                        blockFaceUV = blockFaceUVs.at(curr).at(neighborFace.direction) + dat.uv;
                                    }

                                    glm::vec4 uvAndFlags = glm::vec4(blockFaceUV.x, blockFaceUV.y, 0.0f, 1.0f);

                                    if(curr == WATER){
                                        uvAndFlags[2] = 1.0f;
                                    } else if (curr == REDSTONE){
                                        RedstoneState state = getRedstoneState(x, y, z);
                                        if (unlitStates.count(state) == 1) {
                                            uvAndFlags[2] = 2.0f;
                                        } else if (litStates.count(state) == 1) {
                                            uvAndFlags[2] = 3.0f;
                                        }
                                    }
                                    transparentVec.push_back(uvAndFlags);
                                }

                                transparentIdx.push_back(faceIdxs[0]);
                                transparentIdx.push_back(faceIdxs[1]);
                                transparentIdx.push_back(faceIdxs[2]);
                                transparentIdx.push_back(faceIdxs[0]);
                                transparentIdx.push_back(faceIdxs[2]);
                                transparentIdx.push_back(faceIdxs[3]);
                            }

                    }
                }
            }
        }
    }
    return ChunkVBOData(this, vec, idx, transparentVec, transparentIdx);
}


void Chunk::setupVBO(std::vector<glm::vec4> &vec, std::vector<GLuint> &idx, std::vector<glm::vec4> &vec2, std::vector<GLuint> &idx2) {

    m_opaqueCount = idx.size();
    generateIdxOpaque();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpaque);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generateOpaque();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufOpaque);
    mp_context->glBufferData(GL_ARRAY_BUFFER, vec.size() * sizeof(glm::vec4), vec.data(), GL_STATIC_DRAW);

    m_transparentCount = idx2.size();
    generateIdxTransparent();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTransparent);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx2.size() * sizeof(GLuint), idx2.data(), GL_STATIC_DRAW);

    generateTransparent();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufTransparent);
    mp_context->glBufferData(GL_ARRAY_BUFFER, vec2.size() * sizeof(glm::vec4), vec2.data(), GL_STATIC_DRAW);

}

