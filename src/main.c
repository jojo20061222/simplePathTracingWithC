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

    Object **objects = getDynamicArray(0, Object *);
    
    
    
    Point p1 = {150, 450, 1200};
    Point p2 = {150, 350, 1800};
    Point p3 = {450, 350, 1800};
    Point p4 = {450, 450, 1200};
    Triangle tri1 = newTriangle(p1, p2, p3);
    Triangle tri2 = newTriangle(p3, p4, p1);
    Object obj1 = newObject((Shape*)&tri1, (ObjectColor){1, 1, 1}, 0.1);
    Object obj2 = newObject((Shape*)&tri2, (ObjectColor){1, 1, 1}, 0.1);
    // pushBack(&objects, &obj1, Object*); 
    // pushBack(&objects, &obj2, Object*);

    Sphere sph = newSphere((Point){300, 300, 1500}, 100);
    Object obj3 = (Object){(Shape *)&sph, {0.0 / 255, 0.0 / 255, 250.0 / 255}, 0.9};
    pushBack(&objects, &obj3, Object*);

    Sphere sph2 = newSphere((Point){0, 0, -4000}, 8000);
    Object background = newObject((Shape*)&sph2, (ObjectColor){1, 1, 1}, 1);
    pushBack(&objects, &background, Object*);
    
    Sphere lightSourceSph = newSphere((Point){300, -400, 1500}, 100);
    LightSource lightSource = {(Shape *)&lightSourceSph, {1000, 1000, 1000}};
    
    LightSource **sources = getDynamicArray(0, LightSource *);
    
    pushBack(&sources, &lightSource, LightSource*);

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