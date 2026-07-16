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
    result->buffer = calloc(width * height, sizeof(ScreenColor));
    result->sum = malloc(sizeof(LightIntensity) * width * height);
    for (int i = 0; i < width * height; i++) {
        result->sum[i] = (LightIntensity){0, 0, 0};
    }
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
    if (firstCollideSourceResult.t <= firstCollideObjectResult.t && firstCollideSource != NULL) {
        return firstCollideSourceResult.intensity;
    }

    // case 3: we collide with an object first
    Vector d = ray.direction;
    Point collidePoint = plus(ray.start, scalerMultiplication(firstCollideObjectResult.t, d));
    Vector N = unitVector(
        firstCollideObject->shape->normalVector(firstCollideObject->shape, collidePoint));
    if (dot(N, d) > 0) N = (Vector){-N.x, -N.y, -N.z};
    Vector nxtDirection;

    // in such case we need to throw a dice first to decide which way to reflect
    if (randomReal(mt) <= firstCollideObject->diffuseFactor) {
        // first we generate a random point within a circle (uniformly)
        // To convert this into distribution of probabilty, we find the probability near center
        // of the circle (within 0 <= d <= r): (pi*r^2) / (piR^2)
        // we want to use a uniform distributed variable u1 to decide if the point is in such area,
        // so u = (r/R)^2, r = Rsqrt(u)
        // https://stackoverflow.com/questions/5837572/generate-a-random-point-within-a-circle-uniformly
        double u1 = randomReal(mt), u2 = randomReal(mt);
        double sinPhi = sqrt(1 - u1), theta = 2 * M_PI * u2;

        // then we shall project it onto an sphere, then we get the cosine distribution normally
        // suppose the normal vector point towards z-axis, then base on sphirical coordinate
        // <1, theta, phi> <=> <cos(theta)sin(phi), sin(theta)sin(phi), cos(phi)>
        // where sin(phi) is sqrt(1 - r^2), or sqrt(1 - u1)
        Vector unturnedDirection = {cos(theta) * sinPhi, sin(theta) * sinPhi, sqrt(u1)};

        // finally we turn this into right angle
        Vector helper = (fabs(N.x) > 0.9) ? ((Vector){0, 1, 0}) : ((Vector){1, 0, 0});
        Vector T = unitVector(cross(helper, N));
        Vector B = cross(N, T);
        // project the unturned to turned
        nxtDirection = (Vector){
            unturnedDirection.x * T.x + unturnedDirection.y * B.x + unturnedDirection.z * N.x,
            unturnedDirection.x * T.y + unturnedDirection.y * B.y + unturnedDirection.z * N.y,
            unturnedDirection.x * T.z + unturnedDirection.y * B.z + unturnedDirection.z * N.z};
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

static inline LightIntensity *accessLightIntensitySum(VirtualScreen *vs, int x, int y) {
    return &vs->sum[y * vs->width + x];
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

    *accessLightIntensitySum(s, x, y) =
        plusLightIntensity(*accessLightIntensitySum(s, x, y), intensityResult);

    LightIntensity averageIntensity = *accessLightIntensitySum(s, x, y);
    averageIntensity.R /= (double)renderCount;
    averageIntensity.G /= (double)renderCount;
    averageIntensity.B /= (double)renderCount;

    *accessScreen(s, x, y) = intensityToColor(averageIntensity);
}
