#ifndef MULTITHREADS_H
#define MULTITHREADS_H
#include "render.h"
#include <pthread.h>

typedef struct {
    pthread_t *threadPool;
    int threadCount;
    int *exit;
} Multithreads;

Multithreads *initMultithreads(VirtualScreen *vs, int threadCount, Object **objects,
                               LightSource **sources, Point *camera, int maxDepth);

void destroyMultithreads(Multithreads *mul);

#endif