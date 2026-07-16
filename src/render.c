#include "render.h"
#include "base.h"
#include "lightSource.h"
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

VirtualScreen *newScreen(int width, int height) {
    VirtualScreen *result = malloc(sizeof(VirtualScreen));
    result->width = width;
    result->height = height;
    result->buffer = calloc(width * height, sizeof(ScreenColor *));
    return result;
}

void freeScreen(VirtualScreen *screen) {
    free(screen->buffer);
    free(screen);
    return;
}

typedef struct {
    CollideColorResult finalcollideResult;
    const Object *finalcollideObj;
} RayCollideWithObjectsResult;

RayCollideWithObjectsResult rayCollideWithObjects(ReversedRay initRay, const Object **objects,
                                                  int objectsCount) {
    double minT = INFINITY;
    const Object *finalcollideObj = NULL;
    CollideColorResult finalcollideResult;
    for (int i = 0; i < objectsCount; i++) {
        CollideColorResult collideResult = collideObject(objects[i], &initRay);
        if (!collideResult.collide) continue;
        if (collideResult.t < minT) {
            finalcollideObj = objects[i];
            minT = collideResult.t;
            finalcollideResult = collideResult;
        }
    }
    return (RayCollideWithObjectsResult){finalcollideResult, finalcollideObj};
}

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, const Object **objects,
                 int objectsCount, const LightSource **sources, int sourcesCount, int renderCount) {
    if (sources == NULL) return;
    /*
     * Some basic idea about this function is to generate a reversed ray start from the camera and
     * goes through a pixel, then see if it collide with an object. If it collides with an object
     * then do something consequently.
     */
    ReversedRay initRay = {*camera, minus((Vector){x, y, 0}, *camera)};
    RayCollideWithObjectsResult result = rayCollideWithObjects(initRay, objects, objectsCount);
    CollideColorResult finalcollideResult = result.finalcollideResult;
    const Object *firstcollideObj = result.finalcollideObj;
    if (firstcollideObj == NULL) return;

}
