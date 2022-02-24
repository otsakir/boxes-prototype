// sdl-game.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>

// You shouldn't really use this statement, but it's fine for small programs
using namespace std;

#define MAX_FILEPATH_SIZE 128
#define MAX_RESOURCE_IMAGES 100 // how much slots to allocate for images in the Resources objects

class LogStream {
private:
	std::ostream& out;
public:

	LogStream(std::ostream& out) : out(out) {}

	LogStream& operator<<(const char* arg) {
		out << arg;
		out.flush();
		return *this;
	}
    
	LogStream& operator<<(int arg) {
		out << arg;
		out.flush();
		return *this;
	}
    
};

LogStream infoLog(std::cout);
LogStream errorLog(std::cerr);
LogStream warningLog(std::cerr);


enum ImageId {
    BLOCK1 = 1,
    RED_BLOCK = 2,
    
};

enum BoxId {
    RED_BOX = 1
};
    
struct Configuration {
    float boxWidth = 64; // default
    float boxHeight = 64;
};


Configuration configuration;


class Resources {
private:
    char rootPath[MAX_FILEPATH_SIZE];
    SDL_Renderer* renderer;
    SDL_Texture* textures[MAX_RESOURCE_IMAGES];
    
public:

	Resources(SDL_Renderer* renderer, const char* rootPath = "") {
		strncpy(this->rootPath, rootPath, MAX_FILEPATH_SIZE); // keep a local copy
		this->rootPath[MAX_FILEPATH_SIZE-1] = 0; // null-terminate just in case
        this->renderer = renderer;
        memset(textures, 0, sizeof(textures));
	}
    
    ~Resources() {
        for (int i = 0; i < MAX_RESOURCE_IMAGES; i++) {
            if (textures[i]) {
                SDL_DestroyTexture(textures[i]);
                textures[i] = 0;
            }
        }
    }
	

	bool init() {
		int flags=IMG_INIT_JPG|IMG_INIT_PNG;
		int initted=IMG_Init(flags);
		if((initted&flags) != flags) {
			errorLog << "error starting image loader " << " : '" << IMG_GetError() << "\n";
			return false;
		}
		return true;	
	}
	
	bool loadImage(const char* imagefile, SDL_Texture*& texture) {
		
		// load sample.png into image
        SDL_Surface *image;
		image = IMG_Load(imagefile);
		if (!image) {
			errorLog << "IMG_Load: " << IMG_GetError() << "\n";
			return false;
		}
                
		SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, image);
        if (tex == NULL) {
            errorLog << "CreateTextureFromSurface failed: " << SDL_GetError() << "\n";
            SDL_FreeSurface(image);
        } else {
            texture = tex;
            SDL_FreeSurface(image);
        }
		return true;
	}
    
    void registerImage(const char* imagefile, const ImageId imageId) {
        SDL_Texture* texture;
        if (loadImage(imagefile, texture)) {
            if (textures[imageId]) {
                warningLog << "registerImage: texture already set for " << imageId;
            } else {
                textures[imageId] = texture;
            }
        }
    }
    
    SDL_Texture* getImage(const ImageId imageId) {
        return textures[imageId];
    }
	
	void done() {
		IMG_Quit();
	}

	
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
    
    RenderableBitmap(SDL_Texture* texture, SDL_Rect& sourceRect) : RenderableBitmap(texture) {
        SDL_Rect* prect = new SDL_Rect();
        *prect = sourceRect;
        this->sourceRect = prect;
        // destination rectangle has the same width/height by default
        width = sourceRect.w;
        height = sourceRect.h;
    }
    
    ~RenderableBitmap() {
        if (sourceRect)
            delete sourceRect;
    }

	virtual void render(float x, float y, SDL_Renderer* renderer) {
		SDL_Rect destRect;
		destRect.x = x;
		destRect.y = y;
		destRect.w = width;
		destRect.h = height;
		SDL_RenderCopy(renderer, texture, sourceRect, &destRect);
	}
};

class Sprite {
public:
    Renderable* renderable;
    float x;
    float y;
    
    Sprite(Renderable* renderable) : renderable(renderable), x(0), y(0) {}
    
    void setPos(float x, float y) {
        // TODO - check limits ?
        this->x = x;
        this->y = y;
    }
    
    void render(SDL_Renderer* renderer) {
        renderable->render(x, y, renderer);
    }
};

class BoxSprite : public Sprite {
public:

    BoxSprite(Renderable* renderable) : Sprite(renderable) {}
};


// knows how to build boxes
class BoxFactory {
private:
    Resources* resources;
public:
    BoxFactory(Resources* resources) : resources(resources) {}

    BoxSprite* create(BoxId boxId) {
        if (boxId == BoxId::RED_BOX) {
            SDL_Texture* texture = resources->getImage(ImageId::RED_BLOCK);
            SDL_Rect sourceRect;
            sourceRect.x = 0;
            sourceRect.y = 0;
            sourceRect.w = 64;
            sourceRect.h = 64;
            RenderableBitmap* renderable = new RenderableBitmap(texture, sourceRect); // texture mem handled by Resources
            BoxSprite* boxSprite = new BoxSprite(renderable);
            return boxSprite;
        } else {
            warningLog << "unknown boxId: " << boxId << "\n";
            return 0;
        }
    }
};

// a rectangular map with colored boxes where boxes move, disappear and all gameplay is implemented
class BoxMap {
private:
    int width; // number of boxes in x
    int height;  // number of boxes in y
    
    Sprite** boxes = 0; // BoxMap owns the sprites
    
public:

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
    
    void renderBoxes(SDL_Renderer* renderer) {
        for (int i=0; i < width; i++) {
            for (int j=0; j<height; j++) {
                Sprite* sprite = boxes[ width*j + i ];
                if (sprite) {
                    sprite->render(renderer);
                }
            }
        }
    }
    
    void putBox(int posX, int posY, Sprite* boxSprite) {
        // TODO - what if boxSprite == 0
        if ( boxes[posY*width + posX] ) {
            warningLog << "BoxMap: there is already a box position (" << posX << "," << posY << ")\n";
        } else {
            boxes[posY*width + posX] = boxSprite;
        }
    }
    
    inline int getWidth() { return width; }
    
    inline int getHeight() { return height; }
    
};

class OreslikeGame {
public:


    void main() {
        // 1. ask user to get ready, press something, show "loading..." banner etc.
        
        // 2. load level, create and initialize boxmap
                
        // 3. gameplay loop - everything is set, keep rolling until game over
    }
};
    


// You must include the command line parameters for your main function to be recognized by SDL
int main(int argc, char** args) {
    
	SDL_Surface* winSurface = 0;
	SDL_Window* window = 0;
    SDL_Renderer* renderer = 0;
    Resources* resources = 0;
    BoxFactory* boxFactory = 0;
    BoxMap* boxMap = 0;


	// Initialize SDL. SDL_Init will return -1 if it fails.
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		system("pause");
		// End the program
		return 1;
	}

	// create our window
	window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
	if (!window) {
		cout << "Error creating window: " << SDL_GetError() << endl;
		system("pause");
		// End the program
		return 1;
	}
    
    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED); // TODO fallback to software rendering, check SDL_RENDERER_PRESENTVSYNC
    
    resources = new Resources(renderer);
    resources->registerImage("../files/red.png", RED_BLOCK);
    boxFactory = new BoxFactory(resources);
    boxMap = new BoxMap();
    
    for (int j=0; j < boxMap->getHeight(); j++) {
        for (int i=0; i< boxMap->getWidth(); i++) {
            BoxSprite* boxSprite = boxFactory->create(BoxId::RED_BOX);
            boxSprite->setPos(64.0f*i, 64.0f*j);
            boxMap->putBox(i,j, boxSprite);
        }
    }

	SDL_Event ev;
	bool running = true;

	unsigned int i = 0;
	// Main loop
	while (running) {

		// Event loop
		while (SDL_PollEvent(&ev) != 0) {
			// check event type
			switch (ev.type) {
			case SDL_QUIT:
				// shut down
				running = false;
				break;
			}
		}

		i++;
		//// Fill the window with a white rectangle
		//SDL_FillRect(winSurface, NULL, SDL_MapRGB(winSurface->format, 255, 255, i % 256 ));
		//// Update the window display
		//SDL_UpdateWindowSurface(window);

        //Clear screen
         SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear( renderer );

/*
        SDL_Rect destRect;
        destRect.x = 10;
        destRect.y = 10;
        destRect.w = 64;
        destRect.h = 64;
*/
        //Render texture to screen
        //SDL_RenderCopy( renderer, resources->getImage(RED_BLOCK), NULL, NULL );
        //SDL_RenderCopy( renderer, BlueShapes, NULL, &destRect );
        
        boxMap->renderBoxes(renderer);

        //Update screen
        SDL_RenderPresent( renderer );



		// Wait before next frame
		SDL_Delay(100);
	}

    // wrap up
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
