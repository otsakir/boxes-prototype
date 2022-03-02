#ifndef _GAME_H_
#define _GAME_H_

#include "engine.h"
#include <string.h>  // includes memset() for windows

#define BOX_TILE_WIDTH 64.0
#define BOX_TILE_HEIGHT 64.0

enum ImageId {
    RED_BLOCK,
    BLUE_BLOCK,
    ORANGE_BLOCK,
    GREY_BLOCK,
    BROWN_BLOCK,
    GREEN_BLOCK,   
    
     
};

enum BoxId {
    RED_BOX = 1, // need to number them in order to randomize
    BLUE_BOX = 2,
    ORANGE_BOX = 3,
    GREY_BOX = 4,
    BROWN_BOX = 5,
    GREEN_BOX = 6,
    
    RANDOM_BOX = 7, // special ids
    UNDEFINED_BOX = 8
};


// status of moving boxes on the map
enum MoveStatus {
    OK,
    PAST_LIMITS,
    PAST_LEFT_LIMITS,
    PAST_RIGHT_LIMITS,
    ALREADY_OCCUPIED
};


enum GameStatus {
    GAME_OK,
    GAME_OVER,
    GAME_ERROR
};


class BoxAnimator;
class Sprite;




// The fundamental gameplay unit. A colored brick in a wall with many others.
class BoxSprite : public Sprite {
public:
    BoxId boxId;

    BoxSprite(Renderable* renderable, BoxId boxId) : Sprite(renderable), boxId(boxId) {}
};



// Core gameplay data structure. Defines a rectangular map with clickable colored boxes that fall, collapse and disappear under conditions
struct BoxMap {
    
    static BoxSprite* OUT_OF_LIMITS; // see Sprite*& at(int tilex, int tiley) below on how to use this

    int width; // number of boxes in x
    int height;  // number of boxes in y
    
    BoxSprite** boxes = 0; // BoxMap owns the sprites

    BoxMap(int width, int height) : width(width), height(height) {        
        boxes = new BoxSprite*[width*height];
        memset(boxes, 0, width*height*sizeof(boxes[0])); // initialize
    }
    
    ~BoxMap() {
        if (boxes) {
            delete [] boxes;
            boxes = 0;
        }
    }
    
    void renderBoxes(SDL_Renderer* renderer);
    void putBox(int posX, int posY, BoxSprite* boxSprite);    
    inline int getWidth() { return width; }
    inline int getHeight() { return height; }
    
    BoxSprite*& at(int tilex, int tiley);  // to check if tile out of map limits : if &boxMap->at(mapx, mapy) == &BoxMap::OUT_OF_LIMITS
    
};

// knows how to build boxes
class BoxFactory {
private:
    Resources* resources;

public:
    BoxFactory(Resources* resources) : resources(resources) {}

    BoxSprite* create(BoxId boxId);
       
};

// moving a sprite is done by an animator. Knows the final destinations (toPos). Set 'finished' to mark it done.
struct Animator {
    Sprite* sprite;
    Point2 toPos;
    int steps; // how many steps/frames remaining
    bool finished;
    
    Animator() : sprite(0), steps(0), finished(true) {}
        
    // Move sprite one step further. Returns true when done.
    bool tick();

    // initiate the animation
    void set(Sprite* sprite, Point2 pos, int steps) {
        this->sprite = sprite;
        this->toPos = pos;
        this->steps = steps; // TODO - make this parametric
        this->finished = false;
    }
    
};

// a (not efficient) pool for Animators
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

// high level game api
class Game {
private:
    void discardBox(BoxSprite*& discardedSprite);
    bool columnEmpty(int i);

public:
    Point2 pos; // top-left corner
    Uint32 columnFeedPeriod = 5000; // in millisec

    BoxMap* boxMap;
    BoxFactory* boxFactory;
    Animations* animations;
        
    Game(BoxMap* boxMap, BoxFactory* boxFactory, Animations* animations) : boxMap(boxMap), boxFactory(boxFactory), animations(animations) {}
    
    // screen coordinates for box at tilex,tiley map position
    Point2 posAt(int tilex, int tiley);    
    bool tileXYAt(int screenx, int screeny, int& tilex, int& tiley);
    // high level box creation 
    BoxSprite* newBoxAt(int mapX, int mapY, BoxId boxId);
    MoveStatus moveBlockLeft(int top, int left, int pastBottom, int pastRight);
    MoveStatus moveBlockRight(int top, int left, int pastBottom, int pastRight);
    MoveStatus moveColumnRight(int i, int posCount);
    GameStatus newColumn(); // a new column is added periodically to the right and all boxes are moved to the left
    void discardSameColor(int tilex, int tiley, int& discardedCount, BoxId prevBoxId = UNDEFINED_BOX);
    int gravityEffect();
    GameStatus condense();

};




#endif
