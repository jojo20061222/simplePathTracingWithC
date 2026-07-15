#include "render.h"
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

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, const Object **objects,
                 int objectsCount, const LightSource *sources, int sourcesCount, double ambient) {
    /*
     * Some basic idea about this function is to generate a reversed ray start from the camera and
     * goes through a pixel, then see if it collide with an object. If it collides with an object
     * then do something consequently.
     */
    ReversedRay *initRay = malloc(sizeof(ReversedRay));
    *initRay = (ReversedRay){*camera, minus((Vector){x, y, 0}, *camera), 1.0};
    double minT = INFINITY;
    CollideColorResult finalCollisionResult;
    for (int i = 0; i < objectsCount; i++) {
        CollideColorResult collisionResult = collideObject(objects[i], initRay);
        if (!collisionResult.collide) continue;
        if (collisionResult.t < minT) {
            minT = collisionResult.t;
            finalCollisionResult = collisionResult;
        }
    }
    // TODO: to modify the program to calculate the reflection
    /*
    ScreenColor finalColor = {(uint8_t)(finalCollisionResult.color.R * 255),
                              (uint8_t)(finalCollisionResult.color.G * 255),
                              (uint8_t)(finalCollisionResult.color.B * 255)};
    */

    // calculate the diffuse and specular part
    if (sources != NULL) {
        for (int i = 0; i < sourcesCount; i++) {
            // test 
        }
    }
    ScreenColor finalColor = {// ambient + ...
                              (uint8_t)(ambient * 255 * finalCollisionResult.color.R),
                              (uint8_t)(ambient * 255 * finalCollisionResult.color.G),
                              (uint8_t)(ambient * 255 * finalCollisionResult.color.B), (uint8_t)0};
    

    *accessScreen(s, x, y) = finalColor;
}
