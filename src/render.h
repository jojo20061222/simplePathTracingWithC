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

Real getRelativeError(VirtualScreen *vs, int x, int y);

bool *accessConvergent(VirtualScreen *vs, int x, int y);

int getScreenWidth(VirtualScreen *vs);

int getScreenHeight(VirtualScreen *vs);

uint32_t getPixel(VirtualScreen *vs, int i);

uint32_t *getBuffer(VirtualScreen *vs);

bool *getTouchMap(Point *camera, VirtualScreen *vs, Object **objects, LightSource **sources);

void setThreshold(VirtualScreen *vs, bool *touchMap, int sampleThreshold);

bool isThresholdAchived(VirtualScreen *vs, int x, int y);

int getX(VirtualScreen *vs, int i);

int getY(VirtualScreen *vs, int i);

int getIndex(VirtualScreen *vs, int x, int y);

void freeScreen(VirtualScreen *screen);

void renderPixel(Camera *camera, VirtualScreen *s, int x, int y, Object **objects,
                 LightSource **sources, MT19937 *mt, int maxDepth);

#endif