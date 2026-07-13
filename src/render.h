#ifndef RENDER_H
#define RENDER_H
#include "base.h"
#include "dynamicArray.h"
#include "object.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    ScreenColor *buffer;
    int width, height;
} VirtualScreen;

ScreenColor *accessScreen(VirtualScreen *screen, int x, int y);

VirtualScreen *newScreen(int width, int height);

void freeScreen(VirtualScreen *screen);

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, const Object **objects,
                 int objectsCount);

void renderScreen(Point *camera, VirtualScreen *s, const Object **objects, int objectsCount);

#endif