#include "workers.h"
#include <iostream>
#include <QThread>

FBMWorker::FBMWorker(glm::ivec2 coords, std::vector<Chunk*>& toDo, std::unordered_set<Chunk*>* completed,
                     QMutex* completedLock, Terrain* terrain)
    : zoneX(coords.x), zoneZ(coords.y),
      m_toDo(toDo),
      m_completedChunks(completed), m_completedChunksLock(completedLock),
      m_terrain(terrain)
{}

void FBMWorker::run() {
    createTGZChunks(zoneX, zoneZ);
}

void FBMWorker::createTGZChunks(int worldX, int worldZ) {
    // Generate height field based on biome
    m_terrain->createTGZ(worldX, worldZ);

    // Update completed chunks
    for(Chunk* c : m_toDo) {
        m_completedChunksLock -> lock();
        m_completedChunks -> insert(c);
        m_completedChunksLock -> unlock();
    }
}


VBOWorker::VBOWorker(Chunk* chunk, std::vector<ChunkVBOData>* completed, QMutex* completedLock)
    : m_chunk(chunk),
      m_completedChunks(completed), m_completedChunksLock(completedLock)
{}

void VBOWorker::run() {
    m_completedChunksLock -> lock();
    m_completedChunks -> push_back(m_chunk->fillVBOdata());
    m_completedChunksLock -> unlock();
}


