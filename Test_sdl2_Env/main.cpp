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

    SDL_Window *window = SDL_CreateWindow("SDL2_window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
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
void testP();
void testP() {
    int a = 20;
    int b = 30;
    int c = 40;

    const int *p = &a;
    std::cout << "step 1 :" << *p << std::endl;
    p = &b;

    std::cout << *p << std::endl;
    std::cout << "step 2 :" << *p << std::endl;

    std::cout << "------------------------" << std::endl;
    int *const p2 = &a;
    std::cout << "step 3 :" << *p2 << std::endl;
    *p2 = c;
    std::cout << "step 4 :" << *p2 << std::endl;
}


int main() {

    openSDL2Window();
    return 0;
}
