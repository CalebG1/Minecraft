#ifndef INVENTORY_H
#define INVENTORY_H

#include <drawable.h>
#include "chunk.h"
#include "smartpointerhelp.h"
#include "inventorybox.h"
#include <array>
//BLK_UVX
//BLK_UVY
//BLK_UV
const static std::array<glm::vec2, 10> numberUVs {
    glm::vec2(8.f BLK_UVX, 4.f BLK_UVY),
    glm::vec2(9.f BLK_UVX, 4.f BLK_UVY),
    glm::vec2(10.f BLK_UVX, 4.f BLK_UVY),
    glm::vec2(11.f BLK_UVX, 4.f BLK_UVY),
    glm::vec2(12.f BLK_UVX, 4.f BLK_UVY),
    glm::vec2(13.f BLK_UVX, 4.f BLK_UVY),
    glm::vec2(14.f BLK_UVX, 4.f BLK_UVY),
    glm::vec2(15.f BLK_UVX, 4.f BLK_UVY),
    glm::vec2(8.f BLK_UVX, 3.f BLK_UVY),
    glm::vec2(9.f BLK_UVX, 3.f BLK_UVY),

};



const static std::unordered_map<BlockType, std::array<BlockType, 9>> orderedCraftingRecipes {
    {
        LAVA, std::array<BlockType, 9> {EMPTY, SNOW, EMPTY,
                                        EMPTY, DESERT, EMPTY,
                                        DESERT, DESERT, DESERT}
    },
    {
        SANDSTONE, std::array<BlockType, 9> {EMPTY, STONE, EMPTY,
                                          EMPTY, DESERT, EMPTY,
                                          EMPTY, EMPTY, EMPTY}
    },
    {
        CACTUS_DARKGREEN, std::array<BlockType, 9> {LEAF_PINE, LEAF_PINE, LEAF_PINE,
                                                   LEAF_PINE, WATER, LEAF_PINE,
                                                   LEAF_PINE, LEAF_PINE, LEAF_PINE}
    },
    {
        STONE,
        std::array<BlockType, 9> {
            WATER, LAVA, WATER,
            LAVA, WATER, LAVA,
            WATER, LAVA, WATER
        }
    },
    {
        WOOD_BIRCH,
        std::array<BlockType, 9> {
            EMPTY, SNOW, EMPTY,
            EMPTY, WOOD_PINE, EMPTY,
            EMPTY, WOOD_PINE, EMPTY
        }
    }
};

class Inventory : public Drawable
{
public:




    std::vector<uPtr<InventoryBox>> items;

    int craftingIdx;
    int outputIdx;

    int windowWidth;
    int windowHeight;
    bool invOpen;

    glm::vec2 currPos;



    InventoryBox* selectedItem; // item current clicked on/held by player
    int currentToolbar; // item currently selected on toolbar

    glm::vec2 screenSpaceToPixelSpace(glm::vec2 pixelCoords);
    glm::vec2 pixelSpaceToScreenSpace(glm::vec2 screenCoords);

    Inventory(OpenGLContext* context, int w, int h);
    void createVBOdata() override;

    void populateInventory();
    void addToInventory(BlockType block, int count);
    void mouseMove(int x, int y);
    void click(int x, int y, bool isLeftClick);
    void checkCrafting();
    void confirmCraft();
};

#endif // INVENTORY_H
