#ifndef MULTITHREADS_H
#define MULTITHREADS_H
#include "render.h"
#include <pthread.h>

typedef struct Multithreads Multithreads;

Multithreads *initMultithreads(VirtualScreen *vs, int threadCount, Shape **shapes,
                               Texture **textures, Object **objects, LightSource **sources,
                               Point *camera, int maxDepth, int sampleThreshold);

void destroyMultithreads(Multithreads *mul);

#endif