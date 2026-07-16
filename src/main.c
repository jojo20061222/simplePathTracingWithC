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

    SDL_Window *window = SDL_CreateWindow("Path Tracing", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_METAL);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                             SDL_TEXTUREACCESS_STREAMING, width, height);
    SDL_Event event;
    bool isRunning = true;

    VirtualScreen *vs = newScreen(width, height);
    Point camera = {(double)width / 2, (double)height, -4000};

    Object **objects = getDynamicArray(1, Object *);
    /*
    Triangle tri1 =
        newTriangle((Point){980, 720, 0}, (Point){0, 720, 1500}, (Point){1080, 0, 1500});
    Object obj1 = (Object){(Shape *)&tri1, {255.0 / 255, 100.0 / 255, 100.0 / 255}, 0.9};
    objects[2] = &obj1;
    Triangle tri =
        newTriangle((Point){100, 0, 3000}, (Point){0, height, 1500}, (Point){width, 0, 1500});
    Object obj2 = (Object){(Shape *)&tri, {255.0 / 255, 100.0 / 255, 100.0 / 255}, 0.9};
    objects[0] = &obj2;
    */

    Sphere sph = newSphere((Point){300, 300, 1550}, 100);
    Object obj3 = (Object){(Shape *)&sph, {255.0 / 255, 255.0 / 255, 100.0 / 255}, 0.9};
    objects[0] = &obj3;

    Sphere lightSourceSph = newSphere((Point){700, -500, 1500}, 100);
    LightSource lightSource = {(Shape *)&lightSourceSph, {100, 100, 100}};

    Sphere lightSourceSph1 = newSphere((Point){-100, -500, 1200}, 150);
    LightSource lightSource1 = {(Shape *)&lightSourceSph1, {100, 100, 100}};
    LightSource **sources = getDynamicArray(2, LightSource *);
    
    sources[0] = &lightSource;
    sources[1] = &lightSource1;

    double angle = 0;

    Multithreads *mul = initMultithreads(vs, 11, (const Object **)objects, getSize(objects),
                                         (const LightSource **)sources, getSize(sources), &camera, 10);

    while (isRunning) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) isRunning = false;
        }

        // angle += M_PI / 180 * 2;
        // sph.center = (Vector){300, 300, 1500 + 300 * sin(angle)};

        SDL_UpdateTexture(texture, NULL, vs->buffer, width * sizeof(ScreenColor));
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(8);
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