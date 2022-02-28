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
    Level* level = 0;

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
    gBoxMap = new BoxMap(10, 10);
    Animations* animations = new Animations(100);
    boxFactory = new BoxFactory(resources);
    level = new Level(gBoxMap, boxFactory);


    level->newBoxAt(5,4, BoxId::RED_BOX);
    level->newBoxAt(6,5, BoxId::RED_BOX);

	SDL_Event ev;
	bool running = true;

	//unsigned int i = 0;
    int playerMapX = 5;
    int playerMapY = 4;
    int destMapX;
    int destMapY;
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
                    if (ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_DOWN) {
                        destMapX = playerMapX;
                        destMapY = playerMapY;
                        switch (ev.key.keysym.sym)
                        {
                            case SDLK_LEFT:  
                                destMapX = playerMapX-1;
                                break;
                            case SDLK_RIGHT:
                                destMapX = playerMapX+1;
                                break;
                            case SDLK_UP:
                                destMapY = playerMapY-1;
                                break;
                            case SDLK_DOWN:
                                destMapY = playerMapY+1;
                                break;
                        }

                        // keep referenences of source and dest positions in map
                        Sprite*& srcMapPos = gBoxMap->at(playerMapX,playerMapY);
                        Sprite*& destMapPos = gBoxMap->at(destMapX, destMapY);
                        if (&destMapPos == &BoxMap::OUT_OF_LIMITS) {
                            warningLog << "Trying to move outside map\n";
                        } else {
                            Sprite* movedSprite = srcMapPos; // keep a copy of the sprite too build the animation
                            // move the sprite in BoxMap 
                            if (destMapPos == 0) { // empty ?
                                infoLog << "Moving the sprite...\n";
                                
                                destMapPos = srcMapPos;
                                srcMapPos = 0; // clear source position
                                // setup animation
                                Animator* animator = animations->getAnimatorSlot();
                                Point2 targetPos = level->posAt(destMapX,destMapY);
                                animator->set(movedSprite, targetPos,30);
                                
                                playerMapX = destMapX;
                                playerMapY = destMapY;
                            }
                        }
                    }
                break;
			}
		}
        
        // animate
        animations->tick();
        

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
		SDL_Delay(14);
	}

    if (level)
        delete level;
    // wrap up
    if (gBoxMap)
        delete gBoxMap;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
