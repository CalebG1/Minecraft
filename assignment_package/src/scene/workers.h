#pragma once
#include <QRunnable>
#include "chunk.h"
#include <unordered_set>
#include <QMutex>
#include "terrain.h"

class FBMWorker : public QRunnable {
private:
    int zoneX, zoneZ;
    std::vector<Chunk*> m_toDo;
    std::unordered_set<Chunk*>* m_completedChunks;
    QMutex* m_completedChunksLock;
    Terrain* m_terrain;
public:
    FBMWorker(glm::ivec2 coords, std::vector<Chunk*>& toDo, std::unordered_set<Chunk*>* completed,
              QMutex* completedLock, Terrain* terrain);
    void run() override;
    void createTGZChunks(int worldX, int worldZ);
};

class VBOWorker : public QRunnable {
private:
    Chunk* m_chunk;
    std::vector<ChunkVBOData>* m_completedChunks;
    QMutex* m_completedChunksLock;
public:
    VBOWorker(Chunk* chunk, std::vector<ChunkVBOData>* completed, QMutex* completedLock);
    void run() override;
};

