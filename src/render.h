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

typedef struct VirtualScreen VirtualScreen;

VirtualScreen *newScreen(int width, int height);

double getRelativeError(VirtualScreen *vs, int x, int y);

bool *accessConvergent(VirtualScreen *vs, int x, int y);

int getScreenWidth(VirtualScreen *vs);

int getScreenHeight(VirtualScreen *vs);

uint8_t getPixel(VirtualScreen *vs, int i);

uint8_t *getBuffer(VirtualScreen *vs);

void freeScreen(VirtualScreen *screen);

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, Object **objects,
                 LightSource **sources, MT19937 *mt, int maxDepth);

#endif