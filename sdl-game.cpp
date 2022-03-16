#include "utils.h"
LogStream infoLog(std::cout);
LogStream errorLog(std::cerr);
LogStream warningLog(std::cerr);

#include "engine.h"
#include "game.h"

  
int main(int argc, char** args) {
    
    Animations* animations = new Animations(224);
    Engine engine(animations);

    if (!engine.initialize()) {
        return 1;
    }
    
    Resources* resources = 0;
    BoxMap* boxMap = 0;
    BoxFactory* boxFactory = 0;
    Game* game = 0;
    Uint32 lastFeedMillis; // count milllis since last time we fed a column

    resources = new Resources(engine.renderer);
    resources->init();
    resources->registerImage("./files/red.png", RED_BLOCK);
    resources->registerImage("./files/blue.png", BLUE_BLOCK);
    resources->registerImage("./files/orange.png", ORANGE_BLOCK);
    resources->registerImage("./files/grey.png", GREY_BLOCK);
    resources->registerImage("./files/brown.png", BROWN_BLOCK);
    resources->registerImage("./files/green.png", GREEN_BLOCK);
    resources->done(); 

    boxMap = new BoxMap(14, 8);
    boxFactory = new BoxFactory(resources);
    game = new Game(boxMap, boxFactory, &engine);
    game->mapPos.y = 64*2; // push some space at the top

    engine.clipping->set(Point2(50,50), 800,500);

    infoLog << "Press k to feed new columns manually\n";

    engine.mouseState.update(); // initialize mouse state

    lastFeedMillis = SDL_GetTicks();
	SDL_Event ev;
	bool running = true;
    int coolingDown = 0;

    // main loop
	while (running) {
        
        // discard same-color on click
        engine.mouseState.update();
        if (engine.mouseState.leftReleased) {
            int mouseReleasedTileX = 0;
            int mouseReleasedTileY = 0;
            if ( game->tileXYAt(engine.mouseState.mouseX, engine.mouseState.mouseY, mouseReleasedTileX, mouseReleasedTileY) ) {
                BoxSprite*& clickedSprite = game->boxMap->at(mouseReleasedTileX, mouseReleasedTileY);
                if (clickedSprite) {
                    infoLog << "Mouse released at tile (" << mouseReleasedTileX << "," << mouseReleasedTileY << ") - " << clickedSprite->boxId << "\n";
                    int discardedCount = 0;
                    game->discardSameColor(mouseReleasedTileX, mouseReleasedTileY, discardedCount, BoxId::UNDEFINED_BOX);
                    infoLog << discardedCount << " tiles discarded\n";                    
                } else {
                    infoLog << "Mouse released at tile (" << mouseReleasedTileX << "," << mouseReleasedTileY << ") - " << "no tile there\n";
                }                
            } else {
                infoLog << "Mouse released outside of box map\n";
            }
        }
        
        // gravity
        int movedCount = game->gravityEffect();
        if (movedCount)
            infoLog << "moved by 1st gravity: " << movedCount << "\n";
        movedCount = game->gravityEffect();
        if (movedCount)
            infoLog << "moved by 2nd gravity: " << movedCount << "\n";
        
        // condense empty columns
        game->condense();
        
		// event loop, generate new column on "k"
		while (SDL_PollEvent(&ev) != 0) {
			// check event type
			switch (ev.type) {
                case SDL_QUIT:
                    // shut down
                    running = false;
                break;
                case SDL_KEYDOWN:
                    MoveStatus moveStatus;
                    GameStatus gameStatus;
                    switch (ev.key.keysym.sym) {
                        case SDLK_k:
                            if (!coolingDown) {
                                gameStatus = game->newColumn();
                                coolingDown = 30; // prevent manually adding newColumn before 30 ticks
                                if (gameStatus == GameStatus::GAME_OVER) {
                                    infoLog << "GAME OVER\n";
                                }
                            }
                        break;   
                        case SDLK_c:
                            infoLog << animations->animators.getUsedCount() << "\n";
                        break;
                    }
                break;
			}
		}
        
        // feed a column from the right side when the time comes (see Game::columnFeedPeriod)
        Uint32 currentMillis = SDL_GetTicks();
        if (currentMillis - lastFeedMillis > game->columnFeedPeriod) {
            lastFeedMillis = currentMillis;
            if (game->newColumn() == GameStatus::GAME_OVER) {
                infoLog << "GAME OVER\n";
                break;
            } 
            coolingDown = 30;

        }
        
        // animate
        animations->tick();
        
        //Clear screen
        SDL_SetRenderDrawColor(engine.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(engine.renderer );
        
        // rendering
        game->boxMap->renderBoxes(&engine);

        // page flipping (?)
        SDL_RenderPresent(engine.renderer);

		// Wait before next frame. Rough assumption of a 60Hz monitor, 2ms for rendereing a 14ms for waiting. 1sec/60 = 16.6ms
		SDL_Delay(14);

        // decrease cooldown counter. Cooldown allows newColumn to be added only after the previous has settled.
        if (coolingDown > 0)
            coolingDown--;
	}


    if (game)
        delete game;
    if (boxFactory)
        delete boxFactory;
    if (animations)
        delete animations;
    if (boxMap)
        delete boxMap;
    if (resources)
        delete resources;
        
    engine.close();

	return 0;
}
