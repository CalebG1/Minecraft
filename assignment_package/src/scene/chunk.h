#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include "drawable.h"
#include "glm/glm.hpp"
#include <unordered_set>
#include <QMutex>
//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{ // put stuff after empty, before bedrock
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW, DESERT, DESERT2, FORREST, COOLSTONE, LAVA,
    WOOD_BIRCH, WOOD_OAK, WOOD_PINE, CACTUS_LIGHTGREEN, CACTUS_DARKGREEN, CACTUS_BROWN,
    LEAF_BIRCH, LEAF_PINE, LEAF_OAK,
    SANDSTONE, REDSTONE, UNLIT_REDSTONE_LAMP,  LIT_REDSTONE_TORCH, SWITCH_OFF, BEDROCK, LIT_REDSTONE_LAMP, UNLIT_REDSTONE_TORCH, SWITCH_ON
};

enum RedstoneState : unsigned char
{
    UNLIT_WIRE_SPOT, LIT_WIRE_SPOT, UNLIT_WIRE_STRAIGHT_HOR, LIT_WIRE_STRAIGHT_HOR, UNLIT_WIRE_STRAIGHT_VER, LIT_WIRE_STRAIGHT_VER, UNLIT_WIRE_JUNCTION, LIT_WIRE_JUNCTION
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG, NONE
};

const static std::unordered_set<RedstoneState> unlitStates {
    {UNLIT_WIRE_SPOT, UNLIT_WIRE_STRAIGHT_HOR, UNLIT_WIRE_STRAIGHT_VER, UNLIT_WIRE_JUNCTION}
};

const static std::unordered_set<RedstoneState> litStates {
    {LIT_WIRE_SPOT, LIT_WIRE_STRAIGHT_HOR, LIT_WIRE_STRAIGHT_VER, LIT_WIRE_JUNCTION}
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};


struct VertexData{
    glm::vec4 pos;

    glm::vec2 uv;

    VertexData(glm::vec4 p, glm::vec2 u)
        : pos(p), uv(u)
    {}
};

struct BlockFace {
    Direction direction;
    glm::vec3 directionVec;
    std::array<VertexData, 4> vertices;
    BlockFace(Direction dir, glm::vec3 dirV, const VertexData &a, const VertexData &b, const VertexData &c , const VertexData &d)
        : direction(dir), directionVec(dirV), vertices{a, b, c, d}
    {}
};


struct ChunkVBOData;

#define BLK_UVX * 0.0625
#define BLK_UVY * 0.0625
#define BLK_UV 0.0625

const static std::unordered_set<BlockType> transparentBlocks {
    {UNLIT_REDSTONE_TORCH, LIT_REDSTONE_TORCH, REDSTONE}
};


const static std::array<BlockFace, 6> adjacentFaces {
    BlockFace(XPOS, glm::vec3(1, 0, 0), VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(0, 0)),
                                        VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(BLK_UV, 0)),
                                        VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(BLK_UV, BLK_UV)),
                                        VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(0, BLK_UV))
              ),
    BlockFace(XNEG, glm::vec3(-1, 0, 0), VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(0, 0)),
                                        VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(BLK_UV, 0)),
                                        VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(BLK_UV, BLK_UV)),
                                        VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(0, BLK_UV))
              ),
    BlockFace(YPOS, glm::vec3(0, 1, 0), VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(0, 0)),
                                        VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(BLK_UV, 0)),
                                        VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(BLK_UV, BLK_UV)),
                                        VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(0, BLK_UV))
              ),
    BlockFace(YNEG, glm::vec3(0, -1, 0), VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(0, 0)),
                                        VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(BLK_UV, 0)),
                                        VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(BLK_UV, BLK_UV)),
                                        VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(0, BLK_UV))
              ),
    BlockFace(ZPOS, glm::vec3(0, 0, 1), VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(0, 0)),
                                        VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(BLK_UV, 0)),
                                        VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(BLK_UV, BLK_UV)),
                                        VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(0, BLK_UV))
              ),
    BlockFace(ZNEG, glm::vec3(0, 0, -1), VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(0, 0)),
                                        VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(BLK_UV, 0)),
                                        VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(BLK_UV, BLK_UV)),
                                        VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(0, BLK_UV))
              )
};

//EMPTY, GRASS, DIRT, STONE, WATER, SNOW, DESERT, FORREST, REDSTONE
// texture bitmap is 256x256 pixels with 16x16 grid
// UVX is horizontal position, UVY is vertical position
// numbers below corrispond to texture cell - 1. So first cell is numbered (0,0)
const static std::unordered_map<BlockType, std::unordered_map<Direction, glm::vec2, EnumHash>, EnumHash> blockFaceUVs{
    {GRASS, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)}}},
    {DIRT, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)}}},
    {STONE, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)}}},


    {WATER, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(14.f BLK_UVX, 3.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(14.f BLK_UVX, 3.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(14.f BLK_UVX, 3.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(14.f BLK_UVX, 3.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(14.f BLK_UVX, 3.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(14.f BLK_UVX, 3.f BLK_UVY)}}},
    {SNOW, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)}}},
    {DESERT, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)}}},
    {DESERT2, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(0.f BLK_UVX, 4.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(0.f BLK_UVX, 4.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(0.f BLK_UVX, 4.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(0.f BLK_UVX, 4.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(0.f BLK_UVX, 4.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(0.f BLK_UVX, 4.f BLK_UVY)}}},
    {FORREST, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)}}},
    {COOLSTONE, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(1.f BLK_UVX, 7.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(1.f BLK_UVX, 7.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(1.f BLK_UVX, 7.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(1.f BLK_UVX, 7.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(1.f BLK_UVX, 7.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(1.f BLK_UVX, 7.f BLK_UVY)}}},
    {SANDSTONE, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(0.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(0.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(0.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(0.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(0.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(0.f BLK_UVX, 2.f BLK_UVY)}}},
    {LAVA, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(14.f BLK_UVX, 1.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(14.f BLK_UVX, 1.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(14.f BLK_UVX, 1.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(14.f BLK_UVX, 1.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(14.f BLK_UVX, 1.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(14.f BLK_UVX, 1.f BLK_UVY)}}},
    {UNLIT_REDSTONE_LAMP, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(3.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(3.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(3.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(3.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(3.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(3.f BLK_UVX, 2.f BLK_UVY)}}},
    {LIT_REDSTONE_LAMP, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(4.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(4.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(4.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(4.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(4.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(4.f BLK_UVX, 2.f BLK_UVY)}}},
    {UNLIT_REDSTONE_TORCH, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(3.f BLK_UVX, 8.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(3.f BLK_UVX, 8.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(7.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(3.f BLK_UVX, 8.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(3.f BLK_UVX, 8.f BLK_UVY)}}},
    {LIT_REDSTONE_TORCH, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(3.f BLK_UVX, 9.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(3.f BLK_UVX, 9.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(8.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(3.f BLK_UVX, 9.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(3.f BLK_UVX, 9.f BLK_UVY)}}},
    {SWITCH_OFF, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(7.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(7.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(7.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(7.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(7.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(7.f BLK_UVX, 14.f BLK_UVY)}}},
    {SWITCH_ON, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(8.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(8.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(8.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(8.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(8.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(8.f BLK_UVX, 14.f BLK_UVY)}}},
    {BEDROCK, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)}}},
    {WOOD_BIRCH, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(5.f BLK_UVX, 8.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(5.f BLK_UVX, 8.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(5.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(5.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(5.f BLK_UVX, 8.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(5.f BLK_UVX, 8.f BLK_UVY)}}},
    {WOOD_OAK, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(4.f BLK_UVX, 8.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(4.f BLK_UVX, 8.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(5.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(5.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(4.f BLK_UVX, 8.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(4.f BLK_UVX, 8.f BLK_UVY)}}},

    {WOOD_PINE, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(4.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(4.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(5.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(5.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(4.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(4.f BLK_UVX, 14.f BLK_UVY)}}},

    {CACTUS_LIGHTGREEN, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)}}},
    {CACTUS_DARKGREEN, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(1.f BLK_UVX, 6.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(1.f BLK_UVX, 6.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(1.f BLK_UVX, 6.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(1.f BLK_UVX, 6.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(1.f BLK_UVX, 6.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(1.f BLK_UVX, 6.f BLK_UVY)}}},
    {CACTUS_BROWN, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(8.f BLK_UVX, 9.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(8.f BLK_UVX, 9.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(8.f BLK_UVX, 9.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(8.f BLK_UVX, 9.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(8.f BLK_UVX, 9.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(8.f BLK_UVX, 9.f BLK_UVY)}}},

    {LEAF_BIRCH, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(4.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(4.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(4.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(4.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(4.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(4.f BLK_UVX, 12.f BLK_UVY)}}},
    {LEAF_PINE, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)}}},
    {LEAF_OAK, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                                  {XNEG, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)}}},

};

const static std::unordered_map<RedstoneState, std::unordered_map<Direction, glm::vec2, EnumHash>, EnumHash> redstoneUVs{
    {UNLIT_WIRE_SPOT, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                         {XNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                         {YPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                         {YNEG, glm::vec2(5.f BLK_UVX, 4.f BLK_UVY)},
                                                                         {ZPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                         {ZNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)}}},
    {LIT_WIRE_SPOT, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                       {XNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                       {YPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                       {YNEG, glm::vec2(5.f BLK_UVX, 4.f BLK_UVY)},
                                                                       {ZPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                       {ZNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)}}},
    {UNLIT_WIRE_STRAIGHT_HOR, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                                 {XNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                                 {YPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                                 {YNEG, glm::vec2(5.f BLK_UVX, 5.f BLK_UVY)},
                                                                                 {ZPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                                 {ZNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)}}},
    {LIT_WIRE_STRAIGHT_HOR, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                               {XNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                               {YPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                               {YNEG, glm::vec2(5.f BLK_UVX, 5.f BLK_UVY)},
                                                                               {ZPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                               {ZNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)}}},
    {UNLIT_WIRE_STRAIGHT_VER, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                                 {XNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                                 {YPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                                 {YNEG, glm::vec2(4.f BLK_UVX, 4.f BLK_UVY)},
                                                                                 {ZPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                                 {ZNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)}}},
    {LIT_WIRE_STRAIGHT_VER, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                               {XNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                               {YPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                               {YNEG, glm::vec2(4.f BLK_UVX, 4.f BLK_UVY)},
                                                                               {ZPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                               {ZNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)}}},
    {UNLIT_WIRE_JUNCTION, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                             {XNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                             {YPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                             {YNEG, glm::vec2(4.f BLK_UVX, 5.f BLK_UVY)},
                                                                             {ZPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                             {ZNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)}}},
    {LIT_WIRE_JUNCTION, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                           {XNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                           {YPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                           {YNEG, glm::vec2(4.f BLK_UVX, 5.f BLK_UVY)},
                                                                           {ZPOS, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)},
                                                                           {ZNEG, glm::vec2(9.f BLK_UVX, 2.f BLK_UVY)}}},
};
// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable
class Chunk : public Drawable {
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;

    // All redstone in the chunk, indexed by m_blocks index
    std::unordered_map<int, RedstoneState> m_redstone;
public:
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    int x, z;
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;
    bool shouldUpdateVBO;
    Chunk(OpenGLContext* context, int x, int z);
    void createVBOdata() override;
    ChunkVBOData fillVBOdata();
    void setupVBO(std::vector<glm::vec4> &vec, std::vector<GLuint> &idx, std::vector<glm::vec4> &vec2, std::vector<GLuint> &idx2);
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    RedstoneState getRedstoneState(unsigned int x, unsigned int y, unsigned int z) const;
    RedstoneState getRedstoneState(int x, int y, int z) const;
    void setRedstoneState(unsigned int x, unsigned int y, unsigned int z, RedstoneState s);
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void updateBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void placeBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    void setBlockAtNeighbor(int x, int y, int z, BlockType t);
};

struct ChunkVBOData {
    std::vector<glm::vec4> m_vbo;
    std::vector<GLuint> m_idx;
    std::vector<glm::vec4> m_tvbo;
    std::vector<GLuint> m_tidx;
    Chunk* mp_chunk;

    ChunkVBOData(Chunk* c, std::vector<glm::vec4>& vbo, std::vector<GLuint>& idx, std::vector<glm::vec4>& tvbo, std::vector<GLuint> tidx)
        : m_vbo(vbo), m_idx(idx), m_tvbo(tvbo), m_tidx(tidx), mp_chunk(c)
    {}
};
