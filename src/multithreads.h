#ifndef MULTITHREADS_H
#define MULTITHREADS_H
#include "render.h"
#include <pthread.h>

typedef struct Multithreads Multithreads;

Multithreads *initMultithreads(VirtualScreen *vs, int threadCount, Object **objects,
                               LightSource **sources, Camera *camera, int maxDepth);

void destroyMultithreads(Multithreads *mul);

// Render every pixel exactly `spp` times and return (blocking). Deterministic for a given seed,
// independent of threadCount. Used for benchmarking.
void runFixedWorkload(VirtualScreen *vs, int threadCount, Object **objects, LightSource **sources,
                      Camera *camera, int maxDepth, int spp, uint32_t seed);

#endif