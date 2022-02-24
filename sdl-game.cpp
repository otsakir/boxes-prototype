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

/*
 * A box that can be blitted to the screen with position and size
 */
class Renderable {
private:
public:
	virtual void render() = 0;
	float x;
	float y;
	float width;
	float height;
};

class RenderableBitmap : public Renderable {
private:
	SDL_Texture* texture; // does not own texture
	
public:
	virtual void render(SDL_Renderer* renderer) {
		SDL_Rect destRect;
		destRect.x = x;
		destRect.y = y;
		destRect.w = width;
		destRect.h = height;
		SDL_RenderCopy(renderer, texture, NULL, &destRect);
	}
};



// You must include the command line parameters for your main function to be recognized by SDL
int main(int argc, char** args) {
    
	SDL_Surface* winSurface = 0;
	SDL_Window* window = 0;
    SDL_Renderer* renderer = 0;
    Resources* resources = 0;


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
    
/*
	// Get the surface from the window
	winSurface = SDL_GetWindowSurface(window);
	if (!winSurface) {
		cout << "Error getting surface: " << SDL_GetError() << endl;
		system("pause");
		// End the program
		return 1;
	}
*/

  /* Load an additional texture */
/*
  SDL_Surface* Loading_Surf;
  SDL_Texture* BlueShapes;  
  //Loading_Surf = SDL_LoadBMP("../files/red.bmp");
  Loading_Surf = IMG_Load("../files/red.png");
  BlueShapes = SDL_CreateTextureFromSurface(renderer, Loading_Surf);
  SDL_FreeSurface(Loading_Surf);
*/
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
        SDL_RenderCopy( renderer, resources->getImage(RED_BLOCK), NULL, NULL );
        //SDL_RenderCopy( renderer, BlueShapes, NULL, &destRect );

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
