#include "multithreads.h"
#include "random.h"
#include "render.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int id;
    VirtualScreen *vs;
    Object **objects;
    LightSource **sources;
    Point *camera;
    int workerCount;
    int maxDepth;
    int *exit;
} WorkerPara;

/*
    from render.c:
    static inline int getIndex(VirtualScreen *vs, int x, int y) {
        return (vs->height - y - 1) * vs->width + x;
    }
*/

static inline int getX(int i, int width, int height) { return i % width; }

static inline int getY(int i, int width, int height) { return height - (i / width) - 1; }

static void *worker(void *para) {
    WorkerPara *workerPara = (WorkerPara *)para;
    int id = workerPara->id;
    Object **objects = workerPara->objects;
    LightSource **sources = workerPara->sources;
    Point *camera = workerPara->camera;
    int workerCount = workerPara->workerCount;
    int maxDepth = workerPara->maxDepth;
    VirtualScreen *vs = workerPara->vs;

    int width = getScreenWidth(vs);
    int height = getScreenHeight(vs);
    int rangeLength = width * height;
    int leftBoundary = (rangeLength / workerCount) * id;
    int rightBoundary = (rangeLength / workerCount) * (id + 1) - 1;
    rightBoundary = rightBoundary >= rangeLength ? rangeLength - 1 : rightBoundary;

    MT19937 *mt = initMT19937(id * 42 + time(NULL) + 19650218U);

    int blockSize = 32;
    int threshold = 5; // at least render for blockSize * threshold times
    while (*workerPara->exit == 0) {
        for (int b = 0; b < blockSize; b++) {
            for (int i = leftBoundary; i <= rightBoundary; i++) {
                // in order to reduce cache miss rate we have but to do this :(
                int x = getX(i, width, height), y = getY(i, width, height);
                if (!*accessConvergent(vs, x, y) || threshold > 0) { // adaptive sampling
                    renderPixel(camera, vs, x, y, objects, sources, mt, maxDepth);
                }
            }
        }
        threshold--;
        for (int i = leftBoundary; i <= rightBoundary; i++) {
            int x = getX(i, width, height), y = getY(i, width, height);
            if (*accessConvergent(vs, x, y)) continue;
            double RE = getRelativeError(vs, x, y);
            if (RE < 1e-3) *accessConvergent(vs, x, y) = true;
        }
    }

    destroyMT19937(mt);
    free(para);

    return NULL;
}

Multithreads *initMultithreads(VirtualScreen *vs, int threadCount, Object **objects,
                               LightSource **sources, Point *camera, int maxDepth) {
    Multithreads *result = malloc(sizeof(Multithreads));
    pthread_t *threadPool = malloc(sizeof(pthread_t) * threadCount);

    // for the reason that I want to see the whole process of rendering this is deprecated
    // Barrier *start = malloc(sizeof(Barrier));
    // Barrier *end = malloc(sizeof(Barrier));

    // initBarrier(start, threadCount + 1);
    // initBarrier(end, threadCount + 1);

    result->exit = malloc(sizeof(int));
    *result->exit = 0;

    for (int i = 0; i < threadCount; i++) {
        WorkerPara *parai = malloc(sizeof(WorkerPara));
        parai->id = i;
        parai->objects = objects;
        parai->sources = sources;
        parai->vs = vs;
        parai->workerCount = threadCount;
        parai->camera = camera;
        parai->maxDepth = maxDepth;
        parai->exit = result->exit;
        pthread_create(&threadPool[i], NULL, worker, parai);
    }

    result->threadPool = threadPool;
    result->threadCount = threadCount;
    return result;
}

void destroyMultithreads(Multithreads *mul) {
    *mul->exit = 1;
    for (int i = 0; i < mul->threadCount; i++)
        pthread_join(mul->threadPool[i], NULL);
    free(mul->threadPool);
    free(mul->exit);
    free(mul);
}