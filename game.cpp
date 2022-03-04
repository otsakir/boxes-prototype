#include "game.h"
#include "utils.h"

// external linkage
extern LogStream warningLog; 
extern LogStream errorLog;
extern LogStream infoLog;

template class ListPool<Animator,int>; // instansiate class out of class template

BoxSprite* BoxMap::OUT_OF_LIMITS = 0; // definition for static field of BoxMap class. Needed when linking.

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

void BoxMap::putBox(int posX, int posY, BoxSprite* boxSprite) {
    // TODO - what if boxSprite == 0
    if ( boxes[posY*width + posX] ) {
        warningLog << "BoxMap: there is already a box position (" << posX << "," << posY << ")\n";
    } else {
        boxes[posY*width + posX] = boxSprite;
    }
}

// returns a reference to the box item at position (tilex,tiley) 
BoxSprite*& BoxMap::at(int tilex, int tiley) {
    if (tilex < 0 || tilex >= width || tiley < 0 || tiley >=height)
        return BoxMap::OUT_OF_LIMITS;
    
    return boxes[width*tiley+tilex];
}

 
BoxSprite* BoxFactory::create(BoxId boxId) {
    if (boxId == BoxId::RANDOM_BOX) {
        boxId = (BoxId) randomInRange(RED_BOX, GREEN_BOX);
        //boxId = (BoxId) randomInRange(RED_BOX, ORANGE_BOX); // make it easier, use fewer colors
    }
        
    Texture* texture;
    switch (boxId) {
        case BoxId::RED_BOX:
            texture = resources->getImage(ImageId::RED_BLOCK);
        break;
        case BoxId::BLUE_BOX:
            texture = resources->getImage(ImageId::BLUE_BLOCK);
        break;
        case BoxId::ORANGE_BOX:
            texture = resources->getImage(ImageId::ORANGE_BLOCK);
        break;
        case BoxId::GREY_BOX:
            texture = resources->getImage(ImageId::GREY_BLOCK);
        break;
        case BoxId::BROWN_BOX:
            texture = resources->getImage(ImageId::BROWN_BLOCK);
        break;
        case BoxId::GREEN_BOX:
            texture = resources->getImage(ImageId::GREEN_BLOCK);
        break;
        default:
            errorLog << "Can't create box. Invalid boxId: " << boxId << "\n";
            return 0;
        break;
    }
        
    RenderableBitmap* renderable = new RenderableBitmap(texture, 64, 64); // texture mem handled by Resources
    BoxSprite* boxSprite = new BoxSprite(renderable, boxId);
    return boxSprite;
}

// screen position from tile coordinates
Point2 Game::posAt(int tilex, int tiley) {
    Point2 atpos;
    
    atpos.x = this->pos.x + tilex * BOX_TILE_WIDTH;
    atpos.y = this->pos.y + tiley * BOX_TILE_HEIGHT;
    
    return atpos;
}

// return false if out of boxMap limits
bool Game::tileXYAt(int screenx, int screeny, int& tilex, int& tiley) {
    // make relative to BoxMap
    screenx -= this->pos.x;
    screeny -= this->pos.y;
    
    if (screenx < 0 || screenx >= boxMap->width * BOX_TILE_WIDTH)
        return false;

    if (screeny < 0 || screeny >= boxMap->height * BOX_TILE_HEIGHT)
        return false;
        
    tilex = screenx / BOX_TILE_WIDTH;
    tiley = screeny / BOX_TILE_HEIGHT;
    return true;
}


BoxSprite* Game::newBoxAt(int mapX, int mapY, BoxId boxId) {
    BoxSprite* boxSprite = boxFactory->create(boxId);
    if (boxSprite) {
        boxSprite->setPos( posAt(mapX, mapY) );
        //boxMap->at(mapX, mapY
        boxMap->putBox(mapX, mapY, boxSprite);
    }
    
    return boxSprite;
} 

// moves a block of boxes to the left
MoveStatus Game::moveBlockLeft(int top, int left, int pastBottom, int pastRight) {
    for (int i = left; i < pastRight; i++) {
        for (int j=top; j< pastBottom; j++) {
            BoxSprite*& srcMapPos = boxMap->at(i, j);
            if ( srcMapPos ) {
                BoxSprite* movedSprite = srcMapPos;
                if (i > 0) { // make sure we didn't reach the left border
                    BoxSprite*& destMapPos = boxMap->at(i-1,j);
                    if (destMapPos) {
                        errorLog << "Cannot move to the left. Tile already occupied: (" << i-1 << "," << j << ")\n";
                        return MoveStatus::ALREADY_OCCUPIED;
                    } else {
                        destMapPos = srcMapPos;
                        srcMapPos = 0;
                        // set up animation
                        Animator* animator = animations->getAnimatorSlot();
                        Point2 targetPos = posAt(i-1,j);
                        animator->set(movedSprite, targetPos,30);
                    }
                
                } else {
                    return MoveStatus::PAST_LEFT_LIMITS;
                }
            }
        }
    }
    return MoveStatus::OK;
}

// moves a block of boxes to the left
MoveStatus Game::moveBlockRight(int top, int left, int pastBottom, int pastRight) {
    for (int i = pastRight-1; i >= left; i--) {
        for (int j=top; j< pastBottom; j++) {
            BoxSprite*& srcMapPos = boxMap->at(i, j);
            if ( srcMapPos ) {
                BoxSprite* movedSprite = srcMapPos;
                if (i+1 < boxMap->width) { // make sure we didn't reach the right border
                    BoxSprite*& destMapPos = boxMap->at(i+1,j);
                    if (destMapPos) {
                        errorLog << "Cannot move to the right. Tile already occupied: (" << i+1 << "," << j << ")\n";
                        return MoveStatus::ALREADY_OCCUPIED;
                    } else {
                        destMapPos = srcMapPos;
                        srcMapPos = 0;
                        // set up animation
                        Animator* animator = animations->getAnimatorSlot();
                        Point2 targetPos = posAt(i+1,j);
                        animator->set(movedSprite, targetPos,30);
                    }
                
                } else {
                    return MoveStatus::PAST_RIGHT_LIMITS;
                }
            }
        }
    }
    return MoveStatus::OK;
}

MoveStatus Game::moveColumnRight(int i, int posCount) {
    if ( i+posCount >= boxMap->width)
        return MoveStatus::PAST_RIGHT_LIMITS;
        
    for (int j=0; j<boxMap->height; j++) {
        BoxSprite*& srcMapPos = boxMap->at(i, j);
        BoxSprite*& destMapPos = boxMap->at(i+posCount, j);
        if (srcMapPos) {
            BoxSprite* movedSprite = srcMapPos;
            if (destMapPos) {
                errorLog << "Cannot move column to the right. Tile already occupied: (" << i+posCount << "," << j << ")\n";
                return MoveStatus::ALREADY_OCCUPIED;
            } else {
                destMapPos = srcMapPos;
                srcMapPos = 0;
                // set up animation
                Animator* animator = animations->getAnimatorSlot();
                Point2 targetPos = posAt(i+posCount, j);
                animator->set(movedSprite, targetPos,30);            
            }
        }
    }
    return MoveStatus::OK;
}

GameStatus Game::newColumn() {
    MoveStatus status = moveBlockLeft(0,0,boxMap->height, boxMap->width);
    if (status == MoveStatus::OK) {
        for (int j=0; j < boxMap->height; j++) {
            BoxSprite* boxSprite = boxFactory->create(BoxId::RANDOM_BOX);
            if (boxSprite) {
                boxSprite->setPos( posAt(boxMap->width, j) );
                boxMap->putBox(boxMap->width-1, j, boxSprite);
                Animator* animator = animations->getAnimatorSlot();
                Point2 targetPos = posAt(boxMap->width-1,j);
                animator->set(boxSprite, targetPos,30);
            }            
        }
        return GameStatus::GAME_OK;
    } else 
    if (status == MoveStatus::PAST_LEFT_LIMITS) {
        return GameStatus::GAME_OVER;
    }

    return GameStatus::GAME_ERROR; // unexpected behavior
}

// clicked box are discarded using this function
// references BoxMap sprite location
void Game::discardBox(BoxSprite*& discardedSprite) {
    delete discardedSprite;
    discardedSprite = 0;
    // TODO start an animation to make sprite disappear
}

// searches recursively for boxes with the same color as one at (tilex,tiley)
void Game::discardSameColor(int tilex, int tiley, int& discardedCount, BoxId prevBoxId) {
    BoxSprite*& currentBox = boxMap->at(tilex, tiley);
    
    if (currentBox == 0 || &currentBox == &BoxMap::OUT_OF_LIMITS) {
        return; // empty tile or tile out of map bounds
    } else {
        BoxId currentBoxId = currentBox->boxId;
        if (currentBoxId == prevBoxId) {
            discardBox(currentBox);
            discardedCount ++;
        }
        if (currentBoxId == prevBoxId || prevBoxId == UNDEFINED_BOX) {
            discardSameColor(tilex-1, tiley, discardedCount, currentBoxId);
            discardSameColor(tilex, tiley-1, discardedCount, currentBoxId);
            discardSameColor(tilex+1, tiley, discardedCount, currentBoxId);
            discardSameColor(tilex, tiley+1, discardedCount, currentBoxId);
        }
    }
}

// makes unsupported boxes fall and creates animations for them
// returns number of boxes that fell
int Game::gravityEffect() {
    int movedCount = 0;
    for (int i=0; i < boxMap->width; i++) {
        
        int countEmpty = 0;
        int j = boxMap->height-1;
        BoxSprite* boxSprite = 0;
        boxSprite = boxMap->at(i,j);
        //  walk until empty sprite
        while ( j>= 0 && boxSprite ) {
            j--;
            boxSprite = boxMap->at(i,j);
        }
        // count empty tiles
        while ( j >= 0 && !boxSprite) {
            while ( j>=0 && !boxSprite ) {
                countEmpty ++;
                
                j--;
                boxSprite = boxMap->at(i,j);
            }            
            // first non-empty should fall
            if (boxSprite) {
                boxMap->at(i,j+countEmpty) = boxMap->at(i,j);
                boxMap->at(i,j) = 0;
                movedCount ++;
                Animator* animator = animations->getAnimatorSlot();
                Point2 targetPos = posAt(i,j+countEmpty);
                animator->set(boxSprite, targetPos,30);

                boxSprite = 0; // we 'll keep searching for empty boxes upwards
                countEmpty --;
            }        
        }
    }
    return movedCount;
}

// assumes valid column index (i) value
bool Game::columnEmpty(int i) {
    for (int j=0; j<boxMap->height; j++) {
        if (boxMap->at(i,j))
            return false;
            
    }
    return true;
}

// rightward condensing of column gaps
GameStatus Game::condense() {
    int i = boxMap->width-1; // starting from the right edge
    
    while ( i>=0 && !columnEmpty(i) ) {
        i--;
    }
    // count empty columns
    int countEmpty = 0;
    while (i >=0 ) {
        while ( i>=0 && columnEmpty(i) ) {
            countEmpty ++;
            i--;
        }
        // probably found a non-empty column
        if ( i >=0 ) {
            if ( moveColumnRight(i, countEmpty) != MoveStatus::OK ) 
                return GameStatus::GAME_ERROR;
            i --;
        }
    }
    return GameStatus::GAME_OK;
}
    
bool Animator::tick() {
    if (steps <= 0) {
        warningLog << "trying to move a sprite that has already reached its destination. Maybe done-event missed ?\n";
        return true;
    } else 
    if (steps == 1) {
        sprite->pos = toPos;
        steps --;
        return true;
    } else {
        float stepX = (toPos.x - sprite->pos.x)/(float)steps;
        float stepY = (toPos.y - sprite->pos.y)/(float)steps;
        sprite->pos.x += stepX;
        sprite->pos.y += stepY;
        steps --;
        return false;
    }
}

// go through animation slots and tick each one of them
void Animations::tick() {
    AnimatorPool::Index it, nextit;

    it = animators.iter();

    Animator* animp;
    while (it != -1) {
        nextit = animators.nextp(it, animp);
        if (animp->tick())  // returns true finished
            animators.release(it); // current (it) can be released since we've already got next one
        it = nextit;
    }
}

// return an available animator and mark it as non-finished
Animator* Animations::getAnimatorSlot() {
    Animator* animatorp;
    AnimatorPool::Index i = animators.getp(animatorp);
    if (i == -1) {
        errorLog << "no animator slots available" << "\n";
        return 0;
    }

    animatorp->removeIndex = i;
    return animatorp;
}




