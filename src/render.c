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

RayCollideWithObjectsResult rayCollideWithObjects(Point *camera, int x, int y, const Object **objects, int objectsCount) {
    ReversedRay initRay = {*camera, minus((Vector){x, y, 0}, *camera)};
    double minT = INFINITY;
    const Object *finalCollisionObj;
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
                 int objectsCount, const LightSource *sources, int sourcesCount, double ambient) {
    /*
     * Some basic idea about this function is to generate a reversed ray start from the camera and
     * goes through a pixel, then see if it collide with an object. If it collides with an object
     * then do something consequently.
     */
    RayCollideWithObjectsResult result = rayCollideWithObjects(camera, x, y, objects, objectsCount);
    CollideColorResult finalCollisionResult = result.finalCollisionResult;
    const Object finalCollisionObj = result.finalCollisionObj;

    // TODO: to modify the program to calculate the reflection
    /*
    ScreenColor finalColor = {(uint8_t)(finalCollisionResult.color.R * 255),
                              (uint8_t)(finalCollisionResult.color.G * 255),
                              (uint8_t)(finalCollisionResult.color.B * 255)};
    */

    // calculate the diffuse and specular part
    if (sources != NULL) {
        ReversedRay initRay = {*camera, minus((Vector){x, y, 0}, *camera)};
        // first we generate the reflection light
        /*
         * To calculete the direction that reflection ray goes, we use the property that of a
         * diamond Let the direction vector be d, normal vector of a surface be n, reflecting
         * direction be r let N = ^n, then r = d - 2(d dot N)N
         */

        // d = initRay.direction
        Point collidePoint =
            plus(initRay.start, scalerMultiplication(finalCollisionResult.t, initRay.direction));
        Vector nHat = unitVector(
            finalCollisionObj->shape->normalVector(finalCollisionObj->shape, collidePoint));
        if (dot(nHat, initRay.direction) > 0) {
            nHat = (Vector){-nHat.x, -nHat.y, -nHat.z};
        }
        Vector reflectDirection =
            minus(initRay.direction, scalerMultiplication(2 * dot(initRay.direction, nHat), nHat));
        ReversedRay reflection = {
            plus(scalerMultiplication(error * 10, reflectDirection), collidePoint),
            reflectDirection};

        for (int i = 0; i < sourcesCount; i++) {
            // test if the reversed reflection light can directly reaches the light source
        }
    }
    ScreenColor finalColor = {// ambient + ...
                              (uint8_t)(ambient * 255 * finalCollisionResult.color.R),
                              (uint8_t)(ambient * 255 * finalCollisionResult.color.G),
                              (uint8_t)(ambient * 255 * finalCollisionResult.color.B), (uint8_t)0};

    *accessScreen(s, x, y) = finalColor;
}
