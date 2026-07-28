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
    int width, height;
} VirtualScreen;

VirtualScreen *newScreen(int width, int height);

void freeScreen(VirtualScreen *screen);

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, Object **objects,
                 int objectsCount, LightSource **sources, int sourcesCount, MT19937 *mt,
                 int maxDepth);

#endif