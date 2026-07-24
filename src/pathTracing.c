#include "multithreads.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const int threadCount = 30;

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

    Point p1 = {0, 600, 900};
    Point p2 = {0, 200, 2100};
    Point p3 = {(double)width, 200, 2100};
    Point p4 = {(double)width, 600, 900};
    Triangle tri1 = newTriangle(p3, p2, p1);
    Triangle tri2 = newTriangle(p4, p3, p1);
    Object obj1 = newObject((Shape *)&tri1, whitePlastic);
    Object obj2 = newObject((Shape *)&tri2, whitePlastic);
    pushBack(&objects, &obj1, Object *);
    pushBack(&objects, &obj2, Object *);

    Sphere sph = newSphere((Point){(double)width / 2, (double)height / 2 - 75, 1500}, 100);
    Object obj3 = newObject((Shape *)&sph, getGold(0.3));
    pushBack(&objects, &obj3, Object *);

    Sphere sph2 = newSphere((Point){(double)width / 2 - 300, (double)height / 2 - 75, 1500}, 100);
    Object obj4 = newObject((Shape *)&sph2, getGold(0.1));
    pushBack(&objects, &obj4, Object *);

    Sphere sph3 = newSphere((Point){(double)width / 2 + 300, (double)height / 2 - 75, 1500}, 100);
    Object obj5 = newObject((Shape *)&sph3, getGold(0.5));
    pushBack(&objects, &obj5, Object *);

    Sphere lightSourceSph = newSphere((Point){(double)width / 2, -400, 1200}, 150);
    LightSource lightSource = {(Shape *)&lightSourceSph, whiteLightBase};

    LightSource **sources = getDynamicArray(0, LightSource *);

    pushBack(&sources, &lightSource, LightSource *);

    Multithreads *mul =
        initMultithreads(vs, threadCount, (const Object **)objects, getSize(objects),
                         (const LightSource **)sources, getSize(sources), &camera, 10);

    while (isRunning) {
        char input[1000];
        scanf("%s", input);
        if (!strcmp(input, "p")) {
            FILE *f = fopen("outputImg", "w");
            if (f == NULL) {
                printf("open failed\n");
                continue;
            }
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