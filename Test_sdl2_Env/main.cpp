#include <SDL2/SDL.h>
#include <iostream>
const int width = 400;
const int height = 400;
void openSDL2Window();

void openSDL2Window() {
    int result = -1;
    if ((result = SDL_Init(SDL_INIT_EVERYTHING)) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Window *window = SDL_CreateWindow("SDL2_windowq", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                                          SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Event windowEvent;
    while (true) {
        if (SDL_PollEvent(&windowEvent)) {
            switch (windowEvent.type) {
                case SDL_QUIT:
                    std::cerr << "SDL_QUIT" << std::endl;
                    break;
            }
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
}
int main() {

    openSDL2Window();
    return 0;
}
