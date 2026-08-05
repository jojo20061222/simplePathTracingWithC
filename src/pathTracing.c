#include "frame.h"
#include "multithreads.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const int threadCount = 30;

double strongWhiteLight(double waveLength) { return 6 * whiteLightBase(waveLength); }


int main() {
    initLightIntensity();
    initTexture();

    bool isRunning = true;

    static const int width = 720;
    static const int height = 540;
    VirtualScreen *vs = newScreen(width, height);
    Point camera = {(double)width / 2, (double)height / 2, width};

    // 材質設定：準備白色反光板與黑色擋板
    // 假設系統有 pureBlack 變數，若無請替換為 RGB(0,0,0) 的對應常數
    Texture *whiteReflector =
        newTexture(pureWhite, getPlasticRefractivity, getAirRefractivity, false, false, 1);
    Texture *blackBlocker =
        newTexture(pureBlack, getPlasticRefractivity, getAirRefractivity, false, false, 1);

    Object **objects = newDynamicArray(0, Object *);
    LightSource **sources = newDynamicArray(0, LightSource *);

    double startx = 0, endx = width;
    double starty = 0, endy = height;
    double pz = -(double)2 / (double)3 * width;
    double bz = -(double)1 / (double)3 * width;

    // 1. 底部白色反光板 (Bounce Card)
    Point *groundPoints = newDynamicArray(4, Point);
    groundPoints[0] = (Point){endx, starty, bz};
    groundPoints[1] = (Point){endx, starty, pz};
    groundPoints[2] = (Point){startx, starty, pz};
    groundPoints[3] = (Point){startx, starty, bz};
    Shape *groundShape = newConvexPolygon(groundPoints);
    freeDynamicArray(groundPoints);
    Object *ground = newObject(groundShape, whiteReflector);
    pushBack(&objects, ground, Object *);

    // 2. 左側黑色擋板 (Left Black Flag)
    Point *leftFlagPoints = newDynamicArray(4, Point);
    leftFlagPoints[0] = (Point){startx, starty, pz};
    leftFlagPoints[1] = (Point){startx, endy, pz};
    leftFlagPoints[2] = (Point){startx, endy, bz};
    leftFlagPoints[3] = (Point){startx, starty, bz};
    Shape *leftFlagShape = newConvexPolygon(leftFlagPoints);
    freeDynamicArray(leftFlagPoints);
    Object *leftFlag = newObject(leftFlagShape, blackBlocker);
    pushBack(&objects, leftFlag, Object *);

    // 3. 右側黑色擋板 (Right Black Flag)
    Point *rightFlagPoints = newDynamicArray(4, Point);
    rightFlagPoints[0] = (Point){endx, starty, bz};
    rightFlagPoints[1] = (Point){endx, endy, bz};
    rightFlagPoints[2] = (Point){endx, endy, pz};
    rightFlagPoints[3] = (Point){endx, starty, pz};
    Shape *rightFlagShape = newConvexPolygon(rightFlagPoints);
    freeDynamicArray(rightFlagPoints);
    Object *rightFlag = newObject(rightFlagShape, blackBlocker);
    pushBack(&objects, rightFlag, Object *);

    // 4. 背景黑色擋板 (Background)
    Point *backPoints = newDynamicArray(4, Point);
    backPoints[0] = (Point){endx, starty, pz};
    backPoints[1] = (Point){endx, endy, pz};
    backPoints[2] = (Point){startx, endy, pz};
    backPoints[3] = (Point){startx, starty, pz};
    Shape *backShape = newConvexPolygon(backPoints);
    freeDynamicArray(backPoints);
    Object *backWall = newObject(backShape, blackBlocker);
    pushBack(&objects, backWall, Object *);

    // 5. 載入並放置鑽石模型
    double radius = (double)width / 10;
    Point showing = {(double)width / 2, (double)height / 2, bz - radius * 1.5};

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
    Shape *lightSph1 =
        newSphere((Point){(double)width * 0.25, (double)height * 0.8, pz + radius}, lightRadius);
    LightSource light1 = newLightSource(lightSph1, strongWhiteLight);
    pushBack(&sources, &light1, LightSource *);

    // 輔助光：右上方 45 度角
    Shape *lightSph2 =
        newSphere((Point){(double)width * 0.75, (double)height * 0.8, pz + radius}, lightRadius);
    LightSource light2 = newLightSource(lightSph2, strongWhiteLight);
    pushBack(&sources, &light2, LightSource *);

    // 頂/背光 (勾邊光)：放置於鑽石正後上方，強化冠部輪廓
    Shape *lightSph3 = newSphere((Point){(double)width / 2, (double)height * 1.2, pz}, lightRadius);
    LightSource light3 = newLightSource(lightSph3, strongWhiteLight);
    pushBack(&sources, &light3, LightSource *);

    Multithreads *mul = initMultithreads(vs, threadCount, objects, sources, &camera, 100);

    while (isRunning) {
        char input[1000];
        scanf("%s", input);
        if (!strcmp(input, "p")) {
            FILE *f = fopen("outputImg", "w");
            if (f == NULL) {
                printf("open failed\n");
                continue;
            }
            fprintf(f, "%d %d\n", width, height);
            for (int i = 0; i < width * height; i++) {
                fprintf(f, "%u\n", getPixel(vs, i));
            }
            fclose(f);

            f = fopen("convergentImg", "w");
            if (f == NULL) {
                printf("open failed\n");
                continue;
            }
            fprintf(f, "%d %d\n", width, height);
            for (int i = 0; i < width * height; i++) {
                fprintf(f, "%d\n",
                        (int)*accessConvergent(vs, getX(vs, i), getY(vs, i)));
            }
            fclose(f);
        }
        else if (!strcmp(input, "q")) {
            isRunning = false;
        }
    }

    destroyMultithreads(mul);
    freeDynamicArray(objects);
    freeDynamicArray(sources);

    return 0;
}