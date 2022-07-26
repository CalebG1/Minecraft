#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>
#include <QThreadPool>
#include <QDateTime>
#include "noisefunctions.h"
#include "workers.h"

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), m_redstoneToUpdate(), mp_context(context),
      chunksDone(0), renderRadius(1),
      m_chunksWithBlockData{}, m_chunksWithVBOData{}, m_cwbdLock(), m_cwvdLock(),
      m_expansionTimer(0.f), m_lastExpansionPos(glm::vec3(250.f, 175.f, 10.f)),
      inWater(false), inLava(false), waterSound(false)
{}

Terrain::~Terrain() {
//    m_geomCube.destroyVBOdata();
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

RedstoneState Terrain::getRedstoneState(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getRedstoneState(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

void Terrain::setRedstoneState(int x, int y, int z, RedstoneState s) {
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        if (c != nullptr) {
            c->setRedstoneState(static_cast<unsigned int>(x - chunkOrigin.x),
                          static_cast<unsigned int>(y),
                          static_cast<unsigned int>(z - chunkOrigin.y),
                          s);
        } else {
            throw std::out_of_range("no chunk");
        }

    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

void Terrain::updateRedstone(int x, int y, int z, std::vector<glm::vec3>* visited, std::unordered_set<Chunk*>* chunksToUpdate, bool updateVBOs) {
    // if this is a wire
        // check if it is reachable
        // if reachable, light it up and examine 4 steps around for wires, 6 steps for torches, 4 steps for lamps
        // if not reachable, unlight it and examine 4 steps around for wires, 6 steps for torches, 4 steps for lamps
    // if this is a torch
        // check if it is reachable
        // if reachable, light it up and examine 6 steps around for wires
        // if not reachable, unlight it and examine 6 steps around for wires
    // if this is a lamp
        // check if it is reachable
        // if reachable, light it
        // if not reachable, don't light it
    // if it is empty (meaning something was removed)
        // if there is a redstone object in any of 6 neighboring squares, update it.

    if (chunksToUpdate -> count(getChunkAt(x, z).get()) == 0) {
        chunksToUpdate -> insert(getChunkAt(x, z).get());
    }

    std::vector<glm::ivec3> steps = {glm::ivec3(1, 0, 0),
                                     glm::ivec3(-1, 0, 0),
                                     glm::ivec3(0, 0, 1),
                                     glm::ivec3(0, 0, -1),
                                     glm::ivec3(0, 1, 0),
                                     glm::ivec3(0, -1, 0)};

    visited -> push_back(glm::vec3(x, y, z));

    BlockType type = getBlockAt(x, y, z);
    if (type == REDSTONE) {
        RedstoneState state = getRedstoneState(x, y, z);
        std::vector<glm::vec3> visited2{};
        if (isReachable(x, y, z, &visited2)) {
            if (unlitStates.count(state) == 1) {
                setRedstoneState(x, y, z, oppositeStates.at(state));
            }
        } else {
            if (litStates.count(state) == 1) {
                setRedstoneState(x, y, z, oppositeStates.at(state));
            }
        }
        for (glm::ivec3& step : steps) {
            glm::vec3 neighbor = glm::vec3(x + step[0], y + step[1], z + step[2]);
            if (std::find(visited->begin(), visited->end(), neighbor) == visited->end()) {
                BlockType n_type = getBlockAt(neighbor);
                if (step[1] == 0) {
                    if (n_type == REDSTONE || n_type == UNLIT_REDSTONE_LAMP || n_type == LIT_REDSTONE_LAMP) {
                        updateRedstone(neighbor[0], neighbor[1], neighbor[2], visited, chunksToUpdate, false);
                        //m_redstoneToUpdate.push_back(std::pair<glm::vec3, std::vector<glm::vec3>>(
                                                         //glm::vec3(neighbor[0], neighbor[1], neighbor[2]), *visited));
                    }
                }
                if (n_type == UNLIT_REDSTONE_TORCH || n_type == LIT_REDSTONE_TORCH) {
                    updateRedstone(neighbor[0], neighbor[1], neighbor[2], visited, chunksToUpdate, false);
                    //m_redstoneToUpdate.push_back(std::pair<glm::vec3, std::vector<glm::vec3>>(
                                                     //glm::vec3(neighbor[0], neighbor[1], neighbor[2]), *visited));
                }
            }
        }
    } else if (type == UNLIT_REDSTONE_TORCH || type == LIT_REDSTONE_TORCH) {
        std::vector<glm::vec3> visited2{};
        if (isReachable(x, y, z, &visited2)) {
            if (type == UNLIT_REDSTONE_TORCH) {
                updateBlockAt(x, y, z, LIT_REDSTONE_TORCH);
            }
        } else {
            if (type == LIT_REDSTONE_TORCH) {
                updateBlockAt(x, y, z, UNLIT_REDSTONE_TORCH);
            }
        }
        for (glm::ivec3& step : steps) {
            glm::vec3 neighbor = glm::vec3(x + step[0], y + step[1], z + step[2]);
            if (std::find(visited->begin(), visited->end(), neighbor) == visited->end()) {
                BlockType n_type = getBlockAt(neighbor);
                if (n_type == REDSTONE) {
                    updateRedstone(neighbor[0], neighbor[1], neighbor[2], visited, chunksToUpdate, false);
                    // m_redstoneToUpdate.push_back(std::pair<glm::vec3, std::vector<glm::vec3>>(
                                                     //glm::vec3(neighbor[0], neighbor[1], neighbor[2]), *visited));
                }
            }
        }
    } else if (type == UNLIT_REDSTONE_LAMP || type == LIT_REDSTONE_LAMP) {
        std::vector<glm::vec3> visited2{};
        if (isReachable(x, y, z, &visited2)) {
            if (type == UNLIT_REDSTONE_LAMP) {
                updateBlockAt(x, y, z, LIT_REDSTONE_LAMP);
            }
        } else {
            if (type == LIT_REDSTONE_LAMP) {
                updateBlockAt(x, y, z, UNLIT_REDSTONE_LAMP);
            }
        }
    } else {
        for (glm::ivec3& step : steps) {
            glm::vec3 neighbor = glm::vec3(x + step[0], y + step[1], z + step[2]);
            if (std::find(visited->begin(), visited->end(), neighbor) == visited->end()) {
                BlockType n_type = getBlockAt(neighbor);
                if (n_type == REDSTONE || n_type == LIT_REDSTONE_TORCH || n_type == LIT_REDSTONE_LAMP) {
                    updateRedstone(neighbor[0], neighbor[1], neighbor[2], visited, chunksToUpdate, false);
                    //m_redstoneToUpdate.push_back(std::pair<glm::vec3, std::vector<glm::vec3>>(
                                                     //glm::vec3(neighbor[0], neighbor[1], neighbor[2]), *visited));
                }
            }
        }
    }

    if (updateVBOs) {
        for (Chunk* c : *chunksToUpdate) {
            c -> createVBOdata();
        }
    }
}

void Terrain::switchSwitch(int x, int y, int z) {
    std::vector<glm::ivec3> steps = {glm::ivec3(1, 0, 0),
                                     glm::ivec3(-1, 0, 0),
                                     glm::ivec3(0, 0, 1),
                                     glm::ivec3(0, 0, -1),
                                     glm::ivec3(0, 1, 0),
                                     glm::ivec3(0, -1, 0)};
    std::vector<glm::vec3> visited{};
    std::unordered_set<Chunk*> chunks{};
    if (getBlockAt(x, y, z) == SWITCH_OFF) {
        for (glm::ivec3& step : steps) {
            glm::vec3 neighbor = glm::vec3(x + step[0], y + step[1], z + step[2]);
            BlockType type = getBlockAt(neighbor[0], neighbor[1], neighbor[2]);
            if (type == REDSTONE || type == UNLIT_REDSTONE_TORCH || type == LIT_REDSTONE_TORCH) {
                updateRedstone(neighbor[0], neighbor[1], neighbor[2], &visited, &chunks, true);
                //m_redstoneToUpdate.push_back(std::pair<glm::vec3, std::vector<glm::vec3>>(
                                                 //glm::vec3(neighbor[0], neighbor[1], neighbor[2]), visited));
            }
        }
    } else if (getBlockAt(x, y, z) == SWITCH_ON) {
        for (glm::ivec3& step : steps) {
            glm::vec3 neighbor = glm::vec3(x + step[0], y + step[1], z + step[2]);
            BlockType type = getBlockAt(neighbor[0], neighbor[1], neighbor[2]);
            if (type == REDSTONE || type == LIT_REDSTONE_TORCH || type == UNLIT_REDSTONE_TORCH) {
                updateRedstone(neighbor[0], neighbor[1], neighbor[2], &visited, &chunks, true);
                //m_redstoneToUpdate.push_back(std::pair<glm::vec3, std::vector<glm::vec3>>(
                                                 //glm::vec3(neighbor[0], neighbor[1], neighbor[2]), visited));
            }
        }
    }
   getChunkAt(x, z) -> createVBOdata();
}

bool Terrain::isReachable(int x, int y, int z, std::vector<glm::vec3>* visited) {
    // if this is a wire
        // check 4 steps around for other wires, if any are reachable return true
        // if this is a straight wire or wire spot
            // check 6 steps around for other torches, if any are reachable return true
        // else (this is a junction wire)
            // check 6 steps around for at least two reachable redstone torches or wires, if so return true
        // check 6 steps around for on switches, if you find any return true
    // if this is a torch
        // check 6 steps around for other wires, if any are reachable return true
        // check 6 steps around for off switches, if you find any return true
        // check 6 steps around for on switches, if you find any return false
        // otherwise return true
    // if this is a lamp
        // check 4 steps around for other wires, if any are reachable return true
    // otherwise return false

    visited -> push_back(glm::vec3(x, y, z));

    BlockType type = getBlockAt(x, y, z);

    std::vector<glm::ivec3> steps = {glm::ivec3(1, 0, 0),
                                     glm::ivec3(-1, 0, 0),
                                     glm::ivec3(0, 0, 1),
                                     glm::ivec3(0, 0, -1)};

    std::vector<glm::ivec3> steps2 = {glm::ivec3(1, 0, 0),
                                     glm::ivec3(-1, 0, 0),
                                     glm::ivec3(0, 0, 1),
                                     glm::ivec3(0, 0, -1),
                                     glm::ivec3(0, 1, 0),
                                     glm::ivec3(0, -1, 0)};

    if (type == REDSTONE) {
        RedstoneState state = getRedstoneState(x, y, z);

        for (glm::ivec3& step : steps) {
            glm::vec3 neighbor = glm::vec3(x + step[0], y + step[1], z + step[2]);
            if (std::find(visited->begin(), visited->end(), neighbor) == visited->end()) {
                BlockType n_type = getBlockAt(neighbor);
                if (n_type == REDSTONE && isReachable(neighbor[0], neighbor[1], neighbor[2], visited)) {
                    return true;
                }
            }
        }

        if (oneTorchTrigger.count(state) == 1) {
            for (glm::ivec3& step : steps2) {
                glm::vec3 neighbor = glm::vec3(x + step[0], y + step[1], z + step[2]);
                if (std::find(visited->begin(), visited->end(), neighbor) == visited->end()) {
                    BlockType n_type = getBlockAt(neighbor);
                    if ((n_type == UNLIT_REDSTONE_TORCH || n_type == LIT_REDSTONE_TORCH) && isReachable(neighbor[0], neighbor[1], neighbor[2], visited)) {
                        return true;
                    }
                }
            }
        } else {
            int red_obj = 0;
            for (glm::ivec3& step : steps2) {
                glm::vec3 neighbor = glm::vec3(x + step[0], y + step[1], z + step[2]);
                if (std::find(visited->begin(), visited->end(), neighbor) == visited->end()) {
                    BlockType n_type = getBlockAt(neighbor);
                    if ((n_type == REDSTONE || n_type == UNLIT_REDSTONE_TORCH || n_type == LIT_REDSTONE_TORCH) && isReachable(neighbor[0], neighbor[1], neighbor[2], visited)) {
                        red_obj++;
                    }
                }
            }
            if (red_obj >= 2) {
                return true;
            }
        }

        for (glm::ivec3& step : steps) {
            glm::vec3 neighbor = glm::vec3(x + step[0], y + step[1], z + step[2]);
            if (std::find(visited->begin(), visited->end(), neighbor) == visited->end()) {
                BlockType n_type = getBlockAt(neighbor);
                if (n_type == SWITCH_ON) {
                    return true;
                }
            }
        }

    } else if (type == UNLIT_REDSTONE_TORCH || type == LIT_REDSTONE_TORCH) {
        bool powered_off = false;
        for (glm::ivec3& step : steps2) {
            glm::vec3 neighbor = glm::vec3(x + step[0], y + step[1], z + step[2]);
            if (std::find(visited->begin(), visited->end(), neighbor) == visited->end()) {
                BlockType n_type = getBlockAt(neighbor);
                if ((n_type == REDSTONE && isReachable(neighbor[0], neighbor[1], neighbor[2], visited))
                        || n_type == SWITCH_OFF) {
                    return true;
                } else if (n_type == SWITCH_ON) {
                    powered_off = true;
                }
            }
        }
        if (powered_off) {
            return false;
        }
        return true;
    } else if (type == UNLIT_REDSTONE_LAMP || type == LIT_REDSTONE_LAMP) {
        for (glm::ivec3& step : steps) {
            glm::vec3 neighbor = glm::vec3(x + step[0], y + step[1], z + step[2]);
            if (std::find(visited->begin(), visited->end(), neighbor) == visited->end()) {
                BlockType n_type = getBlockAt(neighbor);
                if (n_type == REDSTONE && isReachable(neighbor[0], neighbor[1], neighbor[2], visited)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void Terrain::delayRedstoneLighting(float dT) {
//    m_redstoneTimer += dT;
//    if (m_redstoneTimer > 0.2f) {
//        std::unordered_set<Chunk*> chunksToUpdate;
//        if (m_redstoneToUpdate.size() > 0) {
//            std::vector<std::pair<glm::vec3, std::vector<glm::vec3>>> toDo; // use new set so member set isn't being modified during loop
//            for (auto& block : m_redstoneToUpdate) {
//                toDo.push_back(block);
//                chunksToUpdate.insert(getChunkAt(block.first[0], block.first[2]).get());
//            }
//            m_redstoneToUpdate.clear();
//            for (auto& block : toDo) {
//                updateRedstone(block.first[0], block.first[1], block.first[2], &block.second);
//            }
//        }
//        for (auto& c : chunksToUpdate) {
//            c -> createVBOdata();
//        }
//        m_redstoneTimer = 0;
//    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    glm::vec3 rounded = glm::floor(p);
    return getBlockAt(rounded[0], rounded[1], rounded[2]);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        if (c != nullptr) {
            c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                          static_cast<unsigned int>(y),
                          static_cast<unsigned int>(z - chunkOrigin.y),
                          t);
        } else {
            throw std::out_of_range("no chunk");
        }

    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

void Terrain::setBlockAt(glm::vec3 p, BlockType t) {
    glm::vec3 rounded = glm::floor(p);
    setBlockAt(rounded[0], rounded[1], rounded[2], t);
}

void Terrain::updateBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        if (c != nullptr) {
            c->updateBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                          static_cast<unsigned int>(y),
                          static_cast<unsigned int>(z - chunkOrigin.y),
                          t);
        } else {
            throw std::out_of_range("no chunk");
        }

    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

void Terrain::updateBlockAt(glm::vec3 p, BlockType t) {
    glm::vec3 rounded = glm::floor(p);
    updateBlockAt(rounded[0], rounded[1], rounded[2], t);
}

void Terrain::placeBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        if (c != nullptr) {
            BlockType prev = getBlockAt(x, y, z);
            if (!(t == REDSTONE && (prev == WATER || prev == LAVA))){
                c->placeBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                              static_cast<unsigned int>(y),
                              static_cast<unsigned int>(z - chunkOrigin.y),
                              t);
                std::vector<glm::vec3> visited{};
                std::unordered_set<Chunk*> chunks{};
                if (t == REDSTONE || t == LIT_REDSTONE_TORCH || t == UNLIT_REDSTONE_TORCH || t == LIT_REDSTONE_LAMP || t == UNLIT_REDSTONE_LAMP
                        || prev == REDSTONE || prev == LIT_REDSTONE_TORCH || prev == UNLIT_REDSTONE_TORCH || prev == LIT_REDSTONE_LAMP || prev == UNLIT_REDSTONE_LAMP) {
                    updateJunctions(x, y, z, true);
                    updateRedstone(x, y, z, &visited, &chunks, true);
                    //m_redstoneToUpdate.push_back(std::pair<glm::vec3, std::vector<glm::vec3>>(
                                                     //glm::vec3(x, y, z), visited));
                } else if (transparentBlocks.count(getBlockAt(x, y+1, z)) == 1 && t == EMPTY) {
                    placeBlockAt(x, y+1, z, EMPTY);
                }
            }
        } else {
            throw std::out_of_range("no chunk");
        }
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

void Terrain::placeBlockAt(glm::vec3 p, BlockType t) {
    glm::vec3 rounded = glm::floor(p);
    placeBlockAt(rounded[0], rounded[1], rounded[2], t);
}

void Terrain::updateJunctions(int x, int y, int z, bool restart) { // won't update things on a hill
    BlockType left = getBlockAt(x+1, y, z);
    BlockType right = getBlockAt(x-1, y, z);
    BlockType forward = getBlockAt(x, y, z+1);
    BlockType backward = getBlockAt(x, y, z-1);

    if (getBlockAt(x, y, z) == REDSTONE) {
        if (((left == REDSTONE || left == UNLIT_REDSTONE_TORCH || left == LIT_REDSTONE_TORCH)
             || (right == REDSTONE || right == UNLIT_REDSTONE_TORCH || right == LIT_REDSTONE_TORCH))
                && ((forward == REDSTONE || forward == UNLIT_REDSTONE_TORCH || forward == LIT_REDSTONE_TORCH)
                    || (backward == REDSTONE || backward == UNLIT_REDSTONE_TORCH || backward == LIT_REDSTONE_TORCH))) {
            int lit_obj = 0;
            if ((left == REDSTONE && litStates.count(getRedstoneState(x+1, y, z)) == 1)
                    || left == LIT_REDSTONE_TORCH) {
                lit_obj++;
            }
            if ((right == REDSTONE && litStates.count(getRedstoneState(x-1, y, z)) == 1)
                    || right == LIT_REDSTONE_TORCH) {
                lit_obj++;
            }
            if ((forward == REDSTONE && litStates.count(getRedstoneState(x, y, z+1)) == 1)
                    || forward == LIT_REDSTONE_TORCH) {
                lit_obj++;
            }
            if ((backward == REDSTONE && litStates.count(getRedstoneState(x, y, z-1)) == 1)
                    || backward == LIT_REDSTONE_TORCH) {
                lit_obj++;
            }
            if (lit_obj >= 2) {
                setRedstoneState(x, y, z, LIT_WIRE_JUNCTION);
            } else {
                setRedstoneState(x, y, z, UNLIT_WIRE_JUNCTION);
            }
        } else if ((left == REDSTONE || left == UNLIT_REDSTONE_TORCH || left == LIT_REDSTONE_TORCH)
                   || (right == REDSTONE || right == UNLIT_REDSTONE_TORCH || right == LIT_REDSTONE_TORCH)) {
            if (((left == REDSTONE && litStates.count(getRedstoneState(x+1, y, z)) == 1) || left == LIT_REDSTONE_TORCH)
                    || ((right == REDSTONE && litStates.count(getRedstoneState(x-1, y, z)) == 1) || right == LIT_REDSTONE_TORCH)) {
                setRedstoneState(x, y, z, LIT_WIRE_STRAIGHT_HOR);
            } else {
                setRedstoneState(x, y, z, UNLIT_WIRE_STRAIGHT_HOR);
            }
        } else if ((forward == REDSTONE || forward == UNLIT_REDSTONE_TORCH || forward == LIT_REDSTONE_TORCH)
                   || (backward == REDSTONE || backward == UNLIT_REDSTONE_TORCH || backward == LIT_REDSTONE_TORCH)) {
            if (((forward == REDSTONE && litStates.count(getRedstoneState(x, y, z+1)) == 1) || forward == LIT_REDSTONE_TORCH)
                    || ((backward == REDSTONE && litStates.count(getRedstoneState(x, y, z-1)) == 1) || backward == LIT_REDSTONE_TORCH)) {
                setRedstoneState(x, y, z, LIT_WIRE_STRAIGHT_VER);
            } else {
                setRedstoneState(x, y, z, UNLIT_WIRE_STRAIGHT_VER);
            }
        } else {
            setRedstoneState(x, y, z, UNLIT_WIRE_SPOT);
        }
    }

    if (restart == true) {
        std::vector<glm::vec3> steps = {glm::vec3(1, 0, 0),
                                        glm::vec3(-1, 0, 0),
                                        glm::vec3(0, 0, 1),
                                        glm::vec3(0, 0, -1)};
        for (glm::vec3 step : steps) {
            if (getBlockAt(x + step[0], y + step[1], z + step[2]) == REDSTONE) {
                updateJunctions(x + step[0], y + step[1], z + step[2], false);
                if (getChunkAt(x, z) != getChunkAt(x + step[0], z + step[2])) {
                    getChunkAt(x + step[0], z + step[2]) -> createVBOdata(); // ensures junctions across chunks are updated
                }
            }
        }
        //getChunkAt(x, z) -> createVBOdata();
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context, x, z);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }

    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
//    m_geomCube.clearOffsetBuf();
//    m_geomCube.clearColorBuf();
//    std::vector<glm::vec3> offsets, colors;

    for(int z = minZ; z < maxZ; z += 16) {
        for(int x = minX; x < maxX; x += 16) {
            if(hasChunkAt(x,z)){
                const uPtr<Chunk> &chunk = getChunkAt(x, z);
                if(chunk->shouldUpdateVBO){
                    //chunk->createVBOdata();
                    m_cwbdLock.lock();
                    m_chunksWithBlockData.insert(chunk.get());
                    m_cwbdLock.unlock();
                    chunk->shouldUpdateVBO = false;
                } else {
                }
                shaderProgram->setModelMatrix(glm::mat4(1.f, 0.f, 0.f, 0.f,
                                                        0.f, 1.0f, 0.f, 0.f,
                                                        0.f, 0.f, 1.f, 0.f,
                                                        x, 0.f, z, 1.f));
                if (chunk -> elemCount() > 0) {
                    shaderProgram->drawInterleaved(*chunk, false);
                }
            }
        }
    }

    for(int z = minZ; z < maxZ; z += 16) {
        for(int x = minX; x < maxX; x += 16) {
            if(hasChunkAt(x,z)){
                const uPtr<Chunk> &chunk = getChunkAt(x, z);
                shaderProgram->setModelMatrix(glm::mat4(1.f, 0.f, 0.f, 0.f,
                                                        0.f, 1.0f, 0.f, 0.f,
                                                        0.f, 0.f, 1.f, 0.f,
                                                        x, 0.f, z, 1.f));
                if (chunk -> elemCount() > 0) {
                    shaderProgram->drawInterleaved(*chunk, true);
                }
            }
        }
    }

//    m_geomCube.createInstancedVBOdata(offsets, colors);
//    shaderProgram->drawInstanced(m_geomCube);
}

///
/// \brief Terrain::populateNeighboringTGZ
/// \param x
/// \param z
///
void Terrain::populateNeighboringTGZ(float x, float z)
{
    glm::ivec2 tgzPos = getTGZCoord(x,z);
    // now get left lower corner of the left lower neighboring TGZ
    int xPos = tgzPos.x - 64;
    int zPos = tgzPos.y - 64;

    // If the 3x3 bounding TGZs are not created, create them
    if (true)   createTGZWithChunks(xPos,zPos + 128);
    if (true)   createTGZWithChunks(xPos,zPos + 64);
    if (true)   createTGZWithChunks(xPos,zPos);
    if (true)   createTGZWithChunks(xPos+64,zPos+64);
    if (true)   createTGZWithChunks(xPos + 64, zPos + 128);
    if (true)   createTGZWithChunks(xPos + 64,zPos);
    if (true)   createTGZWithChunks(xPos + 128,zPos + 128);
    if (true)   createTGZWithChunks(xPos + 128,zPos + 64);
    if (true)   createTGZWithChunks(xPos + 128,zPos);
}


void Terrain::terrainExpansion(glm::vec3 pos) {
//    for(int x = -1; x < 2; x++){
//        for(int z = -1; z < 2; z++){
    for(int x = -3; x < 4; x++){
        for(int z = -4; z < 4; z++){
                int currX = pos.x - 16 * x;
                int currZ = pos.z - 16 * z;
                if(!hasChunkAt(currX, currZ)){
                    instantiateChunkAt(currX, currZ);

                    glm::ivec2 tgzPos = getTGZCoord(currX,currZ);

                    m_generatedTerrain.insert(toKey(tgzPos[0], tgzPos[1]));

                    // create terrain for TGZ
                    createTGZTerrain(tgzPos[0], tgzPos[1]);
                }
        }
    }
}

void Terrain::multithreadExpansion(glm::vec3 pos, float dT) {
    m_expansionTimer += dT;
    if (m_expansionTimer > 1.f) { // change to 1 second
        tryExpansion(pos, m_lastExpansionPos);
        m_lastExpansionPos = pos;
        checkThreads();
        m_expansionTimer = 0.f;
    }
}

void Terrain::tryExpansion(glm::vec3 pos, glm::vec3 prevPos) {
    std::unordered_set<int64_t> prevZones{};
    std::unordered_set<int64_t> currZones{};
    // Populate sets with 5x5 grid of zone ids
    for (int x = -renderRadius; x <= renderRadius; x++) {
        for (int z = -renderRadius; z <= renderRadius; z++) {
            glm::ivec2 zone = getTGZCoord(pos[0] + 64 * x, pos[2] + 64 * z);
            glm::ivec2 prevZone = getTGZCoord(prevPos[0] + 64 * x, prevPos[2] + 64 * z);
            prevZones.insert(toKey(prevZone.x, prevZone.y));
            currZones.insert(toKey(zone.x, zone.y));
        }
    }


    for (auto zone : prevZones) { // check which zones need to be deleted
        if (currZones.count(zone) == 0) {
            glm::ivec2 tgzCoord = toCoords(zone);
            for (int x = 0; x < 64; x += 16) {
                for (int z = 0; z < 64; z += 16) {
                    getChunkAt(tgzCoord[0] + x, tgzCoord[1] + z) -> destroyVBOdata();
                }
            }
        }
    }

    std::unordered_set<int64_t> fbmZones;
    for (/* Get in the zone */ auto zone : currZones) { // check which zones need to be restored/created
        glm::ivec2 tgzCoord = toCoords(zone);
        if (hasChunkAt(tgzCoord.x, tgzCoord.y)) { // if zone is already generated we can create VBO worker for it
            if (prevZones.count(zone) == 0) { // make sure this is a new zone
                for (int x = 0; x < 64; x += 16) {
                    for (int z = 0; z < 64; z += 16) {
                        auto &chunk = getChunkAt(tgzCoord[0] + x, tgzCoord[1] + z);
                        spawnVBOWorker(chunk.get());
                    }
                }
            }
        } else { // if zone isn't already generated, generate it with FBM worker
            fbmZones.insert(zone);
        }
    }

    spawnFBMWorkers(fbmZones);
}

void Terrain::spawnFBMWorker(int64_t zone, std::vector<Chunk*>& toDo) {
    FBMWorker *worker = new FBMWorker(toCoords(zone), toDo, &m_chunksWithBlockData, &m_cwbdLock, this);
    QThreadPool::globalInstance() -> start(worker);
}

void Terrain::spawnFBMWorkers(std::unordered_set<int64_t>& zones) {
    for (int64_t zone : zones) { // instantiate all chunks first to prevent threads from editing chunks while instantiating
        glm::ivec2 tgzCoord = toCoords(zone);
        for (int x = 0; x < 64; x += 16) {
            for (int z = 0; z < 64; z += 16) {
                if (!hasChunkAt(tgzCoord.x + x, tgzCoord.y + z)) {
                    instantiateChunkAt(tgzCoord.x + x, tgzCoord.y + z);
                }
            }
        }
    }
    for (int64_t zone : zones) { // spawn fbm workers
        glm::ivec2 tgzCoord = toCoords(zone);
        std::vector<Chunk*> newChunks;
        for (int x = 0; x < 64; x += 16) {
            for (int z = 0; z < 64; z += 16) {
                Chunk* newChunk = getChunkAt(tgzCoord.x + x, tgzCoord.y + z).get();
                newChunks.push_back(newChunk);
            }
        }
        spawnFBMWorker(zone, newChunks);
    }
}

void Terrain::spawnVBOWorker(Chunk* chunk) {
    VBOWorker *worker = new VBOWorker(chunk, &m_chunksWithVBOData, &m_cwvdLock);
    worker -> setAutoDelete(true);
    QThreadPool::globalInstance() -> start(worker);
}

void Terrain::checkThreads() {
    m_cwbdLock.lock();
    for (Chunk* c : m_chunksWithBlockData) {
        spawnVBOWorker(c);
    }
    m_chunksWithBlockData.clear();
    m_cwbdLock.unlock();

    m_cwvdLock.lock();
    for (ChunkVBOData& d : m_chunksWithVBOData) {
        d.mp_chunk -> setupVBO(d.m_vbo, d.m_idx, d.m_tvbo, d.m_tidx);
        chunksDone++;
    }
    m_chunksWithVBOData.clear();
    m_cwvdLock.unlock();
}

void Terrain::BiomeUnderground(int x, int z, bool (&solidOrNot)[140])
{
    for (int y = 0; y < 130; y++) {
        if (solidOrNot[y] == 1) {
            setBlockAt(x,y,z, STONE);
        } else {
            if (y < 35) {
                setBlockAt(x,y,z, LAVA);
            } else {
                setBlockAt(x,y,z, EMPTY);
            }
        }
    }
}


void Terrain::BiomeMountain(int x, int z, float h, bool (&solidOrNot) [140])
{
    // REDSTONE
    for(int y = 120; y < h; y++) {
        setBlockAt(x,y,z,STONE);
    }
    if (h > 142) {
        setBlockAt(x,142,z,COOLSTONE);
    }
//    for(int y = 120; y < h; y++) {
//        setBlockAt(x,y,z,STONE);
//    }
    if (h < 139) {
        for (int i = h; i < 135; ++i) {
            setBlockAt(x,i,z, WATER);
        }
    }
    if (h > 200) setBlockAt(x,h+1,z, SNOW);
    BiomeUnderground(x,z,solidOrNot);
    setBlockAt(x,0,z, BEDROCK);
}

void Terrain::BiomeGrass(int x, int z, float h, bool (&solidOrNot) [140])
{
    // Set dirt for all y levels below final height
    for(int y = 120; y < h - 1; y++) {
        setBlockAt(x,y,z,DIRT);
    }
    for (int i = h; i < 135; ++i) {
        setBlockAt(x,i,z, WATER);
    }
     BiomeUnderground(x,z,solidOrNot);
    if (((h > 135) && (h < 137))|| ((h > 141) && (h < 143))) {
//    if (h==136 || h==137 || h==142) {
        setBlockAt(x,h,z,DIRT);
    } else {
        setBlockAt(x,h,z,GRASS);
    }
    setBlockAt(x,0,z, BEDROCK);
}

void Terrain::BiomeDesert(int x, int z, float h, bool (&solidOrNot) [140])
{
    for(int y = 120; y < h; y++) {
        if (y > 140) {
            setBlockAt(x,y,z, DESERT2);

        } else {
            setBlockAt(x,y,z, DESERT);
        }
    }
    BiomeUnderground(x,z,solidOrNot);
    setBlockAt(x,0,z, BEDROCK);
}

void Terrain::BiomeForrest(int x, int z, float height, bool (&solidOrNot) [140])
{
    // Set dirt for all y levels below final height
    for(int y = 120; y < height - 1; y++) {
        setBlockAt(x,y,z,DIRT);
    }
    for (int i = height; i < 135; ++i) {
        setBlockAt(x,i,z, WATER);
    }
     BiomeUnderground(x,z,solidOrNot);
    setBlockAt(x,height,z,GRASS);
    setBlockAt(x,0,z, BEDROCK);
}

BlockType Terrain::getBiomeType(int x, int z)
{
    float h = m_biomes.getHeight(x,z);
    float m = m_biomes.getMoisture(x,z);
    if (h<=5)  return WATER;
    if (h<=15)  {
        if (m < .25)  return DESERT;
        if (m > .75)  return FORREST;
        return GRASS;
    }
    if (h>72 && m>.25)  return SNOW;
    return STONE;
}

///
/// \brief Terrain::spawnTreeLocations
/// \param x origin of TGZ
/// \param z origin of TGZ
/// \return fills world coord (x,z) with empty tuple - to be filled later
///
std::unordered_map<int64_t, std::tuple<int, BiomeType>> Terrain::spawnTreeLocations(int x, int z) {
    // gridify the entire terrain zone cornered at (x,z)
    // generate a vec3 p for each cell
    // if p.z > threshold, place a tree at p.yz
    std::unordered_map<int64_t, std::tuple<int, BiomeType>> result;
    result.reserve(256);
    // 64x64 blocks
    // 8x8 cells is every 8 blocks
    // 16x16 cells is every 4 blocks
    int gridDensity = 8;
    int cellSize = 64 / gridDensity;

    for (int i=0; i< 64; i+=cellSize)  {
        for (int k =0; k<64; k+=cellSize)  {
            glm::vec3 p = m_biomes.noise.random3from2(glm::vec2(x+i, z+k));
            if(p.z < 0.3)  {
                result.insert({toKey(x+i+p.x*cellSize, z+k+p.y*cellSize),{0,GRASSLAND}});
            }
        }
    }
    return result;
}

std::vector<std::tuple<glm::ivec3, BlockType> > Terrain::generateTree(glm::ivec3 location, BiomeType b) const
{
    std::vector<std::tuple<glm::ivec3, BlockType>> result;
    switch(b)  {
        case GRASSLAND:
            location.y = location.y +1;
            return pineTree(location);
            break;
        case FORRESTLAND:
            location.y = location.y +1;
            return birchTree(location);
            break;
        case MOUNTAINLAND:
            location.y = location.y +1;
            return oakTree(location);
            break;
        case DESERTLAND:
            return Cactus(location);
            break;
    }
    return result;

}

std::vector<std::tuple<glm::ivec3, BlockType> > Terrain::pineTree(glm::ivec3 location) const
{
    std::vector<std::tuple<glm::ivec3,BlockType>> result;
    // 7 high trunk
    for(int i=0;i<7;++i) {
        result.push_back({location+glm::ivec3(0,i,0), WOOD_PINE});
    }
    // 4 high canopy
    // 3 width bottom 2 canopy
    for(int x = -2; x <= 2; ++x)  {
        for(int z=-2; z<=2; ++z)  {
            for(int y=4;y<=5;++y) {
                if(!((glm::abs(x)==2 && glm::abs(z)==2) || (x==0 && z==0))) {
                    result.push_back({location+glm::ivec3(x,y,z), LEAF_PINE});
                }
            }
        }
    }
    // 2 width above 3w
    for(int x=-1;x<=1;++x) {
        for(int z=-1;z<=1;++z) {
            if(!(x==0 && z==0)) {
                result.push_back({location+glm::ivec3(x,6,z), LEAF_PINE});
            }
        }
    }
    // + shaape at top
    for(int x=-1;x<=1;++x) {
        for(int z=-1;z<=1;++z) {
            if(!(glm::abs(x)==1 && glm::abs(z)==1)) {
                result.push_back({location+glm::ivec3(x,7,z), LEAF_PINE});
            }
        }
    }
    return result;
}

std::vector<std::tuple<glm::ivec3, BlockType> > Terrain::oakTree(glm::ivec3 location) const
{
    std::vector<std::tuple<glm::ivec3,BlockType>> result;
    // 7 high trunk
    for(int i=0;i<7;++i) {
        result.push_back({location+glm::ivec3(0,i,0), WOOD_OAK});
    }
    // 4 high canopy
    // 3 width bottom 2 canopy
    for(int x = -2; x <= 2; ++x)  {
        for(int z=-2; z<=2; ++z)  {
            for(int y=4;y<=5;++y) {
                if(!((glm::abs(x)==2 && glm::abs(z)==2) || (x==0 && z==0))) {
                    result.push_back({location+glm::ivec3(x,y,z), LEAF_OAK});
                }
            }
        }
    }
    // 2 width above 3w
    for(int x=-1;x<=1;++x) {
        for(int z=-1;z<=1;++z) {
            if(!(x==0 && z==0)) {
                result.push_back({location+glm::ivec3(x,6,z), LEAF_OAK});
            }
        }
    }
    // + shaape at top
    for(int x=-1;x<=1;++x) {
        for(int z=-1;z<=1;++z) {
            if(!(glm::abs(x)==1 && glm::abs(z)==1)) {
                result.push_back({location+glm::ivec3(x,7,z), LEAF_OAK});
            }
        }
    }
    return result;
}

std::vector<std::tuple<glm::ivec3, BlockType> > Terrain::birchTree(glm::ivec3 location) const
{
    std::vector<std::tuple<glm::ivec3,BlockType>> result;
    // 7 high trunk
    for(int i=0;i<7;++i) {
        result.push_back({location+glm::ivec3(0,i,0), WOOD_BIRCH});
    }
    // 4 high canopy
    // 3 width bottom 2 canopy
    for(int x = -2; x <= 2; ++x)  {
        for(int z=-2; z<=2; ++z)  {
            for(int y=4;y<=5;++y) {
                if(!((glm::abs(x)==2 && glm::abs(z)==2) || (x==0 && z==0))) {
                    result.push_back({location+glm::ivec3(x,y,z), LEAF_BIRCH});
                }
            }
        }
    }
    // 2 width above 3w
    for(int x=-1;x<=1;++x) {
        for(int z=-1;z<=1;++z) {
            if(!(x==0 && z==0)) {
                result.push_back({location+glm::ivec3(x,6,z), LEAF_BIRCH});
            }
        }
    }
    // + shaape at top
    for(int x=-1;x<=1;++x) {
        for(int z=-1;z<=1;++z) {
            if(!(glm::abs(x)==1 && glm::abs(z)==1)) {
                result.push_back({location+glm::ivec3(x,7,z), LEAF_BIRCH});
            }
        }
    }
    return result;

}

std::vector<std::tuple<glm::ivec3, BlockType> > Terrain::Cactus(glm::ivec3 location) const
{
    float p = m_biomes.noise.random1(glm::vec2(location.x,location.z));
    if(p < 0.1)  {
        return Pyramid(location);
    }

    std::vector<std::tuple<glm::ivec3,BlockType>> result;

    // level 1
    result.push_back({location+glm::ivec3(-1,0,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,0,-1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,0,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-1,0,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(0,0,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(1,0,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(-1,0,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,0,1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,0,1), CACTUS_LIGHTGREEN});
    // level 2
    result.push_back({location+glm::ivec3(-1,1,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,1,-1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,1,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-1,1,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(0,1,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(1,1,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(-1,1,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,1,1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,1,1), CACTUS_LIGHTGREEN});
    // level 3
    result.push_back({location+glm::ivec3(-1,2,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,2,-1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,2,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-1,2,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(0,2,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(1,2,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(-1,2,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,2,1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,2,1), CACTUS_LIGHTGREEN});
    // level 4
    result.push_back({location+glm::ivec3(-1,3,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,3,-1), CACTUS_BROWN});
    result.push_back({location+glm::ivec3(1,3,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-1,3,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(0,3,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(1,3,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(-1,3,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,3,1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,3,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(3,3,0), CACTUS_LIGHTGREEN});
    // level 5
    result.push_back({location+glm::ivec3(-1,4,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,4,-1), CACTUS_BROWN});
    result.push_back({location+glm::ivec3(1,4,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-1,4,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(0,4,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(1,4,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(-1,4,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,4,1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,4,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(3,4,0), CACTUS_LIGHTGREEN});
    // level 6
    result.push_back({location+glm::ivec3(-1,5,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,5,-1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,5,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-1,5,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(0,5,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(1,5,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(-1,5,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,5,1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,5,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(2,5,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(3,5,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-2,5,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-3,5,0), CACTUS_LIGHTGREEN});
    // level 7
    result.push_back({location+glm::ivec3(-1,6,-1), CACTUS_BROWN});
    result.push_back({location+glm::ivec3(0,6,-1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,6,-1), CACTUS_BROWN});
    result.push_back({location+glm::ivec3(-1,6,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(0,6,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(1,6,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(-1,6,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,6,1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,6,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(2,6,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(3,6,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-2,6,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-3,6,0), CACTUS_LIGHTGREEN});
    // level 8
    result.push_back({location+glm::ivec3(-1,7,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,7,-1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,7,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-1,7,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(0,7,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(1,7,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(-1,7,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,7,1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,7,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-3,7,0), CACTUS_LIGHTGREEN});
    // level 9
    result.push_back({location+glm::ivec3(-1,8,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,8,-1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,8,-1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-1,8,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(0,8,0), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(1,8,0), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(-1,8,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(0,8,1), CACTUS_DARKGREEN});
    result.push_back({location+glm::ivec3(1,8,1), CACTUS_LIGHTGREEN});
    result.push_back({location+glm::ivec3(-3,8,0), CACTUS_LIGHTGREEN});

    return result;

}

void Terrain::drawXLine(std::vector<std::tuple<glm::ivec3, BlockType>> &o, glm::ivec3 origin, int len, BlockType b) const
{
    for(int x=0;x<len;x++)  {
        o.push_back({origin+glm::ivec3(x,0,0), b});
    }
}
void Terrain::drawZLine(std::vector<std::tuple<glm::ivec3, BlockType>> &o, glm::ivec3 origin, int len, BlockType b) const
{
    for(int z=0;z<len;z++)  {
        o.push_back({origin+glm::ivec3(0,0,z), b});
    }
}
std::vector<std::tuple<glm::ivec3, BlockType> > Terrain::Pyramid(glm::ivec3 location) const
{
    std::vector<std::tuple<glm::ivec3,BlockType>> result;
    int height = 5;
    for(int y=0;y<height;y++)  {
        for(int n=-(height-y); n<=(height-y); n++)  {
            result.push_back({location+glm::ivec3(n,y,(height-y)), BEDROCK});
            result.push_back({location+glm::ivec3(n,y,(y-height)), BEDROCK});
            result.push_back({location+glm::ivec3((height-y),y,n), COOLSTONE});
            result.push_back({location+glm::ivec3((y-height),y,n), COOLSTONE});
        }
    }
    return result;
}
std::vector<std::tuple<glm::ivec3, BlockType> > Terrain::Sphinx(glm::ivec3 location) const
{
    std::vector<std::tuple<glm::ivec3,BlockType>> result;
    int x,y,z;
    // level 1
    y=0;
    for(z=0;z<39;z++) {
        x=0;
        result.push_back({location+glm::ivec3(x,y,z), BEDROCK});
        x=14;
        result.push_back({location+glm::ivec3(x,y,z), BEDROCK});
    }
    for(x=0;x<15;x++)  {
        z=0;
        result.push_back({location+glm::ivec3(x,y,z), BEDROCK});
        z=38;
        result.push_back({location+glm::ivec3(x,y,z), BEDROCK});
    }
    // level 2
    y=1;
    for(z=0;z<39;z++) {
        x=0;
        result.push_back({location+glm::ivec3(x,y,z), COOLSTONE});
        x=14;
        result.push_back({location+glm::ivec3(x,y,z), COOLSTONE});
    }
    for(x=0;x<15;x++)  {
        z=0;
        result.push_back({location+glm::ivec3(x,y,z), COOLSTONE});
        z=38;
        result.push_back({location+glm::ivec3(x,y,z), COOLSTONE});
    }
    return result;
}

void Terrain::createTGZTerrain(int worldX, int worldZ)
{
    // get TGZ origin
    glm::ivec2 tgzOrigin = getTGZCoord(worldX, worldZ);

    // contains randomly generated tree locations
    // int64_t is the (x,z), user toKey(x,z) and toCoords(k)
    // tuple: get<0> gets y height;  get<1> gets BiomeType
    std::unordered_map<int64_t, std::tuple<int, BiomeType>> treeLocations = spawnTreeLocations(tgzOrigin.x,tgzOrigin.y);

    for(int x = worldX; x < worldX+64; ++x) {
        for(int z = worldZ; z < worldZ+64; ++z) {
            BiomeType btype;
            float height;
            // get y height and BiomeType
            m_biomes.getBiomeandHeight(x,z,height,btype);

            // caves
            bool solidOrNot [140];
            m_biomes.getCaveSystem(x, z, solidOrNot);

            // if there is a tree at this (x,z) fill in height and BiomeType info
            auto it = treeLocations.find(toKey(x,z));
            if(it != treeLocations.end()) {
                it->second = {height,btype};
            }

            if (btype == GRASSLAND)  {
                BiomeGrass(x,z,height, solidOrNot);
            } else if (btype == FORRESTLAND)  {
                BiomeForrest(x,z,height, solidOrNot);
            } else if (btype == DESERTLAND) {
                BiomeDesert(x,z,height, solidOrNot);
            } else if (btype == MOUNTAINLAND)  {
                BiomeMountain(x,z,height, solidOrNot);
            }
        }
    }

    // Draw Trees
    for (auto &kvp : treeLocations) {
         // get world coord (x,z)
         glm::vec2 coords = glm::vec2(toCoords(kvp.first));
         if (hasChunkAt(coords.x, coords.y))  {
             glm::ivec2 chunkCoord = glm::ivec2(glm::floor(coords/16.f)*16.f);
             uPtr<Chunk> &c = getChunkAt(coords.x, coords.y);

             // get height
             int y = std::get<0>(kvp.second);
             // don't draw if too high of an elevation or too low
             if(y<160 && y>135)  {
                 BiomeType b = std::get<1>(kvp.second);
                 std::vector<std::tuple<glm::ivec3,BlockType>> treeBlocks = generateTree(glm::ivec3(coords.x, y, coords.y),b);
                 // sample draw vertical
                 for(auto &element: treeBlocks) {
                     glm::ivec3 localPos = std::get<0>(element) - glm::ivec3(chunkCoord.x, 0, chunkCoord.y);
                     localPos.y = glm::clamp(localPos.y,0,255);
                     c->setBlockAtNeighbor(localPos.x, localPos.y, localPos.z, std::get<1>(element));
                 }
             }

         }

    }
}

///
/// \brief Terrain::createTGZ
/// \param worldX
/// \param worldZ
/// \param t
///  Create one Terrain Generation Zone (TGZ) which is
///    (x,z): 64x64 blocks or 4x4 Chunks
void Terrain::createTGZ(int worldX, int worldZ)
{
    glm::ivec2 tgzCoord = getTGZCoord(worldX, worldZ);
    m_generatedTerrain.insert(toKey(tgzCoord.x, tgzCoord.y));

    // create terrain for TGZ
    createTGZTerrain(worldX, worldZ);
}

void Terrain::createTGZWithChunks(int worldX, int worldZ)
{
    // Tell our existing terrain set that
    // the "generated terrain zone"
    if (hasChunkAt(worldX,worldZ)) {
        std::cout << "Chunk already created at (" << worldX << ", " << worldZ << ")" << std::endl;
        return;
    }

    for(int x = worldX; x < worldX + 64; x += 16) {
        for(int z = worldZ; z < worldZ + 64; z += 16) {
            if (!hasChunkAt(x, z)) {
                instantiateChunkAt(x, z);
            }
        }
    }

    glm::ivec2 tgzCoord = getTGZCoord(worldX, worldZ);
    m_generatedTerrain.insert(toKey(tgzCoord.x, tgzCoord.y));

    // create terrain for TGZ
    createTGZTerrain(worldX, worldZ);
}

///
/// \brief Terrain::getTGZCoord
/// \param x
/// \param z
/// \return ivec2 - left lower coord of TGZ
///
glm::ivec2 Terrain::getTGZCoord(float x, float z)
{
    // find the left lower corner of the TGZ
    int xFloor = 16 * static_cast<int>(glm::floor(x / 16.f));
    int zFloor = 16 * static_cast<int>(glm::floor(z / 16.f));
    int xOffset = xFloor % (4 * 16);
    int zOffset = zFloor % (4 * 16);
    int xPos = xFloor;
    int zPos = zFloor;
    if (xOffset < 0) {
        xPos -= 64 + xOffset;
    } else {
        xPos -= xOffset;
    }
    if (zOffset < 0) {
        zPos -= 64 + zOffset;
    } else {
        zPos -= zOffset;
    }
    return glm::ivec2(xPos,zPos);
}


///
/// \brief Terrain::CreateInitScene
/// \param m_terrain
/// \param x
/// \param z
///
void Terrain::CreateInitScene(float x, float z)
{
     // TODO: DELETE THIS LINE WHEN YOU DELETE m_geomCube!
//     m_geomCube.createVBOdata();

    populateNeighboringTGZ(x,z);
}


void Terrain::CreateTestScene()
{
    // TODO: DELETE THIS LINE WHEN YOU DELETE m_geomCube!

    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            instantiateChunkAt(x, z);
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(0, 0));

    // Create the basic terrain floor
    for(int x = 0; x < 64; ++x) {
        for(int z = 0; z < 64; ++z) {
            if((x + z) % 2 == 0) {
                setBlockAt(x, 128, z, STONE);
            }
            else {
                setBlockAt(x, 128, z, DIRT);
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x) {
        setBlockAt(x, 129, 0, GRASS);
        setBlockAt(x, 130, 0, GRASS);
        setBlockAt(x, 129, 63, GRASS);
        setBlockAt(0, 130, x, GRASS);
    }
    // Add a central column
    for(int y = 129; y < 140; ++y) {
        setBlockAt(32, y, 32, GRASS);
    }
}
