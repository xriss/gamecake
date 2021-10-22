
#include "SDL.h"


int SDL_main(int argc, char* argv[]) {

    SDL_Window *window;                    // Declare a pointer
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        640,                               // width, in pixels
        480,                               // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );

    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

	for( int i=1 ; i<100 ; i++ )
	{
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"WINRAR %d",i);
	}

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    return 0;
}
