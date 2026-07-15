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

    // calculate the diffuse and specular part

    LightIntensity specularIntensity = {0, 0, 0};
    LightIntensity diffusionIntensity = {0, 0, 0};

    if (sources != NULL) {
        // first we generate the reflection light
        // direction be r let N = ^n, then r = d - 2(d dot N)N

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
        ReversedRay reflectionRay = {
            plus(scalerMultiplication(error * 10, reflectDirection), collidePoint),
            reflectDirection};

        RayCollideWithObjectsResult reflectObjectResult =
            rayCollideWithObjects(reflectionRay, objects, objectsCount);

        for (int i = 0; i < sourcesCount; i++) {
            // test if the reversed reflection light can directly reaches the light source
            // Specular Part
            CollisionResult reflectLightSourceResult =
                sources[i]->shape->detectCollide(sources[i]->shape, &reflectionRay);
            if (reflectLightSourceResult.collide) {
                if (reflectObjectResult.finalCollisionResult.t >= reflectLightSourceResult.t)
                    specularIntensity = plusLightIntensity(
                        specularIntensity,
                        (LightIntensity){sources[i]->intensity.R * finalCollisionObj->color.R,
                                         sources[i]->intensity.G * finalCollisionObj->color.G,
                                         sources[i]->intensity.B * finalCollisionObj->color.B});
            }
            // Diffusion Part
            Vector diffuseRayDirection = minus(sources[i]->center, collidePoint);
            ReversedRay diffuseRay = {
                plus(scalerMultiplication(error * 10, diffuseRayDirection), collidePoint),
                diffuseRayDirection};
        }
    }

    ScreenColor finalColor = {
        (uint8_t){255 * (ambient * finalCollisionResult.color.R + specularIntensity.R)},
        (uint8_t){255 * (ambient * finalCollisionResult.color.G + specularIntensity.G)},
        (uint8_t){255 * (ambient * finalCollisionResult.color.B + specularIntensity.B)},
        (uint8_t)0};

    *accessScreen(s, x, y) = finalColor;
}
