#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <SDL.h>
#include "listpool.h"

#define MAX_FILEPATH_SIZE 128


struct Point2 {
    float x;
    float y;
    
    Point2() : x(0), y(0) {}
    Point2(float x, float y) : x(x), y(y) {}
};


// statefull mouse state
class MouseState {
private:
    bool initialized = false;
    Uint32 previousMouseButtons;

public:

    int mouseX;
    int mouseY;
    bool leftPressed = false;
    bool leftReleased = false;

    void update();
};


class Engine {
public:
	SDL_Window* window = 0;
    SDL_Renderer* renderer = 0;
    MouseState mouseState;

    bool initialize();
    void close();
};


// convenience wrapper class of SDL_Texture
class Texture {
public:
    SDL_Texture* sdlTexture = 0;
    int w = 0;
    int h = 0;
    
    ~Texture();
};


class Resources {
private:
    char rootPath[MAX_FILEPATH_SIZE];
    SDL_Renderer* renderer;
    Texture* textures ;
    int capacity; // number of slots in 'textures' 
    
    bool loadImage(const char* imagefile, SDL_Texture*& texture, int& w, int& h);
    
public:

    Resources(SDL_Renderer* renderer, const char* rootPath = "", int capacity = 16);
    ~Resources();

	bool init();
    bool registerImage(const char* imagefile, int imageId);
    Texture* getImage(const int imageId);
    void done(); // unload image loading stuff
    
};


// something rectangular that can be rendered to the screen
class Renderable {
private:
public:
	virtual void render(float x, float y, SDL_Renderer* renderer) = 0;
	float blitWidth = 10;
	float blitHeight = 10;   
};


// a renderable based on a raster graphic source
class RenderableBitmap : public Renderable {
private:
	SDL_Texture* sdlTexture = 0; // does not own texture
    SDL_Rect* sourceRect = 0; // source rectangle within texture. If null the whole texture is assumed. Owned by RenderableBitmap.
	
public:
    RenderableBitmap(Texture* texture, int blitWidth = 0, int blitHeight = 0);
    ~RenderableBitmap();

	virtual void render(float x, float y, SDL_Renderer* renderer);

};


class Sprite {
public:
    Renderable* renderable;
    Point2 pos;
    
    Sprite(Renderable* renderable) : renderable(renderable) {}
    
    void setPos(float x, float y);
    void setPos(const Point2& pos) {
        this->pos = pos;
    }
    void render(SDL_Renderer* renderer);
    
};

struct Animator; // forward declaration
typedef ListPool<Animator,int> AnimatorPool;

// moving a sprite is done by an animator. Knows the final destinations (toPos). Set 'finished' to mark it done.
struct Animator {
    AnimatorPool::Index removeIndex;
    Sprite* sprite;
    Point2 toPos;
    int steps; // how many steps/frames remaining
    //bool finished;


    Animator() : sprite(0), steps(0) {}

    // Move sprite one step further. Returns true when done.
    bool tick();

    // initiate the animation
    void set(Sprite* sprite, Point2 pos, int steps) {
        this->sprite = sprite;
        this->toPos = pos;
        this->steps = steps; // TODO - make this parametric
    }

};

// a (not efficient) pool for Animators
struct Animations {

    //Animator* animators;
    //int count; // how many animators
    AnimatorPool animators;

    Animations(int count = 10) :animators(count) {}

    ~Animations() {}

    // go through animation slots and tick each one of them
    void tick();
    // return an available animator and mark it as non-finished
    Animator* getAnimatorSlot();

};





#endif
