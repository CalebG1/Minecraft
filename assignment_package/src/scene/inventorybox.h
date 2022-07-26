#ifndef INVENTORYBOX_H
#define INVENTORYBOX_H

#include <drawable.h>
#include "chunk.h"
class InventoryBox
{
public:

    BlockType item;
    int boxType; // 0: inventory, 1: toolbar, 2: crafting, 3 output
    int count;
    glm::vec2 coords;

    InventoryBox(glm::vec2 coords, int boxType);

    void decreaseBlock();

    bool checkCoordinateInBox(glm::vec2 coords);

};

#endif // INVENTORYBOX_H
