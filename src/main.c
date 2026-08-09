#include "dynamicArray.h"
#include "frame.h"
#include "multithreads.h"
#include "render.h"
#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

const int width = 720;
const int height = 540;

// 定義高強度白光，用來激發鑽石內部火彩
Real strongWhiteLight(Real waveLength) { return 6 * whiteLightBase(waveLength); }

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
    Point camera = {(Real)width / 2, (Real)height / 2, width};

    // 材質設定：準備白色反光板與黑色擋板
    // 假設系統有 pureBlack 變數，若無請替換為 RGB(0,0,0) 的對應常數
    Texture *whiteReflector =
        newTexture(pureWhite, getPlasticRefractivity, getAirRefractivity, false, false, 1);
    Texture *blackBlocker =
        newTexture(pureBlack, getPlasticRefractivity, getAirRefractivity, false, false, 1);

    Object **objects = newDynamicArray(0, Object *);
    LightSource **sources = newDynamicArray(0, LightSource *);

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
    Point showing = {(Real)width / 2, (Real)height / 2, pz + 1.5 * radius};
    /*
    Frame *diamondFrame = readFrame("src/static/diamond.obj");
    RegularTransformPara *para = newRegularTransformPara(radius, 60, 0, 0, showing);
    Shape *diamondShape = newGroupFromFrame(diamondFrame, regularTransform, para);
    destroyRegulartransformPara(para);
    Object *diamondObj = newObject(diamondShape, getDiamond());
    pushBack(&objects, diamondObj, Object *);
    */
    Shape *goldenBallSphere = newSphere(showing, radius);
    Object *goldenBallObj = newObject(goldenBallSphere, getGold(0.3));
    pushBack(&objects, goldenBallObj, Object *);

    Point *frontPoints = newDynamicArray(4, Point);
    frontPoints[0] = (Point){endx, starty, bz};
    frontPoints[1] = (Point){endx, endy, bz};
    frontPoints[2] = (Point){startx, endy, bz};
    frontPoints[3] = (Point){startx, starty, bz};
    Shape *glassPlateShape = newConvexPolygon(frontPoints);
    freeDynamicArray(frontPoints);
    Object *glassPlateObj = newObject(glassPlateShape, getGlass(0.5));
    pushBack(&objects, glassPlateObj, Object *);

    Point *frontPoints2 = newDynamicArray(4, Point);
    Real thickness = (double)width / 100;
    Real bz2 = bz - thickness;
    frontPoints2[3] = (Point){endx, starty, bz2};
    frontPoints2[2] = (Point){endx, endy, bz2};
    frontPoints2[1] = (Point){startx, endy, bz2};
    frontPoints2[0] = (Point){startx, starty, bz2};
    Shape *glassPlateShape2 = newConvexPolygon(frontPoints2);
    freeDynamicArray(frontPoints2);
    Object *glassPlateObj2 = newObject(glassPlateShape, getGlass(0.5));
    pushBack(&objects, glassPlateObj2, Object *);


    Real lightRadius = (Real)width / 40;

    Shape *lightSph1 =
        newSphere((Point){(Real)width * 0.25, (Real)height * 0.8, pz + radius}, lightRadius);
    LightSource light1 = newLightSource(lightSph1, strongWhiteLight);
    pushBack(&sources, &light1, LightSource *);

    Shape *lightSph2 =
        newSphere((Point){(Real)width * 0.75, (Real)height * 0.8, pz + 5 * radius}, lightRadius);
    LightSource light2 = newLightSource(lightSph2, strongWhiteLight);
    pushBack(&sources, &light2, LightSource *);

    Shape *lightSph3 =
        newSphere((Point){(Real)width / 2, (Real)height / 2, pz + lightRadius}, lightRadius);
    LightSource light3 = newLightSource(lightSph3, strongWhiteLight);
    pushBack(&sources, &light3, LightSource *);

    Multithreads *mul = initMultithreads(vs, 11, objects, sources, &camera, 100, 64);

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

    freeDynamicArray(objects);
    freeDynamicArray(sources);
    freeScreen(vs);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}