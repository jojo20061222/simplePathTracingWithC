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
    ColorXYZ *sum;
    int width, height;
} VirtualScreen;

static inline ScreenColor *accessScreen(VirtualScreen *screen, int x, int y) {
    return &screen->buffer[y * screen->width + x];
}

static inline ColorXYZ *accessSum(VirtualScreen *vs, int x, int y) {
    return &vs->sum[y * vs->width + x];
}

VirtualScreen *newScreen(int width, int height);

void freeScreen(VirtualScreen *screen);

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, const Object **objects,
                 int objectsCount, const LightSource **sources, int renderCount, MT19937 *mt,
                 int maxDepth);

#endif