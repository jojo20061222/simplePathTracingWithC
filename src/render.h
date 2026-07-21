#ifndef RENDER_H
#define RENDER_H
#include "base.h"
#include "dynamicArray.h"
#include "lightSource.h"
#include "object.h"
#include "random.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    ScreenColor *buffer;
    unsigned int *renderCount;
    ColorXYZ *mean;
    double *m2;
    bool *converge;
    int width, height;
} VirtualScreen;

VirtualScreen *newScreen(int width, int height);

void freeScreen(VirtualScreen *screen);

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, const Object **objects,
                 int objectsCount, const LightSource **sources, int sourcesCount, MT19937 *mt,
                 int maxDepth);


static inline double getRelativeError(VirtualScreen *vs, int x, int y) {
    // for convenient I uses pop std error
    int i = y * vs->width + x;
    double SE = sqrt(vs->m2[i] / vs->renderCount[i]);
    return SE / vs->mean[i].Y;
}

static inline bool *accessConverge(VirtualScreen *vs, int x, int y) {
    return &vs->converge[y * vs->width + x];
}

#endif