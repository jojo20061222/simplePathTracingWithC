#include "../include/pathTracing.h"
#ifndef NO_SDL
#if defined(__APPLE__)
#include <SDL.h>
#elif defined(__linux__)
#include <SDL2/SDL.h>
#endif
#endif

#include <string.h>
#include <time.h>
/*
 * @author seaofvoices
 * @brief This file provides two default method for the users to render.
 */

static const int maxDepth = 32;

#ifndef NO_SDL

void renderToWindow(const char *windowTitle, Object **objects, LightSource **sources,
                    Camera *camera, int width, int height, int threadCount) {
    initLightIntensity();
    initTexture();

    SDL_Init(SDL_INIT_VIDEO);
    VirtualScreen *vs = newScreen(width, height);

#if defined(__APPLE__)
    Uint32 windowFlags = SDL_WINDOW_METAL;
#elif defined(__linux__)
    Uint32 windowFlags = SDL_WINDOW_OPENGL;
#endif

    SDL_Window *window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, width, height, windowFlags);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                             SDL_TEXTUREACCESS_STREAMING, width, height);
    SDL_Event event;
    bool isRunning = true;
    Multithreads *mul =
        initMultithreads(vs, threadCount, objects, sources, camera, maxDepth);

    while (isRunning) {
        while (SDL_PollEvent(&event) != 0) {
            // if receive a event to quit then stop running
            if (event.type == SDL_QUIT) isRunning = false;
        }

        SDL_UpdateTexture(texture, NULL, getBuffer(vs), width * sizeof(ScreenColor));
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(8);
    }

    destroyMultithreads(mul);
    destroyScreen(vs);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

#endif // NO_SDL

void renderToPictures(const char *outputPath, Object **objects, LightSource **sources,
                      Camera *camera, int width, int height, int threadCount) {
    initLightIntensity();
    initTexture();

    VirtualScreen *vs = newScreen(width, height);

    bool isRunning = true;
    Multithreads *mul =
        initMultithreads(vs, threadCount, objects, sources, camera, maxDepth);

    while (isRunning) {
        char input[1000];
        scanf("%s", input);
        if (!strcmp(input, "p")) {
            FILE *f = fopen(outputPath, "w");
            if (f == NULL) {
                printf("open failed\n");
                continue;
            }
            fprintf(f, "%d %d\n", width, height);
            for (int i = 0; i < width * height; i++) {
                fprintf(f, "%u\n", getPixel(vs, i));
            }
            fclose(f);
            printf("Output %s finished\n", outputPath);
        }
        else if (!strcmp(input, "q")) {
            isRunning = false;
        }
    }

    destroyMultithreads(mul);
    destroyScreen(vs);
}

// Headless fixed-workload benchmark. Termination condition: every pixel gets exactly `spp`
// samples, each path is bounded by maxDepth (+ Russian roulette, unchanged). Only the render
// itself is timed. Result line is machine-readable: BENCH key=value ...
void renderBenchmark(const char *outputPath, Object **objects, LightSource **sources,
                     Camera *camera, int width, int height, int threadCount, int spp,
                     unsigned int seed, const char *xyzPath) {
    initLightIntensity();
    initTexture();
    VirtualScreen *vs = newScreen(width, height);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    runFixedWorkload(vs, threadCount, objects, sources, camera, maxDepth, spp, seed);
    clock_gettime(CLOCK_MONOTONIC, &t1);

    double sec = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    double samples = (double)width * height * spp;
    printf("BENCH width=%d height=%d spp=%d maxDepth=%d threads=%d seed=%u time_s=%.6f "
           "Msamples_per_s=%.6f\n",
           width, height, spp, maxDepth, threadCount, seed, sec, samples / sec / 1e6);

    if (outputPath != NULL) {
        FILE *f = fopen(outputPath, "w");
        if (f == NULL) {
            printf("open %s failed\n", outputPath);
        } else {
            fprintf(f, "%d %d\n", width, height);
            for (int i = 0; i < width * height; i++) fprintf(f, "%u\n", getPixel(vs, i));
            fclose(f);
        }
    }

    // linear XYZ mean as raw binary: int32 width, int32 height, then width*height*3 float64
    // (X, Y, Z per pixel, same pixel order as the image file). Used for error measurements.
    if (xyzPath != NULL) {
        FILE *f = fopen(xyzPath, "wb");
        if (f == NULL) {
            printf("open %s failed\n", xyzPath);
        } else {
            int32_t wh[2] = {width, height};
            fwrite(wh, sizeof(int32_t), 2, f);
            for (int i = 0; i < width * height; i++) {
                ColorXYZ c = getMeanXYZ(vs, i);
                double xyz[3] = {c.X, c.Y, c.Z};
                fwrite(xyz, sizeof(double), 3, f);
            }
            fclose(f);
        }
    }
    destroyScreen(vs);
}
