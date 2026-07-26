#include "dynamicArray.h"
#include "multithreads.h"
#include "render.h"
#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

const int width = 1080;
const int height = 720;

double whiteLight(double waveLength) { return 50 * whiteLightBase(waveLength); }

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

    // ==================== Camera ====================

    Point camera = {(double)width * 0.5, (double)height * 0.5, -4000};

    // ==================== Objects ====================

    Texture glass = getGlass();
    Object **objects = getDynamicArray(0, Object *);

    // 三稜鏡放在畫面中央偏上
    double cx = width * 0.5;
    double cy = height * 0.36;
    double cz = 1200;

    double prismLength = 240;
    double radius = 120;

    Point fA = {cx - prismLength / 2, cy - radius, cz};
    Point fB = {cx - prismLength / 2, cy + radius * 0.5, cz - radius * 0.8660254};
    Point fC = {cx - prismLength / 2, cy + radius * 0.5, cz + radius * 0.8660254};

    Point bA = {cx + prismLength / 2, cy - radius, cz};
    Point bB = {cx + prismLength / 2, cy + radius * 0.5, cz - radius * 0.8660254};
    Point bC = {cx + prismLength / 2, cy + radius * 0.5, cz + radius * 0.8660254};

    Vector n_front = {-1, 0, 0};
    Vector n_back = {1, 0, 0};
    Vector n_AB = {0, -0.8660254, -0.5};
    Vector n_BC = {0, 1, 0};
    Vector n_CA = {0, -0.8660254, 0.5};

    DirectedTriangle prismTris[8];

    prismTris[0] = newDirectedTriangle(fA, fB, fC, n_front);
    prismTris[1] = newDirectedTriangle(bA, bC, bB, n_back);

    prismTris[2] = newDirectedTriangle(fA, bA, bB, n_AB);
    prismTris[3] = newDirectedTriangle(fA, bB, fB, n_AB);

    prismTris[4] = newDirectedTriangle(fB, bB, bC, n_BC);
    prismTris[5] = newDirectedTriangle(fB, bC, fC, n_BC);

    prismTris[6] = newDirectedTriangle(fC, bC, bA, n_CA);
    prismTris[7] = newDirectedTriangle(fC, bA, fA, n_CA);

    Object prismObjs[8];

    for (int i = 0; i < 8; i++) {
        prismObjs[i] = newObject((Shape *)&prismTris[i], glass);
        pushBack(&objects, &prismObjs[i], Object *);
    }

    // ==================== White wall ====================

    Texture whitePlastic =
        newTexture(pureWhite, getPlasticRefractivity,
                   getAirRefractivity, false, false, 1);

    double wallZ = 9000;

    Point wall1 = {0, 0, wallZ};
    Point wall2 = {0, height, wallZ};
    Point wall3 = {width, height, wallZ};
    Point wall4 = {width, 0, wallZ};

    Triangle wallTri1 = newTriangle(wall1, wall2, wall3);
    Triangle wallTri2 = newTriangle(wall3, wall4, wall1);

    Object wallObj1 = newObject((Shape *)&wallTri1, whitePlastic);
    Object wallObj2 = newObject((Shape *)&wallTri2, whitePlastic);

    pushBack(&objects, &wallObj1, Object *);
    pushBack(&objects, &wallObj2, Object *);

    // ==================== Slit ====================

    // 非常細的狹縫，只讓一小束光通過
    double slitWidth = 2.0;

    // 狹縫略高於三稜鏡頂點
    double slitY = cy - radius - 10.0;

    Point fp1 = {0, slitY, 200};
    Point fp2 = {0, slitY, cz - slitWidth * 0.5};
    Point fp3 = {width, slitY, cz - slitWidth * 0.5};
    Point fp4 = {width, slitY, 200};

    Point bp1 = {0, slitY, cz + slitWidth * 0.5};
    Point bp2 = {0, slitY, wallZ};
    Point bp3 = {width, slitY, wallZ};
    Point bp4 = {width, slitY, cz + slitWidth * 0.5};

    Triangle slitTri[4];

    slitTri[0] = newTriangle(fp1, fp2, fp3);
    slitTri[1] = newTriangle(fp3, fp4, fp1);

    slitTri[2] = newTriangle(bp1, bp2, bp3);
    slitTri[3] = newTriangle(bp3, bp4, bp1);

    Object slitObj[4];

    for (int i = 0; i < 4; i++) {
        slitObj[i] = newObject((Shape *)&slitTri[i], whitePlastic);
        pushBack(&objects, &slitObj[i], Object *);
    }

    // ==================== Light ====================

    LightSource **sources = getDynamicArray(0, LightSource *);

    // 光線完全在 YZ 平面，從上往下、往前照
    ParallelLightShape parallelLightShape =
        getParallelLightShape((Vector){0.0, 0.98, 0.20});

    LightSource sunlight = {(Shape *)&parallelLightShape, whiteLight};

    pushBack(&sources, &sunlight, LightSource *);

    // ==================== Render ====================

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