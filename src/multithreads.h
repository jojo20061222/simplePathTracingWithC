#ifndef MULTITHREADS_H
#define MULTITHREADS_H
#include "render.h"
#include <pthread.h>

// since the POSIX standard supported by macOS is too old I have to implement pthread_barrier_t
// myself, eww
typedef struct {
    int currentWaitingCount;
    int passThreshold;
    unsigned int generation; // record the barrier has been used for `generation` times
    pthread_mutex_t mtx;
    pthread_cond_t cond;
} Barrier;

void initBarrier(Barrier *barrier, int count);

void waitBarrier(Barrier *barrier);

void destroyBarrier(Barrier *barrier);

typedef struct {
    pthread_t *threadPool;
    Barrier *start, *end;
    int threadCount;
    int *exit;
} Multithreads;

Multithreads *initMultithreads(VirtualScreen *vs, int threadCount, const Object **objects,
                               int objectsCount, Point *camera);

void destroyMultithreads(Multithreads *mul);

#endif