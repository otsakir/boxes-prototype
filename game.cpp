#include "game.h"
#include "utils.h"

// external linkage
extern LogStream warningLog; 
extern LogStream errorLog;
extern LogStream infoLog;
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

// returns a reference to the box item at position (tilex,tiley) 
Sprite*& BoxMap::at(int tilex, int tiley) {
    return boxes[width*tiley+tilex];
}

/*
void BoxMap::onBoxMoveDone(BoxAnimator* boxAnimator) {
    boxes[boxAnimator->toMapY*width + boxAnimator->toMapX] = boxes[boxAnimator->fromMapY*width + boxAnimator->fromMapX];
    boxes[boxAnimator->fromMapY*width + boxAnimator->fromMapX] = 0;
}
* */


Point2 Level::posAt(int tilex, int tiley) {
    Point2 atpos;
    
    atpos.x = this->pos.x + tilex * BOX_TILE_WIDTH;
    atpos.y = this->pos.y + tiley * BOX_TILE_HEIGHT;
    
    return atpos;
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




