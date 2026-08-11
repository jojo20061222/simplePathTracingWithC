#include "camera.h"
#include "dynamicArray.h"
#include "frame.h"
#include "multithreads.h"
#include "render.h"
#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

static const int width = 720;
static const int height = 540;

// 定義高強度白光，用來激發鑽石內部火彩
Real strongWhiteLight(Real wavelength) { return 6 * whiteLightBase(wavelength); }

int main() {
    initLightIntensity();
    initTexture();

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window =
        SDL_CreateWindow("Path Tracing - Diamond Studio Setup", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_METAL);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                             SDL_TEXTUREACCESS_STREAMING, width, height);
    SDL_Event event;
    bool isRunning = true;
    VirtualScreen *vs = newScreen(width, height);
    Camera *camera =
        newRegularCamera(width, height, (Real)width * 0.35, (Real)width * 0.65, (Real)height * 0.35,
                         (Real)height * 0.65, (double)width * (-1.0 / 3.0), width);
    Frame **frames = newDynamicArray(0, Frame *);
    Shape **shapes = newDynamicArray(0, Shape *);
    Texture **textures = newDynamicArray(0, Texture *);
    Object **objects = newDynamicArray(0, Object *);
    LightSource **sources = newDynamicArray(0, LightSource *);

    Texture *whiteReflector =
        newOpaqueDieletric(getPlasticRefractivity, getAirRefractivity, 1, pureWhite);
    pushBack(&textures, whiteReflector, Texture *);

    Real startx = 0, endx = width;
    Real starty = (Real)height / 3, endy = height;
    Real pz = -(Real)2 / (Real)3 * width;
    Real bz = -(Real)1 / (Real)3 * width;

    // groud
    Point *groundPoints = newDynamicArray(4, Point);
    groundPoints[0] = (Point){endx, starty, bz};
    groundPoints[1] = (Point){endx, starty, pz};
    groundPoints[2] = (Point){startx, starty, pz};
    groundPoints[3] = (Point){startx, starty, bz};
    Shape *groundShape = newConvexPolygon(groundPoints);
    pushBack(&shapes, groundShape, Shape *);
    freeDynamicArray(groundPoints);
    Object *ground = newObject(groundShape, whiteReflector);
    pushBack(&objects, ground, Object *);

    // back
    Point *backPoints = newDynamicArray(4, Point);
    backPoints[0] = (Point){endx, starty, pz};
    backPoints[1] = (Point){endx, endy, pz};
    backPoints[2] = (Point){startx, endy, pz};
    backPoints[3] = (Point){startx, starty, pz};
    Shape *backShape = newConvexPolygon(backPoints);
    freeDynamicArray(backPoints);
    Object *backWall = newObject(backShape, whiteReflector);
    pushBack(&objects, backWall, Object *);

    // diamond

    Real radius = (Real)width / 10;
    Point showing = {(Real)width / 2, (Real)height / 2, pz + 2 * radius};

    Frame *diamondFrame = newFrame("src/static/diamond.obj");
    pushBack(&frames, diamondFrame, Frame *);
    RegularTransformPara *para = newRegularTransformPara(radius, 60, 0, 0, showing);
    Shape *diamondShape = newGroupFromFrame(diamondFrame, regularTransform, para);
    pushBack(&shapes, diamondShape, Shape *);
    destroyRegulartransformPara(para);

    Object *diamondObj = newObject(diamondShape, getDiamond(0));
    pushBack(&objects, diamondObj, Object *);

    Real lightRadius = (Real)width / 40;

    Shape *lightSph1 =
        newSphere((Point){(Real)width * 0.25, (Real)height * 0.8, pz + radius}, lightRadius);
    pushBack(&shapes, lightSph1, Shape *);

    LightSource light1 = newLightSource(lightSph1, strongWhiteLight);
    pushBack(&sources, &light1, LightSource *);

    Shape *lightSph2 =
        newSphere((Point){(Real)width * 0.75, (Real)height * 0.8, pz + 5 * radius}, lightRadius);
    pushBack(&shapes, lightSph2, Shape *);

    LightSource light2 = newLightSource(lightSph2, strongWhiteLight);
    pushBack(&sources, &light2, LightSource *);

    Shape *lightSph3 =
        newSphere((Point){(Real)width / 2, (Real)height / 2, pz + lightRadius}, lightRadius);
    pushBack(&shapes, lightSph3, Shape *);

    LightSource light3 = newLightSource(lightSph3, strongWhiteLight);
    pushBack(&sources, &light3, LightSource *);
    Multithreads *mul = initMultithreads(vs, 11, objects, sources, camera, 100, 64);

    while (isRunning) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) isRunning = false;
        }

        SDL_UpdateTexture(texture, NULL, getBuffer(vs), width * sizeof(ScreenColor));
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(8);
    }

    destroyMultithreads(mul);
    destroyRegularCamera(camera);

    for (int i = 0; i < getSize(frames); i++) {
        destroyFrame(frames[i]);
    }

    for (int i = 0; i < getSize(sources); i++) {
        destroyShape(shapes[i]);
    }
    freeDynamicArray(shapes);

    for (int i = 0; i < getSize(textures); i++) {
        destroyTexture(textures[i]);
    }
    freeDynamicArray(textures);

    for (int i = 0; i < getSize(objects); i++) {
        destroyObject(objects[i]);
    }
    freeDynamicArray(objects);

    freeScreen(vs);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}