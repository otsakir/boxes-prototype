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

    resources = new Resources(engine.renderer);
    resources->init();
    resources->registerImage("./files/red.png", RED_BLOCK);
    resources->registerImage("./files/blue.png", BLUE_BLOCK);
    resources->done();

    Renderable* renderable1 = new RenderableBitmap(resources->getImage(RED_BLOCK), 64, 64);
    Sprite* sprite1 = new Sprite(renderable1);
    sprite1->setPos(56,66);

    //Point2 p(20,10);
    Point2 p(0,0);
    engine.clipping->set(p, 100,100);

    infoLog << "Press k to feed new columns manually\n";

    engine.mouseState.update(); // initialize mouse state

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
        }


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
                        break;
                        case SDLK_c:
                            infoLog << animations->animators.getUsedCount() << "\n";
                        break;
                    }
                break;
            }
        }



        // animate
        animations->tick();

        //Clear screen
        SDL_SetRenderDrawColor(engine.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(engine.renderer );

        sprite1->render(&engine);

        // page flipping (?)
        SDL_RenderPresent(engine.renderer);

        // Wait before next frame. Rough assumption of a 60Hz monitor, 2ms for rendereing a 14ms for waiting. 1sec/60 = 16.6ms
        SDL_Delay(14);

        // decrease cooldown counter. Cooldown allows newColumn to be added only after the previous has settled.
        if (coolingDown > 0)
            coolingDown--;
    }

    delete sprite1;
    delete (RenderableBitmap*) renderable1;

    if (animations)
        delete animations;
    if (resources)
        delete resources;

    engine.close();

    return 0;
}
