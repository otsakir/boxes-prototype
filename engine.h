#ifndef _ENGINE_H_
#define _ENGINE_H_


#define MAX_FILEPATH_SIZE 128
//#define MAX_RESOURCE_IMAGES 100 // how much slots to allocate for images in the Resources objects

#include <SDL.h>


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

class Resources {
private:
    char rootPath[MAX_FILEPATH_SIZE];
    SDL_Renderer* renderer;
    SDL_Texture** textures;
    int reserved; // number of slots in 'textures' 
    
public:

	Resources(SDL_Renderer* renderer, int reserved = 16, const char* rootPath = "");
    ~Resources();

	bool init();
	bool loadImage(const char* imagefile, SDL_Texture*& texture);    
    void registerImage(const char* imagefile, int imageId);
    SDL_Texture* getImage(const int imageId);
    void done();
    

};


// something rectangular that can be rendered to the screen
class Renderable {
private:
public:
	virtual void render(float x, float y, SDL_Renderer* renderer) = 0;
	float width = 10;    // size when blitted to the destination 
	float height = 10;   // ...
};

// a renderable based on a raster graphic source
class RenderableBitmap : public Renderable {
private:
	SDL_Texture* texture = 0; // does not own texture
    SDL_Rect* sourceRect = 0; // source rectangle within texture. If null the whole texture is assumed. Owned by RenderableBitmap.
	
public:

    RenderableBitmap(SDL_Texture* texture) :  texture(texture) {}
    RenderableBitmap(SDL_Texture* texture, SDL_Rect& sourceRect);
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




#endif
