#include "../include/pathTracing.h"
#include <SDL.h>

void renderThroughWindow(const char *windowTitle, Object **objects, LightSource **sources,
                         Camera *camera, int width, int height, int threadCount) {
    initLightIntensity();
    initTexture();

    SDL_Init(SDL_INIT_VIDEO);
    VirtualScreen *vs = newScreen(width, height);

    SDL_Window *window =
        SDL_CreateWindow("Path Tracing - Diamond Studio Setup", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_METAL);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                             SDL_TEXTUREACCESS_STREAMING, width, height);
    SDL_Event event;
    bool isRunning = true;
    Multithreads *mul = initMultithreads(vs, 11, objects, sources, camera, 100, 64);

    while (isRunning) {
        while (SDL_PollEvent(&event) != 0) {
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

void renderThroughPictures(Object **objects, LightSource **sources,
                         Camera *camera, int width, int height, int threadCount) {
    initLightIntensity();
    initTexture();

    VirtualScreen *vs = newScreen(width, height);

    bool isRunning = true;
    Multithreads *mul = initMultithreads(vs, 11, objects, sources, camera, 100, 64);

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
        }
        else if (!strcmp(input, "q")) {
            isRunning = false;
        }
    }

    destroyMultithreads(mul);
    destroyScreen(vs);
}