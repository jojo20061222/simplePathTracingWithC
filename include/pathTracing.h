#ifndef PATHTRACING_H
#define PATHTRACING_H
#include "../src/camera.h"
#include "../src/lightSource.h"
#include "../src/multithreads.h"
#include "../src/object.h"
#include "../src/render.h"
#include "../src/dynamicArray.h"
#include "../src/frame.h"
#include "../src/textures.h"

void renderThroughWindow(const char *windowTitle, Object **objects, LightSource **sources,
                         Camera *camera, int width, int height, int threadCount) ;


void renderThroughPictures(Object **objects, LightSource **sources,
                         Camera *camera, int width, int height, int threadCount);

#endif