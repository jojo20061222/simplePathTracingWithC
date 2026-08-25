#include "../include/pathTracing.h"
#if defined(__APPLE__)
#include <SDL.h>
#elif defined(__linux__)
#include <SDL2/SDL.h>
#endif

#include <string.h>
/*
 * @author seaofvoices
 * @brief This file provides two default method for the users to render.
 */

static const int maxDepth = 32;
static const int threshold = 64;

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
        }
        else if (!strcmp(input, "q")) {
            isRunning = false;
        }
    }

    destroyMultithreads(mul);
    destroyScreen(vs);
}