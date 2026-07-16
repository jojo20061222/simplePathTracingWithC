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

LightIntensity *newLightIntensitySum(int width, int height) {
    LightIntensity *result = malloc(sizeof(LightIntensity) * width * height);
    for (int i = 0; i < width * height; i++) {
        result[i] = (LightIntensity){0.0, 0.0, 0.0};
    }
    return result;
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
    Vector d = ray.direction;
    Point collidePoint = plus(ray.start, scalerMultiplication(firstCollideObjectResult.t, d));
    Vector N = unitVector(
        firstCollideObject->shape->normalVector(firstCollideObject->shape, collidePoint));
    Vector nxtDirection;

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
        double sinPhi = sqrt(1 - u1), theta = 2 * M_PI * u2;

        // then we shall project it onto an sphere, then we get the cosine distribution normally
        // suppose the normal vector point towards z-axis, then base on sphirical coordinate
        // <1, theta, phi> <=> <cos(theta)sin(phi), sin(theta)sin(phi), cos(phi)>
        // where sin(phi) is sqrt(1 - r^2), or sqrt(1 - u1)
        Vector unturnedDirection = {cos(theta) * sinPhi, sin(theta) * sinPhi, sqrt(u1)};

        // finally we turn this into right angle
        Vector helper = 1 - N.x < error ? (Vector){0, 1, 0} : (Vector){1, 0, 0};
        Vector T = cross(helper, N);
        Vector B = cross(helper, T);
        // project the unturned to turned
        // [N B T]X
        nxtDirection = (Vector){dot((Vector){N.x, B.x, T.x}, unturnedDirection),
                                dot((Vector){N.y, B.y, T.y}, unturnedDirection),
                                dot((Vector){N.z, B.z, T.z}, unturnedDirection)};
    }

    // case specular
    // the direction of reflection can be calculated as r = d - 2(d dot N)N
    // where N stands for unit vector of normal vector (n hat), d is the direction of original
    // ray, r is the direction of reflection
    else {
        nxtDirection = minus(d, scalerMultiplication(2 * dot(d, N), N));
    }

    return irradiate(
        simulateSingleRay(
            (ReversedRay){plus(collidePoint, scalerMultiplication(error * 10, nxtDirection)),
                          nxtDirection},
            objects, objectsCount, sources, sourcesCount, mt, maxDepth - 1),
        firstCollideObjectResult.color);
}

static inline LightIntensity *accessLightIntensitySum(LightIntensity *lightIntensitySum, int x,
                                                      int y, int width) {
    return &lightIntensitySum[y * width + x];
}

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, const Object **objects,
                 int objectsCount, const LightSource **sources, int sourcesCount, int renderCount,
                 LightIntensity *intensitySum, MT19937 *mt, int maxDepth) {
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

    *accessLightIntensitySum(intensitySum, x, y, s->width) =
        plusLightIntensity(*accessLightIntensitySum(intensitySum, x, y, s->width), intensityResult);

    LightIntensity averageIntensity = *accessLightIntensitySum(intensitySum, x, y, s->width);
    averageIntensity.R /= (double)renderCount;
    averageIntensity.G /= (double)renderCount;
    averageIntensity.B /= (double)renderCount;

    *accessScreen(s, x, y) = intensityToColor(averageIntensity);
}
