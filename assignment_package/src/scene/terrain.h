
#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "chunk.h"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include "shaderprogram.h"
#include "cube.h"
#include "biomes.h"
#include <QMutex>

//using namespace std;

const static std::unordered_map<RedstoneState, RedstoneState> oppositeStates {
    {UNLIT_WIRE_SPOT, LIT_WIRE_SPOT},
    {UNLIT_WIRE_STRAIGHT_HOR, LIT_WIRE_STRAIGHT_HOR},
    {UNLIT_WIRE_STRAIGHT_VER, LIT_WIRE_STRAIGHT_VER},
    {UNLIT_WIRE_JUNCTION, LIT_WIRE_JUNCTION},
    {LIT_WIRE_SPOT, UNLIT_WIRE_SPOT},
    {LIT_WIRE_STRAIGHT_HOR, UNLIT_WIRE_STRAIGHT_HOR},
    {LIT_WIRE_STRAIGHT_VER, UNLIT_WIRE_STRAIGHT_VER},
    {LIT_WIRE_JUNCTION, UNLIT_WIRE_JUNCTION}
};

const static std::unordered_set<RedstoneState> oneTorchTrigger {
    {UNLIT_WIRE_SPOT, LIT_WIRE_SPOT,
    UNLIT_WIRE_STRAIGHT_HOR, LIT_WIRE_STRAIGHT_HOR,
    UNLIT_WIRE_STRAIGHT_VER, LIT_WIRE_STRAIGHT_VER}
};

// Helper functions to convert (x, z) to and from hash map key
int64_t toKey(int x, int z);
glm::ivec2 toCoords(int64_t k);

// The container class for all of the Chunks in the game.
// Ultimately, while Terrain will always store all Chunks,
// not all Chunks will be drawn at any given time as the world
// expands.
class Terrain {
private:
    // Stores every Chunk according to the location of its lower-left corner
    // in world space.
    // We combine the X and Z coordinates of the Chunk's corner into one 64-bit int
    // so that we can use them as a key for the map, as objects like std::pairs or
    // glm::ivec2s are not hashable by default, so they cannot be used as keys.
    std::unordered_map<int64_t, uPtr<Chunk>> m_chunks;

    // TODO: DELETE ALL REFERENCES TO m_geomCube AS YOU WILL NOT USE
    // IT IN YOUR FINAL PROGRAM!
    // The instance of a unit cube we can use to render any cube.
    // Presently, Terrain::draw renders one instance of this cube
    // for every non-EMPTY block within its Chunks. This is horribly
    // inefficient, and will cause your game to run very slowly until
    // milestone 1's Chunk VBO setup is completed.
//    Cube m_geomCube;


    std::unordered_set<Chunk*> m_chunksWithBlockData;
    std::vector<ChunkVBOData> m_chunksWithVBOData;
    QMutex m_cwbdLock;
    QMutex m_cwvdLock;

    // Keeps track of time in between checks to alter visible chunks
    float m_expansionTimer;
    // Keeps track of time in between lighting redstone
    float m_redstoneTimer;

    // Location at which expansion was last tried
    glm::vec3 m_lastExpansionPos;

public:
    OpenGLContext* mp_context;

    Terrain(OpenGLContext *context);
    ~Terrain();

    // We will designate every 64 x 64 area of the world's x-z plane
    // as one "terrain generation zone". Every time the player moves
    // near a portion of the world that has not yet been generated
    // (i.e. its lower-left coordinates are not in this set), a new
    // 4 x 4 collection of Chunks is created to represent that area
    // of the world.
    // The world that exists when the base code is run consists of exactly
    // one 64 x 64 area with its lower-left corner at (0, 0).
    // When milestone 1 has been implemented, the Player can move around the
    // world to add more "terrain generation zone" IDs to this set.
    // While only the 3 x 3 collection of terrain generation zones
    // surrounding the Player should be rendered, the Chunks
    // in the Terrain will never be deleted until the program is terminated.
    std::unordered_set<int64_t> m_generatedTerrain;

    std::vector<std::pair<glm::vec3, std::vector<glm::vec3>>> m_redstoneToUpdate;

    // Biome functions
    Biomes m_biomes;
    int chunksDone;

    int renderRadius; // set equal to 1 for 3x3, 2 for 5x5, etc.
  
    // Booleans describing in Water or Lava
    bool inWater;
    bool inLava;

    bool waterSound;
    // Instantiates a new Chunk and stores it in
    // our chunk map at the given coordinates.
    // Returns a pointer to the created Chunk.
    Chunk* instantiateChunkAt(int x, int z);
    // Do these world-space coordinates lie within
    // a Chunk that exists?
    bool hasChunkAt(int x, int z) const;
    // Assuming a Chunk exists at these coords,
    // return a mutable reference to it
    uPtr<Chunk>& getChunkAt(int x, int z);
    // Assuming a Chunk exists at these coords,
    // return a const reference to it
    const uPtr<Chunk>& getChunkAt(int x, int z) const;
    // Given a world-space coordinate (which may have negative
    // values) return the block stored at that point in space.
    BlockType getBlockAt(int x, int y, int z) const;
    BlockType getBlockAt(glm::vec3 p) const;
    RedstoneState getRedstoneState(int x, int y, int z) const;
    void setRedstoneState(int x, int y, int z, RedstoneState s);
    void updateRedstone(int x, int y, int z, std::vector<glm::vec3>* visited, std::unordered_set<Chunk*>* chunksToUpdate, bool updateVBOs);
    void switchSwitch(int x, int y, int z);
    bool isReachable(int x, int y, int z, std::vector<glm::vec3>* visited);
    void delayRedstoneLighting(float dT);

    // Updates visual appearance of redstone junctions
    void updateJunctions(int x, int y, int z, bool repeat);

    // Given a world-space coordinate (which may have negative
    // values) set the block at that point in space to the
    // given type.
    void setBlockAt(int x, int y, int z, BlockType t); // passes to VBOWorker
    void setBlockAt(glm::vec3 p, BlockType t);
    // set but with no VBO updating, used to speed up redstone
    void updateBlockAt(int x, int y, int z, BlockType t);
    void updateBlockAt(glm::vec3 p, BlockType t);
    void placeBlockAt(int x, int y, int z, BlockType t); // immediately updates VBOs
    void placeBlockAt(glm::vec3 p, BlockType t);

    // Draws every Chunk that falls within the bounding box
    // described by the min and max coords, using the provided
    // ShaderProgram
    void draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram);
    void terrainExpansion(glm::vec3 pos);
    void multithreadExpansion(glm::vec3 pos, float dT);
    void tryExpansion(glm::vec3 pos, glm::vec3 prevPos);
    void spawnFBMWorker(int64_t zone, std::vector<Chunk*>& toDo);
    void spawnFBMWorkers(std::unordered_set<int64_t>& zones);
    void spawnVBOWorker(Chunk* chunk);
    void checkThreads();
    
    void BiomeUnderground(int x, int z,  bool (&solidOrNot) [140]);
    void BiomeMountain(int x, int z, float height, bool (&solidOrNot) [140]);
    void BiomeGrass(int x, int z, float height, bool (&solidOrNot) [140]);
    void BiomeDesert(int x, int z, float height, bool (&solidOrNot) [140]);
    void BiomeForrest(int x, int z, float height, bool (&solidOrNot) [140]);
  
    BlockType getBiomeType(int x, int z);
    std::unordered_map<int64_t, std::tuple<int, BiomeType>> spawnTreeLocations(int x, int z);
    std::vector<std::tuple<glm::ivec3, BlockType>> generateTree(glm::ivec3 location, BiomeType b) const;
    std::vector<std::tuple<glm::ivec3,BlockType>> pineTree(glm::ivec3 location) const;
    std::vector<std::tuple<glm::ivec3,BlockType>> oakTree(glm::ivec3 location) const;
    std::vector<std::tuple<glm::ivec3,BlockType>> birchTree(glm::ivec3 location) const;
    std::vector<std::tuple<glm::ivec3,BlockType>> Cactus(glm::ivec3 location) const;
    void drawXLine(std::vector<std::tuple<glm::ivec3,BlockType>>& o, glm::ivec3 origin, int len, BlockType b) const;
    void drawZLine(std::vector<std::tuple<glm::ivec3,BlockType>>& o, glm::ivec3 origin, int len, BlockType b) const;
    std::vector<std::tuple<glm::ivec3,BlockType>> Pyramid(glm::ivec3 location) const;
    std::vector<std::tuple<glm::ivec3,BlockType>> Sphinx(glm::ivec3 location) const;
    void createTGZTerrain(int worldX, int worldZ);
    void createTGZ(int worldX, int worldZ);
    void createTGZWithChunks(int worldX, int worldZ);
    glm::ivec2 getTGZCoord(float x, float z);
    void populateNeighboringTGZ(float x, float z);


    // Initializes the Chunks that store the 64 x 256 x 64 block scene you
    // see when the base code is run.
    void CreateInitScene(float x, float z);
    void CreateTestScene();
};
