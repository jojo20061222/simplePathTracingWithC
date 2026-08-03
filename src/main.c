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
double strongWhiteLight(double waveLength) { return 6.0 * whiteLightBase(waveLength); }

int main() {
    initLightIntensity();
    initTexture();

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Path Tracing - Diamond Studio Setup", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_METAL);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                             SDL_TEXTUREACCESS_STREAMING, width, height);
    SDL_Event event;
    bool isRunning = true;

    VirtualScreen *vs = newScreen(width, height);
    Point camera = {(double)width / 2, (double)height / 2, width};

    // 材質設定：準備白色反光板與黑色擋板
    // 假設系統有 pureBlack 變數，若無請替換為 RGB(0,0,0) 的對應常數
    Texture *whiteReflector = newTexture(pureWhite, getPlasticRefractivity, getAirRefractivity, false, false, 1);
    Texture *blackBlocker = newTexture(pureBlack, getPlasticRefractivity, getAirRefractivity, false, false, 1);

    Object **objects = newDynamicArray(0, Object *);
    LightSource **sources = newDynamicArray(0, LightSource *);

    double startx = 0, endx = width;
    double starty = 0, endy = height;
    double pz = -(double)2 / (double)3 * width;
    double bz = -(double)1 / (double)3 * width;

    // 1. 底部白色反光板 (Bounce Card)：將光線反射進鑽石底部
    Point *groundPoints = newDynamicArray(4, Point);
    groundPoints[0] = (Point){startx, starty, bz};
    groundPoints[1] = (Point){startx, starty, pz};
    groundPoints[2] = (Point){endx, starty, pz};
    groundPoints[3] = (Point){endx, starty, bz};
    Shape *groundShape = newConvexPolygon(groundPoints);
    freeDynamicArray(groundPoints);
    Object *ground = newObject(groundShape, whiteReflector);
    pushBack(&objects, ground, Object *);

    // 2. 左側黑色擋板 (Left Black Flag)：製造鑽石左側切面的深色對比
    Point *leftFlagPoints = newDynamicArray(4, Point);
    leftFlagPoints[0] = (Point){startx, starty, bz};
    leftFlagPoints[1] = (Point){startx, endy, bz};
    leftFlagPoints[2] = (Point){startx, endy, pz};
    leftFlagPoints[3] = (Point){startx, starty, pz};
    Shape *leftFlagShape = newConvexPolygon(leftFlagPoints);
    freeDynamicArray(leftFlagPoints);
    Object *leftFlag = newObject(leftFlagShape, blackBlocker);
    pushBack(&objects, leftFlag, Object *);

    // 3. 右側黑色擋板 (Right Black Flag)：製造鑽石右側切面的深色對比
    Point *rightFlagPoints = newDynamicArray(4, Point);
    rightFlagPoints[0] = (Point){endx, starty, pz};
    rightFlagPoints[1] = (Point){endx, endy, pz};
    rightFlagPoints[2] = (Point){endx, endy, bz};
    rightFlagPoints[3] = (Point){endx, starty, bz};
    Shape *rightFlagShape = newConvexPolygon(rightFlagPoints);
    freeDynamicArray(rightFlagPoints);
    Object *rightFlag = newObject(rightFlagShape, blackBlocker);
    pushBack(&objects, rightFlag, Object *);

    // 4. 背景黑色擋板 (Background)：讓鑽石成為絕對視覺焦點
    Point *backPoints = newDynamicArray(4, Point);
    backPoints[0] = (Point){startx, starty, pz};
    backPoints[1] = (Point){startx, endy, pz};
    backPoints[2] = (Point){endx, endy, pz};
    backPoints[3] = (Point){endx, starty, pz};
    Shape *backShape = newConvexPolygon(backPoints);
    freeDynamicArray(backPoints);
    Object *backWall = newObject(backShape, blackBlocker);
    pushBack(&objects, backWall, Object *);

    // 5. 載入並放置鑽石模型
    double radius = (double)width / 10;
    Point showing = {(double)width / 2, (double)starty + radius * 1, bz - radius * 1.5};
    
    Frame *diamondFrame = readFrame("src/static/diamond.obj");
    RegularTransformPara *para = newRegularTransformPara(radius, 90, 0, 0, showing);
    Shape *diamondShape = newGroupFromFrame(diamondFrame, regularTransform, para);
    destroyRegulartransformPara(para);
    Object *diamondObj = newObject(diamondShape, getDiamond());
    pushBack(&objects, diamondObj, Object *);

    // 6. 佈置小面積點光源 (Spotlights)
    // 將半徑縮小至 width / 40，產生銳利的高光與火彩
    double lightRadius = (double)width / 40; 
    
    // 主光：左上方 45 度角打向鑽石
    Shape *lightSph1 = newSphere((Point){(double)width * 0.25, (double)height * 0.8, pz + radius}, lightRadius);
    LightSource light1 = newLightSource(lightSph1, strongWhiteLight);
    pushBack(&sources, &light1, LightSource *);

    // 輔助光：右上方 45 度角
    Shape *lightSph2 = newSphere((Point){(double)width * 0.75, (double)height * 0.8, pz + radius}, lightRadius);
    LightSource light2 = newLightSource(lightSph2, strongWhiteLight);
    pushBack(&sources, &light2, LightSource *);

    // 頂/背光 (勾邊光)：放置於鑽石正後上方，強化冠部輪廓
    Shape *lightSph3 = newSphere((Point){(double)width / 2, (double)height * 1.2, pz}, lightRadius);
    LightSource light3 = newLightSource(lightSph3, strongWhiteLight);
    pushBack(&sources, &light3, LightSource *);

    // 執行渲染
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