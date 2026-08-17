#ifndef RENDER_H
#define RENDER_H
#include "base.h"
#include "camera.h"
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

#if defined(ADAPTIVE_RENDERING)
void setThreshold(Camera *camera, VirtualScreen *vs, Object **objects, LightSource **sources,
                  int sampleThreshold);

bool isThresholdAchived(VirtualScreen *vs, int x, int y);
#endif

int getX(VirtualScreen *vs, int i);

int getY(VirtualScreen *vs, int i);

int getIndex(VirtualScreen *vs, int x, int y);

void destroyScreen(VirtualScreen *screen);

void renderPixel(Camera *camera, VirtualScreen *s, int x, int y, Object **objects,
                 LightSource **sources, MT19937 *mt, int maxDepth);

#endif