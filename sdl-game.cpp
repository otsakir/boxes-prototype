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

  
// You must include the command line parameters for your main function to be recognized by SDL
int main(int argc, char** args) {
    
	SDL_Surface* winSurface = 0;
	SDL_Window* window = 0;
    SDL_Renderer* renderer = 0;
    Resources* resources = 0;
    BoxFactory* boxFactory = 0;
    Level* level = 0;
    MouseState* mouseState = 0;

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
    resources->registerImage("../files/blue.png", BLUE_BLOCK);
    resources->registerImage("../files/orange.png", ORANGE_BLOCK);
    resources->registerImage("../files/grey.png", GREY_BLOCK);
    resources->registerImage("../files/brown.png", BROWN_BLOCK);
    resources->registerImage("../files/green.png", GREEN_BLOCK);
    
    mouseState = new MouseState();
    mouseState->update();
    gBoxMap = new BoxMap(14, 10);
    Animations* animations = new Animations(140);
    boxFactory = new BoxFactory(resources);
    level = new Level(gBoxMap, boxFactory, animations);


    //level->newBoxAt(9,9, BoxId::RED_BOX);
    //level->newBoxAt(6,8, BoxId::RED_BOX);

	SDL_Event ev;
	bool running = true;

	//unsigned int i = 0;
    int playerMapX = 0;
    int playerMapY = 0;
    int destMapX;
    int destMapY;
	// Main loop
    
	while (running) {
        
        // click and discard
        mouseState->update();
        if (mouseState->leftReleased) {
            int mouseReleasedTileX = 0;
            int mouseReleasedTileY = 0;
            if ( level->tileXYAt(mouseState->mouseX, mouseState->mouseY, mouseReleasedTileX, mouseReleasedTileY) ) {
                BoxSprite*& clickedSprite = gBoxMap->at(mouseReleasedTileX, mouseReleasedTileY);
                if (clickedSprite) {
                    infoLog << "Mouse released at tile (" << mouseReleasedTileX << "," << mouseReleasedTileY << ") - " << clickedSprite->boxId << "\n";
                    int discardedCount = 0;
                    level->discardSameColor(mouseReleasedTileX, mouseReleasedTileY, discardedCount, BoxId::UNDEFINED_BOX);
                    infoLog << discardedCount << " tiles discarded\n";
                    if ( !discardedCount ) {
                        playerMapX = mouseReleasedTileX;
                        playerMapY = mouseReleasedTileY;
                    }
                    
                } else {
                    infoLog << "Mouse released at tile (" << mouseReleasedTileX << "," << mouseReleasedTileY << ") - " << "no tile there\n";
                }                
            } else {
                infoLog << "Mouse released outside of box map\n";
            }
        }
        
        // gravity
        int movedCount = level->gravityEffect();
        if (movedCount)
            infoLog << "moved by 1st gravity: " << movedCount << "\n";
        movedCount = level->gravityEffect();
        if (movedCount)
            infoLog << "moved by 2nd gravity: " << movedCount << "\n";
        
        // condense empty columns
        level->condense();
        
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
                        BoxSprite*& srcMapPos = gBoxMap->at(playerMapX,playerMapY);
                        if (srcMapPos) { 
                            BoxSprite*& destMapPos = gBoxMap->at(destMapX, destMapY);
                            if (&destMapPos == &BoxMap::OUT_OF_LIMITS) {
                                warningLog << "Trying to move outside map\n";
                            } else {
                                BoxSprite* movedSprite = srcMapPos; // keep a copy of the sprite too build the animation
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
                    } else {
                        MoveStatus moveStatus;
                        GameStatus gameStatus;
                        switch (ev.key.keysym.sym) {
                            case SDLK_j:
                                moveStatus = level->moveBlockLeft(0,0,10,10);
                                playerMapX --;
                                if (moveStatus == MoveStatus::PAST_LEFT_LIMITS) {
                                    infoLog << "reached left limits - GAME OVER\n";
                                }
                            break;
                            case SDLK_k:
                                gameStatus = level->newColumn();
                                if (gameStatus == GameStatus::GAME_OK) {
                                    playerMapX --;
                                } else
                                if (gameStatus == GameStatus::GAME_OVER) {
                                    infoLog << "GAME OVER\n";
                                } 
                                
                            break;                            
                        }
                    }
                break;
			}
		}
        
        // animate
        animations->tick();
        
        //Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear( renderer );
        
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
    if (mouseState)
        delete mouseState;
    if (resources)
        delete resources;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
