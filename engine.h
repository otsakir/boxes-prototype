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

    Point2 operator-(const Point2& subtracted) const {
        Point2 result;
        result.x = x - subtracted.x;
        result.y = y - subtracted.y;
        return result;
    }
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


// forward declarations
struct Animations;

class Camera {
public:
    Point2 worldPos;

    Camera() : worldPos(Point2()) {} // place camera at world position (0,0)

    void place(float worldx, float worldy) {
        worldPos.x = worldx;
        worldPos.y = worldy;
    }
};

class Clipping {
    Point2 pos; // screen coordinates
    float width;
    float height;
public:
    Clipping(Point2& pos, float width, float height) : pos(pos), width(width), height(height) {}
    //Clipping(float width, float height) : width(width), height(height) {}

    // pos in screen coordinates
    void set(Point2 pos, float width, float height) {
        this->pos = pos;
        this->width = width;
        this->height = height;
    }

    // true if totally clipped
    bool clipped(const Point2& screenCoords, const float blitWidth, const float blitHeight, SDL_Rect& clippedRect) {
        // check if out of the viewport alltogether
        if (screenCoords.x+blitWidth <= this->pos.x || screenCoords.x >= this->pos.x + this->width)
            return true;
        if (screenCoords.y+blitHeight <= this->pos.y || screenCoords.y >= this->pos.y + this->height)
            return true;

        clippedRect.x = 0;
        clippedRect.w = blitWidth;
        if (screenCoords.x < this->pos.x) {
            clippedRect.x += this->pos.x-screenCoords.x;
            clippedRect.w -= clippedRect.x; //this->pos.x-screenCoords.x;
        }
        if (screenCoords.x+blitWidth > this->pos.x+this->width)
            clippedRect.w -= screenCoords.x+blitWidth - (this->pos.x+this->width);

        clippedRect.y = 0;
        clippedRect.h = blitHeight;
        if (screenCoords.y < this->pos.y) {
            clippedRect.y = this->pos.y-screenCoords.y;
            clippedRect.h -= clippedRect.y; //this->pos.y-screenCoords.y;
        }
        if (screenCoords.y+blitHeight > this->pos.y+this->height)
            clippedRect.h -= screenCoords.y+blitHeight - (this->pos.y+this->height);

        return false;
    }

};


class Engine {
public:
	SDL_Window* window = 0;
    SDL_Renderer* renderer = 0;
    MouseState mouseState;

    int windowWidth = 0; // of the window in pixels. Can change over time. TODO - keep track of the window size.
    int windowHeight = 0;

    Animations* animations; // not owned
    Camera* camera; // owned
    Clipping* clipping; // owned

    Engine(Animations* animations) : animations(animations) {
        camera = new Camera();
        Point2 clippingPos;
        clipping = new Clipping(clippingPos,0,0);
    }

    ~Engine() {
        delete clipping;
        delete camera;
    }

    bool initialize();
    void close();

    void worldToScreen(const Point2& worldCoords, Point2& destScreenCoords) {
        destScreenCoords = worldCoords - camera->worldPos;
    }
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
    virtual void render(float x, float y, SDL_Rect& clippedSourceRect, Engine* engine) = 0;

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

    virtual void render(float x, float y, SDL_Rect& clippedSourceRect, Engine* engine);

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
    void render(Engine* engine);
    
};

struct Animator; // forward declaration
typedef ListPool<Animator,int> AnimatorPool;

// moving a sprite is done by an animator. Knows the final destinations (toPos). Set 'finished' to mark it done.
struct Animator {
    AnimatorPool::Index removeIndex;
    Sprite* sprite;
    Point2 toPos;
    int steps; // how many steps/frames remaining

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
    AnimatorPool animators;

    Animations(int count = 10) :animators(count) {}
    ~Animations() {}

    // go through animation slots and tick each one of them
    void tick();
    Animator* getAnimatorSlot();

};



#endif
