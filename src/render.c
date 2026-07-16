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
    CollideColorResult finalCollisionResult;
    const Object *finalCollisionObj;
} RayCollideWithObjectsResult;

RayCollideWithObjectsResult rayCollideWithObjects(ReversedRay initRay, const Object **objects,
                                                  int objectsCount) {
    double minT = INFINITY;
    const Object *finalCollisionObj = NULL;
    CollideColorResult finalCollisionResult;
    for (int i = 0; i < objectsCount; i++) {
        CollideColorResult collisionResult = collideObject(objects[i], &initRay);
        if (!collisionResult.collide) continue;
        if (collisionResult.t < minT) {
            finalCollisionObj = objects[i];
            minT = collisionResult.t;
            finalCollisionResult = collisionResult;
        }
    }
    return (RayCollideWithObjectsResult){finalCollisionResult, finalCollisionObj};
}

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, const Object **objects,
                 int objectsCount, const LightSource **sources, int sourcesCount, double ambient) {
    if (sources == NULL) return;
    /*
     * Some basic idea about this function is to generate a reversed ray start from the camera and
     * goes through a pixel, then see if it collide with an object. If it collides with an object
     * then do something consequently.
     */
    ReversedRay initRay = {*camera, minus((Vector){x, y, 0}, *camera)};
    RayCollideWithObjectsResult result = rayCollideWithObjects(initRay, objects, objectsCount);
    CollideColorResult finalCollisionResult = result.finalCollisionResult;
    const Object *finalCollisionObj = result.finalCollisionObj;
    if (finalCollisionObj == NULL) return;

    /*
    ScreenColor finalColor = {(uint8_t)(finalCollisionObj->color.R * 255),
                              (uint8_t)(finalCollisionObj->color.G * 255),
                              (uint8_t)(finalCollisionObj->color.B * 255), 0U};
    */
    
    *accessScreen(s, x, y) = (ScreenColor)0U;
}
