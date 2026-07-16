#include "multithreads.h"
#include "random.h"
#include "render.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int id;
    VirtualScreen *vs;
    const Object **objects;
    int objectsCount;
    const LightSource **sources;
    int sourcesCount;
    Point *camera;
    int workerCount;
    int maxDepth;
    int *exit;
} WorkerPara;

void *worker(void *para) {
    WorkerPara *workerPara = (WorkerPara *)para;
    int id = workerPara->id;
    const Object **objects = workerPara->objects;
    int objectsCount = workerPara->objectsCount;
    const LightSource **sources = workerPara->sources;
    int sourcesCount = workerPara->sourcesCount;
    Point *camera = workerPara->camera;
    int workerCount = workerPara->workerCount;
    int maxDepth = workerPara->maxDepth;
    VirtualScreen *s = workerPara->vs;

    int rangeLength = s->width * s->height;
    int leftBoundary = (rangeLength / workerCount) * id;
    int rightBoundary = (rangeLength / workerCount) * (id + 1) - 1;
    rightBoundary = rightBoundary >= rangeLength ? rangeLength - 1 : rightBoundary;


    int renderCount = 0;
    MT19937 *mt = initMT19937(id * 42 + time(NULL) + 19650218U);

    while (*workerPara->exit == 0) {
        renderCount++;
        for (int i = leftBoundary; i <= rightBoundary; i++) {
            int x = i % s->width, y = i / s->width;
            // *accessScreen(s, x, y) = (ScreenColor)0U;
            renderPixel(camera, s, x, y, objects, objectsCount, sources, sourcesCount, renderCount, mt, maxDepth);
        }
        if (renderCount == 30000) printf("hello from worker %d\n", id);
    }

    destroyMT19937(mt);
    free(para);

    return NULL;
}

Multithreads *initMultithreads(VirtualScreen *vs, int threadCount, const Object **objects,
                               int objectsCount, const LightSource **sources, int sourcesCount, Point *camera, int maxDepth) {
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
        parai->objectsCount = objectsCount;
        parai->sources = sources;
        parai->sourcesCount = sourcesCount;
        parai->vs = vs;
        // parai->start = start;
        // parai->end = end;
        parai->workerCount = threadCount;
        parai->camera = camera;
        parai->maxDepth = maxDepth;
        parai->exit = result->exit;
        pthread_create(&threadPool[i], NULL, worker, parai);
    }

    result->threadPool = threadPool;
    // result->start = start;
    // result->end = end;
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