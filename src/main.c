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
    Point camera = {(double)width / 2, (double)height / 2, -4000};

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

    Object rawObj[8];

    // 5. 將三角形推入 objects 陣列
    for (int i = 0; i < 8; i++) {
        rawObj[i] = newObject((Shape *)&prismTris[i], glass);
        pushBack(&objects, &rawObj[i], Object *);
    }

    Texture whitePlastic =
        newTexture(pureWhite, getPlasticRefractivity, getAirRefractivity, false, false, 1);
    // 1. 傾斜的背景接收牆 (放在三稜鏡 Z=1500 後方的 Z=1900 處)
    // 讓牆面從底部 (Z=1700) 向後傾斜到頂部 (Z=2200)
    Point wall_1 = {0, 0, 2200};                          // 左上
    Point wall_2 = {0, (double)height, 1700};             // 左下
    Point wall_3 = {(double)width, (double)height, 1700}; // 右下
    Point wall_4 = {(double)width, 0, 2200};              // 右上

    Triangle wTri1 = newTriangle(wall_1, wall_2, wall_3);
    Triangle wTri2 = newTriangle(wall_3, wall_4, wall_1);

    Object wObj1 = newObject((Shape *)&wTri1, whitePlastic);
    Object wObj2 = newObject((Shape *)&wTri2, whitePlastic);
    pushBack(&objects, &wObj1, Object *);
    pushBack(&objects, &wObj2, Object *);

    // ==================== 平行於視線的頂部狹縫擋板 ====================
    double slitWidthZ = 30.0; // Z 軸方向的狹縫寬度（控制光束前後厚度）
    double slitY = cy - 120;  // 擋板放置在三稜鏡上方

    // 1. 前擋板 (遮擋 Z < cz - slitWidthZ/2 的光)
    Point fp1 = {0, slitY, 800};
    Point fp2 = {0, slitY, cz - slitWidthZ / 2};
    Point fp3 = {(double)width, slitY, cz - slitWidthZ / 2};
    Point fp4 = {(double)width, slitY, 800};

    // 2. 後擋板 (遮擋 Z > cz + slitWidthZ/2 的光)
    Point bp1 = {0, slitY, cz + slitWidthZ / 2};
    Point bp2 = {0, slitY, 2200};
    Point bp3 = {(double)width, slitY, 2200};
    Point bp4 = {(double)width, slitY, cz + slitWidthZ / 2};

    // 建立三角形
    Triangle bTris[4];
    bTris[0] = newTriangle(fp1, fp2, fp3);
    bTris[1] = newTriangle(fp3, fp4, fp1);
    bTris[2] = newTriangle(bp1, bp2, bp3);
    bTris[3] = newTriangle(bp3, bp4, bp1);

    // 推入 objects
    Object bObjs[4];
    for (int i = 0; i < 4; i++) {
        bObjs[i] = newObject((Shape *)&bTris[i], whitePlastic);
        pushBack(&objects, &bObjs[i], Object *);
    }

    LightSource **sources = getDynamicArray(0, LightSource *);

    ParallelLightShape parallelLightShape = getParallelLightShape((Vector){0, 0.9, 0.4});
    LightSource sunlight = {(Shape *)&parallelLightShape, whiteLight};
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