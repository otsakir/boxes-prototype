#ifndef _GAME_H_
#define _GAME_H_

#include "engine.h"

enum ImageId {
    BLOCK1 = 1,
    RED_BLOCK = 2,
    
};

enum BoxId {
    RED_BOX = 1
};

// when a box moves in the map, these are the available options
enum BoxMoveDirection {
    MOVE_LEFT = 1,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN
};


class BoxAnimator;
class Sprite;

// a rectangular map with colored boxes where boxes move, disappear and all gameplay is implemented
class BoxMap {
private:
    int width; // number of boxes in x
    int height;  // number of boxes in y
    
    Sprite** boxes = 0; // BoxMap owns the sprites
    
public:

    // status of a position in the map. Can be empty, full or invalid
    enum PosStatus {
        PosEmpty,
        PosFull,
        PosInvalid
    };

    BoxMap() {
        width = 10;
        height = 10;
        
        boxes = new Sprite*[width*height];
        memset(boxes, 0, width*height*sizeof(boxes[0])); // initialize
    }
    
    ~BoxMap() {
        if (boxes) {
            delete [] boxes;
            boxes = 0;
        }
    }
    
    void renderBoxes(SDL_Renderer* renderer);
    void putBox(int posX, int posY, Sprite* boxSprite);    
    PosStatus boxAt(int posX, int posY, Sprite*& sprite);
    void onBoxMoveDone(BoxAnimator* boxAnimator);
    inline int getWidth() { return width; }
    inline int getHeight() { return height; }
    
};

struct BoxAnimator {
    BoxMap* boxMap;
    Sprite* sprite;
    float toX;
    float toY;
    float fromX;
    float fromY;
    int steps; // how many steps/frames remaining
    bool finished = false;

    int fromMapX; // x/y placement in BoxMap
    int fromMapY;
    int toMapX;
    int toMapY;
    
    BoxAnimator(BoxMap* boxMap) : boxMap(boxMap) {}
    
    // move one step further
    // returns true when done
    bool tick();
};










#endif
