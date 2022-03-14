#include "engine.h"
#include <SDL_image.h>
#include "utils.h"


// statically linked global var
extern LogStream errorLog; 
extern LogStream warningLog; 
extern LogStream infoLog;

bool Engine::initialize() {

	// Initialize SDL. SDL_Init will return -1 if it fails.
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		errorLog << "Error initializing SDL: " << SDL_GetError() << "\n";
		return false;
	}

	//window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
    window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 896, 640, SDL_WINDOW_SHOWN);
	if (!window) {
		errorLog << "Error creating window: " << SDL_GetError() << "\n";
        SDL_Quit();
		return false;
	}
    
    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED); // TODO fallback to software rendering, check SDL_RENDERER_PRESENTVSYNC
    if (!renderer) {
        errorLog << "Error creating renderer: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
    
    return true;
}

void Engine::close() {
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
	SDL_Quit();
}

void MouseState::update() {
    
    if (!initialized) {
        SDL_PumpEvents();
        previousMouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
        initialized = true;
    } else {
        Uint32 mouseButtons;
        SDL_PumpEvents();

        mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
        
        if ( (previousMouseButtons & SDL_BUTTON_LMASK)==0 && (mouseButtons & SDL_BUTTON_LMASK)!=0 ) {
            leftPressed = true;
        } else {
            leftPressed = false;
        }
        
        if ( (previousMouseButtons & SDL_BUTTON_LMASK)!=0 && (mouseButtons & SDL_BUTTON_LMASK)==0 ) {
            leftReleased = true;
        } else {
            leftReleased = false;
        }  
        
        previousMouseButtons = mouseButtons;          
    }

}


Texture::~Texture() {
    if (sdlTexture) {
        SDL_DestroyTexture(sdlTexture);
    }
}

Resources::Resources(SDL_Renderer* renderer, const char* rootPath, int capacity ) : renderer(renderer), capacity(capacity) {
    strncpy(this->rootPath, rootPath, MAX_FILEPATH_SIZE); // keep a local copy  // for linux
    //strncpy_s(this->rootPath, rootPath, MAX_FILEPATH_SIZE); // keep a local copy // for win
    this->rootPath[MAX_FILEPATH_SIZE-1] = 0; // null-terminate just in case
    textures = new Texture[capacity];
}

Resources::~Resources() {
    delete [] textures;
}


bool Resources::init() {
    int flags=IMG_INIT_JPG|IMG_INIT_PNG;
    int initted=IMG_Init(flags);
    if((initted&flags) != flags) {
        errorLog << "error starting image loader " << " : '" << IMG_GetError() << "\n";
        return false;
    }
    return true;	
}

bool Resources::loadImage(const char* imagefile, SDL_Texture*& texture, int& w, int& h) {
    
    // load sample.png into image
    SDL_Surface *image;
    image = IMG_Load(imagefile);
    if (!image) {
        errorLog << "IMG_Load: " << IMG_GetError() << "\n";
        return false;
    } else {
        infoLog << "Loaded image " << imagefile << " " << image->w << "X" << image->h << "\n";
    }
            
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, image);
    if (tex == NULL) {
        errorLog << "CreateTextureFromSurface failed: " << SDL_GetError() << "\n";
        SDL_FreeSurface(image);
        return false;
    } else {
        texture = tex;
        SDL_FreeSurface(image);
        w = image->w;
        h = image->h;
        return true;
    }
}

// load an image file, create a texture for it and bind it with an identifier (see game.h:ImageId)
bool Resources::registerImage(const char* imagefile, int imageId) {
    SDL_Texture* sdlTexture;
    int w, h;
    if (textures[imageId].sdlTexture) {
        warningLog << "registerImage: texture already set for " << imageId;
    } else {
        if (loadImage(imagefile, sdlTexture, w, h)) {     
            Texture& texture = textures[imageId];
            texture.sdlTexture = sdlTexture;
            texture.w = w;
            texture.h = h;
            
            return true;
        }
    }
    return false;
    
}

// return a texture wrapper by identifier
Texture* Resources::getImage(const int imageId) {
    return &textures[imageId];
}

// release image fascilities
void Resources::done() {
    IMG_Quit();
}


// if no blit width/height given will use the width/height of the texture
RenderableBitmap::RenderableBitmap(Texture* texture, int blitWidth, int blitHeight) : sdlTexture(texture->sdlTexture) {
    sourceRect = new SDL_Rect();
    sourceRect->x = 0;
    sourceRect->y = 0;
    sourceRect->w = texture->w;
    sourceRect->h = texture->h;
    
    this->blitWidth = blitWidth ?  blitWidth : texture->w;
    this->blitHeight = blitHeight ? blitHeight : texture->h;
}

RenderableBitmap::~RenderableBitmap() {
    if (sourceRect)
        delete sourceRect;
}

void RenderableBitmap::render(float x, float y, SDL_Renderer* renderer) {
    SDL_Rect destRect;
    destRect.x = x;
    destRect.y = y;
    destRect.w = blitWidth;
    destRect.h = blitHeight;
    SDL_RenderCopy(renderer, sdlTexture, sourceRect, &destRect);
}


void Sprite::setPos(float x, float y) {
    // TODO - check limits ?
    pos.x = x;
    pos.y = y;
}

void Sprite::render(SDL_Renderer* renderer) {
    renderable->render(pos.x, pos.y, renderer);
}

bool Animator::tick() {
    if (steps <= 0) {
        warningLog << "trying to move a sprite that has already reached its destination. Maybe done-event missed ?\n";
        return true;
    } else
    if (steps == 1) {
        sprite->pos = toPos;
        steps --;
        return true;
    } else {
        float stepX = (toPos.x - sprite->pos.x)/(float)steps;
        float stepY = (toPos.y - sprite->pos.y)/(float)steps;
        sprite->pos.x += stepX;
        sprite->pos.y += stepY;
        steps --;
        return false;
    }
}

// go through animation slots and tick each one of them
void Animations::tick() {
    AnimatorPool::Index it, nextit;

    it = animators.iter();

    Animator* animp;
    while (it != -1) {
        nextit = animators.nextp(it, animp);
        if (animp->tick())  // returns true finished
            animators.release(it); // current (it) can be released since we've already got next one
        it = nextit;
    }
}

// return an available animator and mark it as non-finished
Animator* Animations::getAnimatorSlot() {
    Animator* animatorp;
    AnimatorPool::Index i = animators.getp(animatorp);
    if (i == -1) {
        errorLog << "no animator slots available" << "\n";
        return 0;
    }

    animatorp->removeIndex = i;
    return animatorp;
}
