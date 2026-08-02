#include "multithreads.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "frame.h"

static const int threadCount = 30;

double whiteLight(double waveLength) { return 2 * whiteLightBase(waveLength); }

int main() {
    initLightIntensity();
    initTexture();

    bool isRunning = true;

    static const int width = 3024;
    static const int height = 1964;
    VirtualScreen *vs = newScreen(width, height);
    Point camera = {(double)width / 2, (double)height / 2, -4000};

    Texture *whitePlastic =
        newTexture(pureWhite, getPlasticRefractivity, getAirRefractivity, false, false, 1);
    Texture *redPlastic =
        newTexture(red, getPlasticRefractivity, getAirRefractivity, false, false, 0);

    Object **objects = newDynamicArray(0, Object *);
    LightSource **sources = newDynamicArray(0, LightSource *);

    double startx = 0, endx = width;
    double starty = 0, endy = height;
    double pz = (double)2 / (double)3 * width;
    Point *wallPoints = newDynamicArray(4, Point);
    wallPoints[0] = (Point){startx, starty, pz};
    wallPoints[1] = (Point){startx, endy, pz};
    wallPoints[2] = (Point){endx, endy, pz};
    wallPoints[3] = (Point){endx, starty, pz};
    Shape *wallShape = newConvexPolygon(wallPoints);
    freeDynamicArray(wallPoints);
    Object *wall = newObject(wallShape, whitePlastic);
    pushBack(&objects, wall, Object *);

    double bz = (double)1 / (double)3 * width;
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
    Point showing = {(double)width / 2, (double)starty + radius * 1.5, bz + radius * 1.5};

    Shape *lightSph = newSphere((Point){(double)-width, (double)height * 1.5, pz - 3 * radius}, radius);
    LightSource light = newLightSource(lightSph, whiteLight);
    pushBack(&sources, &light, LightSource *);

    Frame *diamondFrame = readFrame("src/static/diamond.obj");
    Shape *diamondShape = newGroupFromFrame(diamondFrame, 100, showing);
    Object *diamondObj = newObject(diamondShape, getDiamond());
    pushBack(&objects, diamondObj, Object *);

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
                fprintf(f, "%u\n", vs->buffer[i].pixel);
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