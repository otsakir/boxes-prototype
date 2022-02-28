#include "game.h"
#include "utils.h"

// external linkage
extern LogStream warningLog; 
extern LogStream errorLog;
extern LogStream infoLog;
extern BoxMap* gBoxMap;

Sprite* BoxMap::OUT_OF_LIMITS;

void BoxMap::renderBoxes(SDL_Renderer* renderer) {
    for (int i=0; i < width; i++) {
        for (int j=0; j<height; j++) {
            Sprite* sprite = boxes[ width*j + i ];
            if (sprite) {
                sprite->render(renderer);
            }
        }
    }
}

void BoxMap::putBox(int posX, int posY, Sprite* boxSprite) {
    // TODO - what if boxSprite == 0
    if ( boxes[posY*width + posX] ) {
        warningLog << "BoxMap: there is already a box position (" << posX << "," << posY << ")\n";
    } else {
        boxes[posY*width + posX] = boxSprite;
    }
}

/*
BoxMap::PosStatus BoxMap::boxAt(int posX, int posY, Sprite*& sprite) {
    if (posX >= 0 && posX < width) 
        if (posY >=0 && posY < height) {
            sprite = boxes[posY*width + posX];
            if (sprite) 
                return PosFull;
            else
                return PosEmpty;
        }
    warningLog << "invalid map position (" << posX << "," << posY << ")\n";
    return PosInvalid;
}
*/

// returns a reference to the box item at position (tilex,tiley) 
Sprite*& BoxMap::at(int tilex, int tiley) {
    if (tilex < 0 || tilex >= width || tiley < 0 || tiley >=height)
        return BoxMap::OUT_OF_LIMITS;
    
    return boxes[width*tiley+tilex];
}


BoxSprite* BoxFactory::create(BoxId boxId) {
    if (boxId == BoxId::RED_BOX) {
        SDL_Texture* texture = resources->getImage(ImageId::RED_BLOCK);
        SDL_Rect sourceRect;
        sourceRect.x = 0;
        sourceRect.y = 0;
        sourceRect.w = 64;
        sourceRect.h = 64;
        RenderableBitmap* renderable = new RenderableBitmap(texture, sourceRect); // texture mem handled by Resources
        BoxSprite* boxSprite = new BoxSprite(renderable);
        return boxSprite;
    } else {
        warningLog << "unknown boxId: " << boxId << "\n";
        return 0;
    }
}


Point2 Level::posAt(int tilex, int tiley) {
    Point2 atpos;
    
    atpos.x = this->pos.x + tilex * BOX_TILE_WIDTH;
    atpos.y = this->pos.y + tiley * BOX_TILE_HEIGHT;
    
    return atpos;
}

BoxSprite* Level::newBoxAt(int mapX, int mapY, BoxId boxId) {
    //if (boxMap->
    BoxSprite* boxSprite = boxFactory->create(boxId);
    if (boxSprite) {
        boxSprite->setPos( posAt(mapX, mapY) );
        boxMap->putBox(mapX, mapY, boxSprite);
    }
    
    return boxSprite;
} 

    
bool Animator::tick() {
    if (steps <= 0) {
        warningLog << "trying to move a sprite that has already reached its destination. Maybe done-event missed ?\n";
        // TODO - trigger animation-done event
        finished = true;
    } else 
    if (steps == 1) {
        sprite->pos = toPos;
        steps --;
        finished = true;
        infoLog << "animation finished\n";
    } else {
        float stepX = (toPos.x - sprite->pos.x)/(float)steps;
        float stepY = (toPos.y - sprite->pos.y)/(float)steps;
        sprite->pos.x += stepX;
        sprite->pos.y += stepY;
        steps --;
    }
    return finished;
}

// go through animation slots and tick each one of them
void Animations::tick() {
    for (int i=0; i < count; i++) {
        if ( ! animators[i].finished ) {
            animators[i].tick();
        }
    }
}

// return an available animator and mark it as non-finished
Animator* Animations::getAnimatorSlot() {
    for (int i=0; i < count; i++) {
        if (animators[i].finished) {
            //animators[i].finished = false;
            return animators + i;
        }
    }
    errorLog << "no animator slots available" << "\n";
    return 0;
}




