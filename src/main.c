#include "dynamicArray.h"
#include "multithreads.h"
#include "render.h"
#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

const int width = 1080;
const int height = 720;

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
        newTexture(red, getPlasticRefractivity, getAirRefractivity, false, false, 0);
    Texture glass = getGlass();

    Object **objects = getDynamicArray(0, Object *);

    // 1. 定義三稜鏡中心與尺寸
    double cx = (double)width / 2;
    double cy = (double)height / 2;
    double cz = 1500;

    double prismLength = 300; // 稜鏡長度 (沿 X 軸)
    double radius = 150;      // 截面外接圓半徑

    // 2. 計算 6 個頂點 (Y 軸朝下為 positive 的常見座標系)
    Point fA = {cx - prismLength / 2, cy - radius, cz};
    Point fB = {cx - prismLength / 2, cy + radius * 0.5, cz - radius * 0.866};
    Point fC = {cx - prismLength / 2, cy + radius * 0.5, cz + radius * 0.866};

    Point bA = {cx + prismLength / 2, cy - radius, cz};
    Point bB = {cx + prismLength / 2, cy + radius * 0.5, cz - radius * 0.866};
    Point bC = {cx + prismLength / 2, cy + radius * 0.5, cz + radius * 0.866};

    // 3. 定義 5 個面的外法向量 (Outer Normal Vectors)
    Vector n_front = {-1, 0, 0};
    Vector n_back = {1, 0, 0};
    Vector n_AB = {0, -0.866, -0.5}; // 面 AB 的外法向
    Vector n_BC = {0, 1, 0};         // 底面 BC 的外法向 (朝下)
    Vector n_CA = {0, -0.866, 0.5};  // 面 CA 的外法向

    // 4. 建立 8 個自帶法向量的三角形
    DirectedTriangle prismTris[8];

    // 前後底面
    prismTris[0] = newDirectedTriangle(fA, fB, fC, n_front);
    prismTris[1] = newDirectedTriangle(bA, bC, bB, n_back);

    // 側面 1 (AB)
    prismTris[2] = newDirectedTriangle(fA, bA, bB, n_AB);
    prismTris[3] = newDirectedTriangle(fA, bB, fB, n_AB);

    // 側面 2 (BC)
    prismTris[4] = newDirectedTriangle(fB, bB, bC, n_BC);
    prismTris[5] = newDirectedTriangle(fB, bC, fC, n_BC);

    // 側面 3 (CA)
    prismTris[6] = newDirectedTriangle(fC, bC, bA, n_CA);
    prismTris[7] = newDirectedTriangle(fC, bA, fA, n_CA);

    // 5. 將三角形推入 objects 陣列
    for (int i = 0; i < 8; i++) {
        Object prismObj = newObject((Shape *)&prismTris[i], glass);
        pushBack(&objects, &prismObj, Object *);
    }
    LightSource **sources = getDynamicArray(0, LightSource *);

    ParallelLightShape parallelLightShape = getParallelLightShape((Vector){0, 1, 0});
    LightSource sunlight = {(Shape *)&parallelLightShape, sunlightBase};
    pushBack(&sources, &sunlight, LightSource *);

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