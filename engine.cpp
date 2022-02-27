#include "engine.h"
#include <SDL_image.h>
#include "utils.h"


extern LogStream errorLog; // statically linked global var
extern LogStream warningLog; // statically linked global var

Resources::Resources(SDL_Renderer* renderer, int reserved, const char* rootPath) : renderer(renderer), reserved(reserved) {
    strncpy(this->rootPath, rootPath, MAX_FILEPATH_SIZE); // keep a local copy
    this->rootPath[MAX_FILEPATH_SIZE-1] = 0; // null-terminate just in case
    textures = new SDL_Texture*[reserved];
    memset(textures, 0, sizeof(textures[0])*reserved);
}

Resources::~Resources() {
    for (int i = 0; i < reserved; i++) {
        if (textures[i]) {
            SDL_DestroyTexture(textures[i]);
            textures[i] = 0;
        }
    }
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

bool Resources::loadImage(const char* imagefile, SDL_Texture*& texture) {
    
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

void Resources::registerImage(const char* imagefile, int imageId) {
    SDL_Texture* texture;
    if (loadImage(imagefile, texture)) {
        if (textures[imageId]) {
            warningLog << "registerImage: texture already set for " << imageId;
        } else {
            textures[imageId] = texture;
        }
    }
}

SDL_Texture* Resources::getImage(const int imageId) {
    return textures[imageId];
}

void Resources::done() {
    IMG_Quit();
}



RenderableBitmap::RenderableBitmap(SDL_Texture* texture, SDL_Rect& sourceRect) : RenderableBitmap(texture) {
    SDL_Rect* prect = new SDL_Rect();
    *prect = sourceRect;
    this->sourceRect = prect;
    // destination rectangle has the same width/height by default
    width = sourceRect.w;
    height = sourceRect.h;
}

RenderableBitmap::~RenderableBitmap() {
    if (sourceRect)
        delete sourceRect;
}

void RenderableBitmap::render(float x, float y, SDL_Renderer* renderer) {
    SDL_Rect destRect;
    destRect.x = x;
    destRect.y = y;
    destRect.w = width;
    destRect.h = height;
    SDL_RenderCopy(renderer, texture, sourceRect, &destRect);
}


void Sprite::setPos(float x, float y) {
    // TODO - check limits ?
    pos.x = x;
    pos.y = y;
}

void Sprite::render(SDL_Renderer* renderer) {
    renderable->render(pos.x, pos.y, renderer);
}

