#include "dynamicArray.h"
#include "frame.h"
#include "multithreads.h"
#include "render.h"
#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

const int width = 720;
const int height = 540;

double whiteLight(double waveLength) { return 3 * whiteLightBase(waveLength); }

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
    Point camera = {(double)width / 2, (double)height / 2, width};

    Texture *whitePlastic =
        newTexture(pureWhite, getPlasticRefractivity, getAirRefractivity, false, false, 1);
    Texture *redPlastic =
        newTexture(red, getPlasticRefractivity, getAirRefractivity, false, false, 1);

    Object **objects = newDynamicArray(0, Object *);
    LightSource **sources = newDynamicArray(0, LightSource *);

    double startx = 0, endx = width;
    double starty = 0, endy = height;
    double pz = -(double)2 / (double)3 * width;
    Point *wallPoints = newDynamicArray(4, Point);
    wallPoints[0] = (Point){startx, starty, pz};
    wallPoints[1] = (Point){startx, endy, pz};
    wallPoints[2] = (Point){endx, endy, pz};
    wallPoints[3] = (Point){endx, starty, pz};
    Shape *wallShape = newConvexPolygon(wallPoints);
    freeDynamicArray(wallPoints);
    Object *wall = newObject(wallShape, whitePlastic);
    pushBack(&objects, wall, Object *);

    double bz = -(double)1 / (double)3 * width;
    Point *groundPoints = newDynamicArray(4, Point);
    groundPoints[0] = (Point){startx, starty, bz};
    groundPoints[1] = (Point){startx, starty, pz};
    groundPoints[2] = (Point){endx, starty, pz};
    groundPoints[3] = (Point){endx, starty, bz};
    Shape *groundShape = newConvexPolygon(groundPoints);
    freeDynamicArray(groundPoints);
    Object *ground = newObject(groundShape, whitePlastic);
    pushBack(&objects, ground, Object *);

    double radius = (double)width / 10;
    Point showing = {(double)width / 2, (double)starty + radius * 1, bz - radius * 1.5};

    Shape *lightSph = newSphere((Point){(double)-width, (double)height * 1.5, -pz + radius * 3}, radius);
    LightSource light = newLightSource(lightSph, whiteLight);
    pushBack(&sources, &light, LightSource *);

    Frame *diamondFrame = readFrame("src/static/diamond.obj");
    Shape *diamondShape = newGroupFromFrame(diamondFrame, radius, showing);
    Object *diamondObj = newObject(diamondShape, getDiamond());
    pushBack(&objects, diamondObj, Object *);

    Multithreads *mul = initMultithreads(vs, 11, objects, sources, &camera, 100);

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