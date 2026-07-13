#include "multithreads.h"
#include "render.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// initialize the barrier
void initBarrier(Barrier *barrier, int count) {
    barrier->currentWaitingCount = 0;
    barrier->passThreshold = count;
    pthread_mutex_init(&barrier->mtx, NULL);
    pthread_cond_init(&barrier->cond, NULL);
    barrier->generation = 0;
}

// wait until contition satisfies
void waitBarrier(Barrier *barrier) {
    pthread_mutex_lock(&barrier->mtx);
    int curGeneration = barrier->generation;
    // add the counter and check if counter reaches the threshold
    if (++barrier->currentWaitingCount >= barrier->passThreshold) {
        barrier->generation++;
        barrier->currentWaitingCount = 0;
        pthread_cond_broadcast(&barrier->cond);
    }
    // otherwise wait until being notified
    else {
        /*
         * we rely on generation of barrier to judge if we need to unlock again
         * or barrier->currentWaitingCount set by the last thread will cause counter < threshold so
         * that other threads are stuck and finally reaches deadlock
         */
        while (barrier->generation == curGeneration)
            pthread_cond_wait(&barrier->cond, &barrier->mtx);
    }
    pthread_mutex_unlock(&barrier->mtx);
}

void destroyBarrier(Barrier *barrier) {
    pthread_mutex_destroy(&barrier->mtx);
    pthread_cond_destroy(&barrier->cond);
}

typedef struct {
    int id;
    VirtualScreen *vs;
    const Object **objects;
    Barrier *start, *end;
    Point *camera;
    int workerCount;
    int *exit;
} WorkerPara;

void *worker(void *para) {
    WorkerPara *workerPara = (WorkerPara *)para;
    int id = workerPara->id;
    VirtualScreen *vs = workerPara->vs;
    const Object **objects = workerPara->objects;
    Barrier *start = workerPara->start;
    Barrier *end = workerPara->end;
    Point *camera = workerPara->camera;
    int workerCount = workerPara->workerCount;
    VirtualScreen *s = workerPara->vs;

    int rangeLength = s->width * s->height;
    int leftBoundary = (rangeLength / workerCount) * id;
    int rightBoundary = (rangeLength / workerCount) * (id + 1) - 1;
    rightBoundary = rightBoundary >= rangeLength ? rangeLength - 1 : rightBoundary;

    while (*workerPara->exit == 0) {
        waitBarrier(start);
        int objectsCount = getSize(objects);
        for (int i = leftBoundary; i <= rightBoundary; i++) {
            int x = i % s->width, y = i / s->width;
            *accessScreen(s, x, y) = (Color)0U;
            renderPixel(camera, s, x, y, objects, objectsCount);
        }
        waitBarrier(end);
    }

    free(para);

    return NULL;
}

Multithreads *initMultithreads(VirtualScreen *vs, int threadCount, const Object **objects,
                               Point *camera) {
    Multithreads *result = malloc(sizeof(Multithreads));
    pthread_t *threadPool = malloc(sizeof(pthread_t) * threadCount);
    Barrier *start = malloc(sizeof(Barrier));
    Barrier *end = malloc(sizeof(Barrier));

    initBarrier(start, threadCount + 1);
    initBarrier(end, threadCount + 1);

    result->exit = malloc(sizeof(int));
    *result->exit = 0;

    for (int i = 0; i < threadCount; i++) {
        WorkerPara *parai = malloc(sizeof(WorkerPara));
        parai->id = i;
        parai->objects = objects;
        parai->vs = vs;
        parai->start = start;
        parai->end = end;
        parai->workerCount = threadCount;
        parai->camera = camera;
        parai->exit = result->exit;
        pthread_create(&threadPool[i], NULL, worker, parai);
    }

    result->threadPool = threadPool;
    result->start = start;
    result->end = end;
    result->threadCount = threadCount;
    return result;
}

void destroyMultithreads(Multithreads *mul) {
    *mul->exit = 1;
    waitBarrier(mul->start);
    waitBarrier(mul->end);
    for (int i = 0; i < mul->threadCount; i++)
        pthread_join(mul->threadPool[i], NULL);
    destroyBarrier(mul->start);
    destroyBarrier(mul->end);
    free(mul->threadPool);
    free(mul->exit);
    free(mul);
}