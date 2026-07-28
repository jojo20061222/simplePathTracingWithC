#include "dynamicArray.h"
#include "multithreads.h"
#include "render.h"
#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

const int width = 1080;
const int height = 720;

double whiteLight(double waveLength) { return 2 * whiteLightBase(waveLength); }

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

    Texture whitePlastic =
        newTexture(pureWhite, getPlasticRefractivity, getAirRefractivity, false, false, 1);
    Texture redPlastic =
        newTexture(red, getPlasticRefractivity, getAirRefractivity, false, false, 1);

    Object **objects = getDynamicArray(0, Object *);
    LightSource **sources = getDynamicArray(0, LightSource *);

    double startx = 0, endx = width;
    double starty = 0, endy = height; 
    double pz = 1800;
    Point p1 = {startx, starty, pz};
    Point p2 = {startx, endy, pz};
    Point p3 = {endx, endy, pz};
    Point p4 = {endx, starty, pz};
    Shape *tri1 = newTriangle(p1, p2, p3);
    Shape *tri2 = newTriangle(p3, p4, p1);
    Object obj1 = newObject(tri1, whitePlastic);
    Object obj2 = newObject(tri2, whitePlastic);
    pushBack(&objects, &obj1, Object *);
    pushBack(&objects, &obj2, Object *);


    double bz = 600;
    Point p5 = {startx, endy, bz};
    Point p6 = {startx, endy, pz};
    Point p7 = {endx , endy, pz};
    Point p8 = {endx , endy, bz};
    Shape *tri3 = newTriangle(p5, p6, p7);
    Shape *tri4 = newTriangle(p7, p8, p5);
    Object obj3 = newObject(tri3, whitePlastic);
    Object obj4 = newObject(tri4, whitePlastic);
    pushBack(&objects, &obj3, Object*);
    pushBack(&objects, &obj4, Object*);

    Shape *sph = newSphere((Point){(double)width / 2, (double)endy - 100, 1500}, 100);
    Object obj = newObject(sph, getDiamond());
    pushBack(&objects, &obj, Object *);

    Shape *lightSph = newSphere((Point){(double)-width, (double)-height, 1000}, 150);
    LightSource light = newLightSource(lightSph, whiteLight);
    pushBack(&sources, &light, LightSource *);

    Multithreads *mul = initMultithreads(vs, 11, objects, getSize(objects), sources,
                                         getSize(sources), &camera, 100);

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