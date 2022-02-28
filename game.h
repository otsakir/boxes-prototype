#ifndef _GAME_H_
#define _GAME_H_

#include "engine.h"

#define BOX_TILE_WIDTH 64.0
#define BOX_TILE_HEIGHT 64.0

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



class BoxSprite : public Sprite {
public:

    BoxSprite(Renderable* renderable) : Sprite(renderable) {}
};



// a rectangular map with colored boxes where boxes move, disappear and all gameplay is implemented
struct BoxMap {
    
    static Sprite* OUT_OF_LIMITS; // see Sprite*& at(int tilex, int tiley) below

    int width; // number of boxes in x
    int height;  // number of boxes in y
    
    Sprite** boxes = 0; // BoxMap owns the sprites

    BoxMap(int width, int height) : width(width), height(height) {        
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
    inline int getWidth() { return width; }
    inline int getHeight() { return height; }
    
    Sprite*& at(int tilex, int tiley);  // to check if tile out of map limits : if &boxMap->at(mapx, mapy) == &BoxMap::OUT_OF_LIMITS
    
};

// knows how to build boxes
class BoxFactory {
private:
    Resources* resources;

public:
    BoxFactory(Resources* resources) : resources(resources) {}

    BoxSprite* create(BoxId boxId);
       
};



struct Level {

    Point2 pos; // top-left corner

    BoxMap* boxMap;
    BoxFactory* boxFactory;
        
    Level(BoxMap* boxMap, BoxFactory* boxFactory) : boxMap(boxMap), boxFactory(boxFactory) {}
    
    // screen coordinates for box at tilex,tiley map position
    Point2 posAt(int tilex, int tiley);    
    // high level box creation 
    BoxSprite* newBoxAt(int mapX, int mapY, BoxId boxId);

};


struct Animator {
    Sprite* sprite;
    Point2 toPos;
    int steps; // how many steps/frames remaining
    bool finished;
    
    Animator() : sprite(0), steps(0), finished(true) {}
        
    // move one step further
    // returns true when done
    bool tick();
    
    void set(Sprite* sprite, Point2 pos, int steps) {
        this->sprite = sprite;
        this->toPos = pos;
        this->steps = steps; // TODO - make this parametric
        this->finished = false;
    }
    
};

// a pool for Animators
struct Animations {

    Animator* animators;
    int count; // how many animators
    
    Animations(int count = 10) :count(count) {
        animators = new Animator[count];
    }
    
    ~Animations() {
        delete [] animators;
    }
    
    // go through animation slots and tick each one of them
    void tick();
    // return an available animator and mark it as non-finished
    Animator* getAnimatorSlot();
    
};


#endif
