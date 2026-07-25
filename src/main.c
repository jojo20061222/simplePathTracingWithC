#include "dynamicArray.h"
#include "multithreads.h"
#include "render.h"
#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

const int width = 1080;
const int height = 720;


double whiteLight(double waveLength) {
    return 1 * whiteLightBase(waveLength);
}

int main() {
    initLightIntensity();
    initTexture();

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Path Tracing", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_METAL);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                             SDL_TEXTUREACCESS_STREAMING, width, height);
    SDL_Event event;
    bool isRunning = true;

    VirtualScreen *vs = newScreen(width, height);
    Point camera = {(double)width / 2, (double)height / 2, -4000};

    Texture whitePlastic = newTexture(pureWhite, getPlasticRefractivity, getAirRefractivity, false, false, 1);
    Texture redPlastic = newTexture(red, getPlasticRefractivity, getAirRefractivity, false, false, 0);
    

    Object **objects = getDynamicArray(0, Object *);

    Point p1 = {0, 600, 900};
    Point p2 = {0, 200, 2100};
    Point p3 = {(double)width, 200, 2100};
    Point p4 = {(double)width, 600, 900};
    Triangle tri1 = newTriangle(p3, p2, p1);
    Triangle tri2 = newTriangle(p4, p3, p1);
    Object obj1 = newObject((Shape *)&tri1, whitePlastic);
    Object obj2 = newObject((Shape *)&tri2, whitePlastic);
    pushBack(&objects, &obj1, Object *);
    pushBack(&objects, &obj2, Object *);

    Sphere sph = newSphere((Point){(double)width / 2, (double)height / 2 - 75, 1500}, 100);
    Object obj3 = newObject((Shape *)&sph, getGlass());
    pushBack(&objects, &obj3, Object *);

    Sphere sph2 = newSphere((Point){(double)width / 2, (double)height / 2 - 175, 1800}, 100);
    Object obj4 = newObject((Shape *)&sph2, getGold(0.1)); 
    pushBack(&objects, &obj4, Object *);

    Sphere sph3 = newSphere((Point){(double)width / 2 +300, (double)height / 2 - 75, 1500}, 100);
    Object obj5 = newObject((Shape *)&sph3, getGold(0.3));
    pushBack(&objects, &obj5, Object *);

    Sphere lightSourceSph = newSphere((Point){(double)width / 2, -400, 1200}, 300);
    LightSource lightSource = {(Shape *)&lightSourceSph, whiteLight};

    LightSource **sources = getDynamicArray(0, LightSource *);

    pushBack(&sources, &lightSource, LightSource *);

    Multithreads *mul =
        initMultithreads(vs, 11, (const Object **)objects, getSize(objects),
                         (const LightSource **)sources, getSize(sources), &camera, 100);

    while (isRunning) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) isRunning = false;
        }

        SDL_UpdateTexture(texture, NULL, vs->buffer, width * sizeof(ScreenColor));
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(8);
    }

    destroyMultithreads(mul);

    freeDynamicArray(objects);
    freeDynamicArray(sources);
    freeScreen(vs);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}