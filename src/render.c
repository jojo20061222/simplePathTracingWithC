#include "render.h"
#include "base.h"
#include "lightSource.h"
#include "random.h"
#include <assert.h>
#include <limits.h>
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

LightIntensity simulateSingleRay(ReversedRay ray, const Object **objects, int objectsCount,
                                 const LightSource **sources, int sourcesCount, MT19937 *mt,
                                 int maxDepth) {
    if (maxDepth == 0) return (LightIntensity){0, 0, 0};

    // first we see if this light first touches a light source
    // to attain that, we need to simulate both collide with sources and objects to decide which is
    // touched first.

    // test the first object to touch, ignore light source
    CollideColorResult firstCollideObjectResult = {false, INFINITY, {0, 0, 0}};
    const Object *firstCollideObject = NULL;
    for (int i = 0; i < objectsCount; i++) {
        CollideColorResult collideResult = collideObject(objects[i], &ray);
        if (!collideResult.collide) continue;
        if (collideResult.t < firstCollideObjectResult.t) {
            firstCollideObjectResult = collideResult;
            firstCollideObject = objects[i];
        }
    }

    // then we do the same on light source
    CollideLightSourceResult firstCollideSourceResult = {false, INFINITY, {0, 0, 0}};
    const LightSource *firstCollideSource = NULL;
    for (int i = 0; i < sourcesCount; i++) {
        CollideLightSourceResult collideResult = collideLightSource(sources[i], &ray);
        if (!collideResult.collide) continue;
        if (collideResult.t < firstCollideSourceResult.t) {
            firstCollideSourceResult = collideResult;
            firstCollideSource = sources[i];
        }
    }
    // case 1: we collide with nothing.
    if (firstCollideObject == NULL && firstCollideSource == NULL) {
        return (LightIntensity){0, 0, 0};
    }

    // case 2: first collide with source, so we return the intensity of light source directly
    if (firstCollideSourceResult.t <= firstCollideObjectResult.t) {
        return firstCollideSourceResult.intensity;
    }

    // case 3: we collide with an object first
    // in such case we need to throw a dice first to decide which way to reflect
    double dice = (double)extractNumber(mt) / (double)UINT_MAX;
    if (dice <= firstCollideObject->diffuseFactor) {
        // first we generate a random point within a circle (uniformly)
        // To convert this into distribution of probabilty, we find the probability near center
        // of the circle (within 0 <= d <= r): (pi*r^2) / (piR^2)
        // we want to use a uniform distributed variable u1 to decide if the point is in such area,
        // so u = (r/R)^2, r = Rsqrt(u)
        // idk, the this is the link from stack overflow
        // https://stackoverflow.com/questions/5837572/generate-a-random-point-within-a-circle-uniformly
        double u1 = randomReal(mt), u2 = randomReal(mt);
        double r = sqrt(u1), theta = 2 * M_PI * u2;

        // then we shall project it onto an sphere, then we get the cosine distribution normally
        Vector 
    }

    // case specular
    // the direction of reflection can be calculated as r = d - 2(d dot ^n)^n
    // where ^n stands for unit vector of normal vector (hat), d is the direction of original
    // ray, r is the direction of reflection

    Vector d = ray.direction;
    Point collidePoint = plus(ray.start, scalerMultiplication(firstCollideObjectResult.t, d));
    Vector n = unitVector(
        firstCollideObject->shape->normalVector(firstCollideObject->shape, collidePoint));
    Vector r = minus(d, scalerMultiplication(2 * dot(d, n), n));

    return irradiate(
        simulateSingleRay((ReversedRay){plus(collidePoint, scalerMultiplication(error * 10, r)), r},
                          objects, objectsCount, sources, sourcesCount, mt, maxDepth - 1),
        firstCollideObjectResult.color);
}

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, const Object **objects,
                 int objectsCount, const LightSource **sources, int sourcesCount, int renderCount,
                 MT19937 *mt, int maxDepth) {
    if (sources == NULL) return;
    /*
     * Some basic idea about this function is to generate a reversed ray start from the camera and
     * goes through a pixel, then see if it collide with an object. If it collides with an object
     * then do something consequently.
     */
    ReversedRay initRay = {*camera, minus((Vector){x, y, 0}, *camera)};

    // Path tracing
    LightIntensity intensityResult =
        simulateSingleRay(initRay, objects, objectsCount, sources, sourcesCount, mt, maxDepth);
    *accessScreen(s, x, y) = intensityToColor(intensityResult);
}
