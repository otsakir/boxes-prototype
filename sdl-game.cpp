// sdl-game.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "utils.h"
LogStream infoLog(std::cout);
LogStream errorLog(std::cerr);
LogStream warningLog(std::cerr);

#include "engine.h"
#include "game.h"


// global stuff (typically prefixed with 'g')
BoxMap* gBoxMap = 0;


/*    
struct Configuration {
    float boxWidth = 64; // default
    float boxHeight = 64;
};


Configuration configuration;
*/


/*
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
*/

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


#include "game.h"

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



	// Initialize SDL. SDL_Init will return -1 if it fails.
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		errorLog << "Error initializing SDL: " << SDL_GetError() << "\n";
		system("pause");
		// End the program
		return 1;
	}

	// create our window
	window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
	if (!window) {
		errorLog << "Error creating window: " << SDL_GetError() << "\n";
		system("pause");
		// End the program
		return 1;
	}
    
    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED); // TODO fallback to software rendering, check SDL_RENDERER_PRESENTVSYNC
    
    resources = new Resources(renderer);
    resources->registerImage("../files/red.png", RED_BLOCK);
    boxFactory = new BoxFactory(resources);
    gBoxMap = new BoxMap(10, 10, 64.0f, 64.0f);
    
    for (int j=0; j < gBoxMap->getHeight(); j++) {
        for (int i=0; i< gBoxMap->getWidth(); i++) {
            if (i != j) {
                BoxSprite* boxSprite = boxFactory->create(BoxId::RED_BOX);
                boxSprite->setPos(gBoxMap->tileWidth*i, gBoxMap->tileHeight*j);
                gBoxMap->putBox(i,j, boxSprite);
            }
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
                case SDL_KEYDOWN:
                    switch (ev.key.keysym.sym)
                    {
                        case SDLK_LEFT:  
                            infoLog << "left pressed\n";
                        break;
                        //case SDLK_RIGHT: x++; break;
                        //case SDLK_UP:    y--; break;
                        //case SDLK_DOWN:  y++; break;
                    }
                break;
			}
		}

		i++;
		//// Fill the window with a white rectangle
		//SDL_FillRect(winSurface, NULL, SDL_MapRGB(winSurface->format, 255, 255, i % 256 ));
		//// Update the window display
		//SDL_UpdateWindowSurface(window);

        //Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
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
        
        gBoxMap->renderBoxes(renderer);

        //Update screen
        SDL_RenderPresent( renderer );



		// Wait before next frame
		SDL_Delay(100);
	}

    // wrap up
    if (gBoxMap)
        delete gBoxMap;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
