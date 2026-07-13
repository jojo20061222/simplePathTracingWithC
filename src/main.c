#include "dynamicArray.h"
#include "multithreads.h"
#include "render.h"
#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

const int width = 1080;
const int height = 720;

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Rotating Cube", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_METAL);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                             SDL_TEXTUREACCESS_STREAMING, width, height);
    SDL_Event event;
    bool isRunning = true;

    VirtualScreen *vs = newScreen(width, height);
    Point camera = {(double)width / 2, (double)height, -4000};

    Object **objects = getDynamicArray(2, Object *);

    Triangle tri =
        newTriangle((Point){100, 0, 1500}, (Point){0, height, 1500}, (Point){width, 0, 1500});
    Object obj2 = (Object){(Shape *)&tri, {255.0 / 255, 100.0 / 255, 100.0 / 255}, 0, 0};
    objects[0] = &obj2;

    Sphere sph = newSphere((Point){300, 300, 1550}, 100);
    Object obj3 = (Object){(Shape *)&sph, {255.0 / 255, 255.0 / 255, 100.0 / 255}, 0, 0};
    objects[1] = &obj3;

    double angle = 0;

    Multithreads *mul =
        initMultithreads(vs, 11, (const Object **)objects, getSize(objects), &camera);

    while (isRunning) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) isRunning = false;
        }

        angle += 2;

        waitBarrier(mul->start);
        waitBarrier(mul->end);

        SDL_UpdateTexture(texture, NULL, vs->buffer, width * sizeof(ScreenColor));
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(5);
    }

    destroyMultithreads(mul);

    freeDynamicArray(objects);
    freeScreen(vs);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}