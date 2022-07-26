#include "inventory.h"
#include "smartpointerhelp.h"
#include "chunk.h"
#include <iostream>
#include <algorithm>

Inventory::Inventory(OpenGLContext* context, int w, int h) : Drawable(context), items(),
    windowWidth(w),
    windowHeight(h),
    invOpen(false),
    currPos(0.f, 0.f),
    selectedItem(nullptr),
    currentToolbar(0)
{

    for(int i = 0; i < 9 ; i++){
        items.push_back(mkU<InventoryBox>((glm::vec2((windowWidth /2.f - 225.f) + i*50.f , windowHeight - 50.f)), 1));
    }

    for(int i = 0; i < 3 ; i++){
        for(int j = 0; j < 9; j++){
            items.push_back(mkU<InventoryBox>((glm::vec2((windowWidth /2.f - 225.f) + j*50.f, (windowHeight / 2.f) + i * -50.f)), 0));

        }
    }

    craftingIdx = items.size();

    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            items.push_back(mkU<InventoryBox>((glm::vec2((windowWidth/ 2.f - 75.f) + j * 50.f, (windowHeight/2.f) + 100.f + i * 50.f)), 2));
        }
    }

    outputIdx = items.size();
    items.push_back(mkU<InventoryBox>((glm::vec2((windowWidth/ 2.f - 75.f) + 200.f, (windowHeight/2.f) + 150.f)), 3));



}

void Inventory::checkCrafting(){
    for(auto const& [block, recipe] : orderedCraftingRecipes){
        bool accepted = true;
        int min = 100000;
        for(int i = 0; i < outputIdx - craftingIdx; i++){
              int uiIdx = i + craftingIdx;

              if(items[uiIdx]->item != recipe[i] || items[uiIdx].get() == selectedItem){
                  accepted = false;
                  break;
              } else if(items[uiIdx]->item != EMPTY) {
                  min = std::min(min, items[uiIdx]->count);
              }
        }
        if(accepted){
            items[outputIdx]->item = block;
            items[outputIdx]->count = min;
            return;
        }
    }

    items[outputIdx]->item = EMPTY;
    items[outputIdx]->count = 0;

}

void Inventory::confirmCraft(){
    for(int i = craftingIdx; i < outputIdx; i++){
        items[i]->count -= items[outputIdx]->count;

        if(items[i]->count <= 0){
            items[i]->item = EMPTY;
            items[i]->count = 0;
        }
    }

    selectedItem = items[outputIdx].get();

}

void Inventory::addToInventory(BlockType block, int count){
    InventoryBox* emptyBox = nullptr;
    for(auto const& item : items){

        if(item->item == EMPTY && emptyBox == nullptr){
            emptyBox = item.get();
        }

        if(item->item == block){
            item->count += count;
            return;
        }

    }

    if(emptyBox != nullptr){
        emptyBox->item = block;
        emptyBox->count = count;
    }



}


void Inventory::populateInventory(){
    int idx = 0;
    for ( int blockInt = GRASS; blockInt != BEDROCK; blockInt++ ) {

        BlockType block = static_cast<BlockType>(blockInt);
        items[idx]->item = block;
        items[idx]->count = 69;

        idx++;
    }
  }

void Inventory::mouseMove(int x, int y){

    currPos.x = x;
    currPos.y = y;
}

void Inventory::click(int x, int y, bool isLeftClick){

    bool hasClicked = false;
    for(auto const& item : items){
        if(item->checkCoordinateInBox(glm::vec2(x, y))) {
                hasClicked = true;


                if(selectedItem != nullptr && selectedItem == item.get() && selectedItem->boxType == 3){
                    return;
                }

                if(isLeftClick){
                    if(selectedItem == nullptr  && item->item != EMPTY){
                       // if no item is selected and you click on a box w/ an item, pick up that item
                       selectedItem = item.get();
                   } else if(selectedItem == item.get()){
                       // if you click on the box of the selected item, unselect it
                       selectedItem = nullptr;
                   } else if(selectedItem != nullptr && item->item != EMPTY
                             && item->item == selectedItem->item){
                        // combine stacks
                        item->count += selectedItem->count;
                        selectedItem->count = 0;
                        selectedItem->item = EMPTY;
                        selectedItem = nullptr;
                    } else if(selectedItem != nullptr && item->item != EMPTY){
                       // if  an item is selected and you another item is clicked, swap
                       BlockType tempBlock = selectedItem->item;
                       int tempCount = selectedItem->count;
                       selectedItem->item = item->item;
                       selectedItem->count = item->count;
                       item->item = tempBlock;
                       item->count = tempCount;
                   } else if(selectedItem != nullptr){
                       // if an item is selected and an empty box is clicked, store item there
                       item->item =  selectedItem->item;
                       item->count =  selectedItem->count;
                       selectedItem->item = EMPTY;
                       selectedItem->count = 0;
                       selectedItem = nullptr;
                   }
                } else {
                    if(selectedItem == item.get()){
                        selectedItem = nullptr;

                    }
                    else if(selectedItem != nullptr
                            && (item->item == EMPTY || item->item == selectedItem->item)){
                                        item->item = selectedItem->item;
                                        item->count++;
                                        selectedItem->decreaseBlock();

                                    }



                }

                if(item->boxType == 2){
                    checkCrafting();
                } else if(item->boxType == 3 && isLeftClick){
                    confirmCraft();
                }


        }
    }
    if(!hasClicked && (selectedItem == nullptr || (selectedItem != nullptr && selectedItem->boxType != 3))){
        selectedItem = nullptr;
    }


}

glm::vec2 Inventory::screenSpaceToPixelSpace(glm::vec2 screenCoords){
    return glm::vec2((screenCoords.x * 1.f / windowWidth) * 2 -1 ,
                     1- (screenCoords.y * 1.f / windowHeight) * 2 );
}

glm::vec2 Inventory::pixelSpaceToScreenSpace(glm::vec2 pixelCoords){
    return glm::vec2(
                ((pixelCoords.x + 1) / 2.f) * windowWidth,
                ((1 - pixelCoords.y)/2.f) * windowHeight
                );
}


void Inventory::createVBOdata(){

    // 4 vertices of outer box, 4 uv coordinates of inner box,
    // 4 vertices of block texture, 4 uv coordinates of block texture
    std::vector<glm::vec2> vec;
    std::vector<GLuint> idx;

    GLuint vertCount = 0;
    const glm::vec2 wH = glm::vec2(windowWidth, windowHeight);

    for(auto const& item : items){
        if(item->boxType == 1 || invOpen){
            glm::vec2 uv = glm::vec2(10.f BLK_UVX, 3.f BLK_UVY);
            if(items[currentToolbar].get() == item.get()){
                uv += glm::vec2(1.f BLK_UVX, 0.f);
            }
            std::vector<GLuint> faceIdxs = std::vector<GLuint>();

            vec.push_back(screenSpaceToPixelSpace(item->coords));
            vec.push_back(uv);
            faceIdxs.push_back(vertCount);
            vertCount++;

            vec.push_back((screenSpaceToPixelSpace(item->coords) + (glm::vec2(100.f, 0.f)/wH)));
            vec.push_back(uv + glm::vec2(BLK_UV, 0.f));
            faceIdxs.push_back(vertCount);
            vertCount++;

            vec.push_back((screenSpaceToPixelSpace(item->coords) + (glm::vec2(100.f, 100.f)/wH)));
            vec.push_back(uv  + glm::vec2(BLK_UV, BLK_UV));
            faceIdxs.push_back(vertCount);
            vertCount++;

            vec.push_back((screenSpaceToPixelSpace(item->coords) + (glm::vec2(0.f, 100.f)/wH)));
            vec.push_back(uv  + glm::vec2(0.f, BLK_UV));
            faceIdxs.push_back(vertCount);
            vertCount++;

            idx.push_back(faceIdxs[0]);
            idx.push_back(faceIdxs[1]);
            idx.push_back(faceIdxs[2]);
            idx.push_back(faceIdxs[0]);
            idx.push_back(faceIdxs[2]);
            idx.push_back(faceIdxs[3]);

            if(item->item != EMPTY && item.get() != selectedItem){

                // block texture
                glm::vec2 innerCoords = item->coords + glm::vec2(5.f, -5.f);
                glm::vec2 innerUV = glm::vec2();

                if(item->item == REDSTONE){
                    innerUV = redstoneUVs.at(UNLIT_WIRE_SPOT).at(YNEG);
                } else {
                    innerUV = blockFaceUVs.at(item->item).at(XPOS);
                }



                vec.push_back(screenSpaceToPixelSpace(innerCoords));
                vec.push_back(innerUV);
                faceIdxs.push_back(vertCount);
                vertCount++;

                vec.push_back((screenSpaceToPixelSpace(innerCoords )+ (glm::vec2(80.f, 0.f)/wH)));
                vec.push_back(innerUV + glm::vec2(BLK_UV, 0.f));
                faceIdxs.push_back(vertCount);
                vertCount++;

                vec.push_back((screenSpaceToPixelSpace(innerCoords )+ (glm::vec2(80.f, 80.f)/wH)));
                vec.push_back(innerUV  + glm::vec2(BLK_UV, BLK_UV));
                faceIdxs.push_back(vertCount);
                vertCount++;

                vec.push_back((screenSpaceToPixelSpace(innerCoords )+ (glm::vec2(0.f, 80.f)/wH)));
                vec.push_back(innerUV  + glm::vec2(0.f, BLK_UV));
                faceIdxs.push_back(vertCount);
                vertCount++;

                idx.push_back(faceIdxs[4]);
                idx.push_back(faceIdxs[5]);
                idx.push_back(faceIdxs[6]);
                idx.push_back(faceIdxs[4]);
                idx.push_back(faceIdxs[6]);
                idx.push_back(faceIdxs[7]);

                // number texture
                const std::array<int, 2> digits
                {(item->count / 10) % 10,
                  item->count % 10};

                for(int i = 0; i < 2; i++){


                    glm::vec2 textCoords = item->coords + glm::vec2(5.f + (i * 10.f), -5.f);
                    glm::vec2 textUV = numberUVs[digits[i]];
                    vec.push_back(screenSpaceToPixelSpace(textCoords));
                    vec.push_back(textUV);
                    faceIdxs.push_back(vertCount);
                    vertCount++;

                    vec.push_back((screenSpaceToPixelSpace(textCoords )+ (glm::vec2(30.f, 0.f)/wH)));
                    vec.push_back(textUV + glm::vec2(BLK_UV, 0.f));
                    faceIdxs.push_back(vertCount);
                    vertCount++;

                    vec.push_back((screenSpaceToPixelSpace(textCoords )+ (glm::vec2(30.f, 30.f)/wH)));
                    vec.push_back(textUV  + glm::vec2(BLK_UV, BLK_UV));
                    faceIdxs.push_back(vertCount);
                    vertCount++;

                    vec.push_back((screenSpaceToPixelSpace(textCoords )+ (glm::vec2(0.f, 30.f)/wH)));
                    vec.push_back(textUV  + glm::vec2(0.f, BLK_UV));
                    faceIdxs.push_back(vertCount);
                    vertCount++;

                }

                idx.push_back(faceIdxs[8]);
                idx.push_back(faceIdxs[9]);
                idx.push_back(faceIdxs[10]);
                idx.push_back(faceIdxs[8]);
                idx.push_back(faceIdxs[10]);
                idx.push_back(faceIdxs[11]);

                idx.push_back(faceIdxs[12]);
                idx.push_back(faceIdxs[13]);
                idx.push_back(faceIdxs[14]);
                idx.push_back(faceIdxs[12]);
                idx.push_back(faceIdxs[14]);
                idx.push_back(faceIdxs[15]);


            }
        }

    }

    if(invOpen && selectedItem != nullptr && selectedItem->item != EMPTY){
        std::vector<GLuint> faceIdxs = std::vector<GLuint>();
        // selected block
        glm::vec2 selectedCoords = screenSpaceToPixelSpace(currPos + glm::vec2(-20.f, 20.f));
//        selectedCoords = glm::vec2(-1.f,-1.f);
//        selectedCoords *= glm::vec2(1.f, -1.f);

//        selectedCoords += glm::vec2(-1.f, 1.f);
//        glm::vec2 selectedUV = blockFaceUVs.at(selectedItem->item).at(XPOS);

        glm::vec2 selectedUV = glm::vec2();

        if(selectedItem->item == REDSTONE){
            selectedUV = redstoneUVs.at(UNLIT_WIRE_SPOT).at(YNEG);
        } else {
            selectedUV = blockFaceUVs.at(selectedItem->item).at(XPOS);
        }
        vec.push_back(selectedCoords);
        vec.push_back(selectedUV);
        faceIdxs.push_back(vertCount);
        vertCount++;

        vec.push_back((selectedCoords + (glm::vec2(75.f, 0.f))/wH));
        vec.push_back(selectedUV + glm::vec2(BLK_UV, 0.f));
        faceIdxs.push_back(vertCount);
        vertCount++;

        vec.push_back((selectedCoords + (glm::vec2(75.f, 75.f)/wH)));
        vec.push_back(selectedUV  + glm::vec2(BLK_UV, BLK_UV));
        faceIdxs.push_back(vertCount);
        vertCount++;

        vec.push_back((selectedCoords + (glm::vec2(0.f, 75.f)/wH)));
        vec.push_back(selectedUV  + glm::vec2(0.f, BLK_UV));
        faceIdxs.push_back(vertCount);
        vertCount++;

        idx.push_back(faceIdxs[0]);
        idx.push_back(faceIdxs[1]);
        idx.push_back(faceIdxs[2]);
        idx.push_back(faceIdxs[0]);
        idx.push_back(faceIdxs[2]);
        idx.push_back(faceIdxs[3]);



    }

    m_opaqueCount = idx.size();
    generateIdxOpaque();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpaque);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generateOpaque();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufOpaque);
    mp_context->glBufferData(GL_ARRAY_BUFFER, vec.size() * sizeof(glm::vec2), vec.data(), GL_STATIC_DRAW);

}
