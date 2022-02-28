#ifndef _GAME_H_
#define _GAME_H_

#include "engine.h"

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
    
    RANDOM_BOX = 7,
    UNDEFINED_BOX = 8
};

// when a box moves in the map, these are the available options
enum BoxMoveDirection {
    MOVE_LEFT = 1,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN
};

// status of moving boxes on the map
enum MoveStatus {
    OK,
    PAST_LIMITS,
    PAST_LEFT_LIMITS,
    ALREADY_OCCUPIED
};

enum GameStatus {
    GAME_OK,
    GAME_OVER,
    GAME_ERROR
};


class BoxAnimator;
class Sprite;



class BoxSprite : public Sprite {
public:
    BoxId boxId;

    BoxSprite(Renderable* renderable, BoxId boxId) : Sprite(renderable), boxId(boxId) {}
};



// a rectangular map with colored boxes where boxes move, disappear and all gameplay is implemented
struct BoxMap {
    
    static BoxSprite* OUT_OF_LIMITS; // see Sprite*& at(int tilex, int tiley) below

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

// high level game api
class Level {
private:
    void discardBox(BoxSprite*& discardedSprite);

public:
    Point2 pos; // top-left corner

    BoxMap* boxMap;
    BoxFactory* boxFactory;
    Animations* animations;
        
    Level(BoxMap* boxMap, BoxFactory* boxFactory, Animations* animations) : boxMap(boxMap), boxFactory(boxFactory), animations(animations) {}
    
    // screen coordinates for box at tilex,tiley map position
    Point2 posAt(int tilex, int tiley);    
    bool tileXYAt(int screenx, int screeny, int& tilex, int& tiley);
    // high level box creation 
    BoxSprite* newBoxAt(int mapX, int mapY, BoxId boxId);
    MoveStatus moveBlockLeft(int top, int left, int pastBottom, int pastRight); // TODO - define return value, game-over etc.
    GameStatus newColumn(); // a new column is added periodically to the right and all boxes are moved to the left
    void discardSameColor(int tilex, int tiley, int& discardedCount, BoxId prevBoxId = UNDEFINED_BOX);

};




#endif
