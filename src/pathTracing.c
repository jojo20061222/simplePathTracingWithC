#include "multithreads.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const int threadCount = 30;

double whiteLight(double waveLength) { return 2 * whiteLightBase(waveLength); }

int main() {
    initLightIntensity();
    initTexture();

    bool isRunning = true;

    static const int width = 1080;
    static const int height = 720;
    VirtualScreen *vs = newScreen(width, height);
    Point camera = {(double)width / 2, (double)height / 2, -4000};

    Texture whitePlastic =
        newTexture(pureWhite, getPlasticRefractivity, getAirRefractivity, false, false, 1);
    Texture redPlastic =
        newTexture(red, getPlasticRefractivity, getAirRefractivity, false, false, 0);

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
    Point p7 = {endx, endy, pz};
    Point p8 = {endx, endy, bz};
    Shape *tri3 = newTriangle(p5, p6, p7);
    Shape *tri4 = newTriangle(p7, p8, p5);
    Object obj3 = newObject(tri3, whitePlastic);
    Object obj4 = newObject(tri4, whitePlastic);
    pushBack(&objects, &obj3, Object *);
    pushBack(&objects, &obj4, Object *);

    Shape *sph = newSphere((Point){(double)width / 2, (double)endy - 100, 1500}, 100);
    Object obj = newObject(sph, getDiamond());
    pushBack(&objects, &obj, Object *);

    Shape *lightSph = newSphere((Point){(double)-width, (double)-height, 1000}, 150);
    LightSource light = newLightSource(lightSph, whiteLight);
    pushBack(&sources, &light, LightSource *);

    Multithreads *mul = initMultithreads(vs, threadCount, objects, sources, &camera, 10);

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