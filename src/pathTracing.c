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

    Texture whitePlastic = newTexture(pureWhite, getPlasticRefractivity, getAirRefractivity, false, false, 1);
    Texture redPlastic = newTexture(red, getPlasticRefractivity, getAirRefractivity, false, false, 0);
    

    Object **objects = getDynamicArray(0, Object *);

    LightSource **sources = getDynamicArray(0, LightSource *);

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