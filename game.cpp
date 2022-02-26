#include "game.h"
#include "utils.h"

// external linkage
extern LogStream warningLog; 
extern BoxMap* gBoxMap;


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

void BoxMap::onBoxMoveDone(BoxAnimator* boxAnimator) {
    boxes[boxAnimator->toMapY*width + boxAnimator->toMapX] = boxes[boxAnimator->fromMapY*width + boxAnimator->fromMapX];
    boxes[boxAnimator->fromMapY*width + boxAnimator->fromMapX] = 0;
}


    
bool BoxAnimator::tick() {
    if (steps <= 0) {
        warningLog << "trying to move a sprite that has already reached its destination. Maybe done-event missed ?\n";
        // TODO - trigger animation-done event
        finished = true;
    } else 
    if (steps == 1) {
        sprite->x = toX;
        sprite->y = toY;
        steps --;
        finished = true;
        gBoxMap->onBoxMoveDone(this);
        // TODO - trigger animation-done event
    } else {
        float stepX = (toX - sprite->x)/(float)steps;
        float stepY = (toY - sprite->y)/(float)steps;
        sprite->x += stepX;
        sprite->y += stepY;
    }
    return finished;
}




