#include "inventorybox.h"
#include "chunk.h"

InventoryBox::InventoryBox(glm::vec2 coords, int boxType)
    :   item(EMPTY), boxType(boxType), count(0), coords(coords)
{}


void InventoryBox::decreaseBlock(){

    count--;
    if(count < 1){
        item = EMPTY;
    }
}

bool InventoryBox::checkCoordinateInBox(glm::vec2 c){
    return c.x > coords.x && c.x < coords.x + 50.f
            && c.y > coords.y - + 50.f && c.y < coords.y ;
}
