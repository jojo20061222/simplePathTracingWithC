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

static const double startWavelength = 390;
static const double endWavelength = 790;

VirtualScreen *newScreen(int width, int height) {
    VirtualScreen *result = malloc(sizeof(*result));
    result->width = width;
    result->height = height;
    result->buffer = calloc(width * height, sizeof(*result->buffer));
    result->renderCount = calloc(width * height, sizeof(*result->renderCount));
    result->mean = calloc(width * height, sizeof(*result->mean));
    result->m2 = calloc(width * height, sizeof(*result->m2));
    result->converge = calloc(width * height, sizeof(*result->converge));

    return result;
}

void freeScreen(VirtualScreen *screen) {
    free(screen->buffer);
    free(screen->mean);
    free(screen->renderCount);
    free(screen->m2);
    free(screen->converge);
    free(screen);
    return;
}

static inline void updateScreen(VirtualScreen *vs, int x, int y, ColorXYZ color) {
    int i = y * vs->width + x;
    // use Wellford's Algorithm to calculate variance
    int n = ++vs->renderCount[i];
    double delta = color.Y - vs->mean[i].Y;
    vs->mean[i].X += (color.X - vs->mean[i].X) / n;
    vs->mean[i].Y += (color.Y - vs->mean[i].Y) / n;
    vs->mean[i].Z += (color.Z - vs->mean[i].Z) / n;

    double delta2 = color.Y - vs->mean[i].Y;
    vs->m2[i] += delta * delta2;

    vs->buffer[i] = XYZtoRGB(vs->mean[i]);
}

static inline Vector GGXVectorSamle(MT19937 *mt, double alpha) {
    double u1 = randomReal(mt), u2 = randomReal(mt);
    double theta = 2 * M_PI * u1;
    double cosPhi = sqrt((1 - u2) / (1 + (alpha * alpha - 1) * u1));
    double sinPhi = sqrt(1 - cosPhi * cosPhi);
    // <cos theta sihPhi, sin theta sin phi, cos phi>
    return (Vector){cos(theta) * sinPhi, sin(theta) * sinPhi, cosPhi};
}

static inline Vector toWorld(Vector local, Vector N) {
    // finally we turn this into right angle
    Vector helper = (fabs(N.x) > 0.9) ? ((Vector){0, 1, 0}) : ((Vector){1, 0, 0});
    Vector T = unitVector(vecCross(helper, N));
    Vector B = vecCross(N, T);
    // project the unturned to turned
    Vector nxtDirection = (Vector){local.x * T.x + local.y * B.x + local.z * N.x,
                                   local.x * T.y + local.y * B.y + local.z * N.y,
                                   local.x * T.z + local.y * B.z + local.z * N.z};
    return nxtDirection;
}

LightIntensity simulateSingleRay(ReversedRay ray, const Object **objects, int objectsCount,
                                 const LightSource **sources, int sourcesCount, MT19937 *mt,
                                 int maxDepth) {
    if (maxDepth == 0) return 0.0;

    // first we see if this light first touches a light source
    // to attain that, we need to simulate both collide with sources and objects to decide which is
    // touched first.

    // test the first object to touch, ignore light source
    CollideResult firstCollideObjectResult = {false, INFINITY};
    const Object *firstCollideObject = NULL;
    for (int i = 0; i < objectsCount; i++) {
        CollideResult collideResult = collideObject(objects[i], &ray);
        if (!collideResult.collide) continue;
        if (collideResult.t < firstCollideObjectResult.t) {
            firstCollideObjectResult = collideResult;
            firstCollideObject = objects[i];
        }
    }

    // then we do the same on light source
    CollideLightSourceResult firstCollideSourceResult = {false, INFINITY, 0};
    const LightSource *firstCollideSource = NULL;
    for (int i = 0; i < sourcesCount; i++) {
        CollideLightSourceResult collideResult = collideLightSource(sources[i], &ray);
        if (!collideResult.collide) continue;
        if (collideResult.t < firstCollideSourceResult.t) {
            firstCollideSourceResult = collideResult;
            firstCollideSource = sources[i];
        }
    }
    // case 1: collide with nothing.
    if (firstCollideObject == NULL && firstCollideSource == NULL) {
        return 0.0;
    }

    // case 2: first collide with source, so we return the intensity of light source directly
    if (firstCollideSourceResult.t <= firstCollideObjectResult.t && firstCollideSource != NULL) {
        return firstCollideSourceResult.intensity;
    }

    // case 3: we collide with an object first
    Vector d = ray.direction;
    Point collidePoint = vecAdd(ray.start, vecMultiply(firstCollideObjectResult.t, d));
    Vector N = unitVector(
        firstCollideObject->shape->normalVector(firstCollideObject->shape, collidePoint));
    Complex n1, n2;
    if (vecDot(N, d) > 0) {
        // N vecDot d < 0 implys the ray is now in the object
        // n1 = inside, n2 = outside, since we goes from inside to outside
        N = (Vector){-N.x, -N.y, -N.z};
        n1 = firstCollideObject->texture.refractiveIndexInside(ray.waveLength);
        n2 = firstCollideObject->texture.refractiveIndexOutside(ray.waveLength);
    }
    else {
        n1 = firstCollideObject->texture.refractiveIndexOutside(ray.waveLength);
        n2 = firstCollideObject->texture.refractiveIndexInside(ray.waveLength);
    }
    Vector nxtDirection;

    double energyCoefficient = 1;
    if (firstCollideObject->texture.metalic) {
        // BRDF
        Vector omegaI = vecNegative(unitVector(ray.direction));
        double alpha =
            firstCollideObject->texture.roughness * firstCollideObject->texture.roughness;
        Vector m = toWorld(GGXVectorSamle(mt, alpha), N);
        Vector omegaO = vecMinus(d, vecMultiply(2 * vecDot(d, m), m));
        // calculate F part
        double cosTheta = vecDot(omegaI, m);
        double x = cplxModulus(cplxDivide(cplxMinus(n1, n2), cplxAdd(n1, n2)));
        double F0 = x * x;
        double y = 1 - cosTheta;
        double y2 = y * y;
        double y5 = y2 * y2 * y;
        double F = F0 + (1 - F0) * y5;
        // calculate G part
        // Smith Model
        double G = ;
    }

    else {
        // in such case we need to throw a dice first to decide which way to reflect
        double cosTheta = -vecDot(d, N) / (lengthOf(d) * lengthOf(N));
        // the reflection probability is decided by Fresnell Equaltion
        double x = cplxModulus(cplxDivide(cplxMinus(n1, n2), cplxAdd(n1, n2)));
        double F0 = x * x;
        double y = 1 - cosTheta;
        double y2 = y * y;
        double y5 = y2 * y2 * y;
        double F = F0 + (1 - F0) * y5;

        if (randomReal(mt) > F) {
            // We DO NOT consider the refraction or diffusion of metals
            if (!firstCollideObject->texture.transparent) {
                // Branch Diffusion, only non-transparent materials need to do this
                // first we generate a random point within a circle (uniformly)
                // To convert this into distribution of probabilty, we find the probability near
                // center of the circle (within 0 <= d <= r): (pi*r^2) / (piR^2) we want to use a
                // uniform distributed variable u1 to decide if the point is in such area, so u =
                // (r/R)^2, r = Rsqrt(u)
                // https://stackoverflow.com/questions/5837572/generate-a-random-point-within-a-circle-uniformly
                double u1 = randomReal(mt), u2 = randomReal(mt);
                double sinPhi = sqrt(1 - u1), theta = 2 * M_PI * u2;

                // then we shall project it onto an sphere, then we get the cosine distribution
                // normally suppose the normal vector point towards z-axis, then base on sphirical
                // coordinate <1, theta, phi> <=> <cos(theta)sin(phi), sin(theta)sin(phi), cos(phi)>
                // where sin(phi) is sqrt(1 - r^2), or sqrt(1 - u1)
                Vector local = {cos(theta) * sinPhi, sin(theta) * sinPhi, sqrt(u1)};
                nxtDirection = toWorld(local, N);
                energyCoefficient *= firstCollideObject->texture.baseColor(ray.waveLength);
            }
            else if (!firstCollideObject->texture.metalic) {
                // TODO: refraction!
            }
        }
        // case specular
        // the direction of reflection can be calculated as r = d - 2(d vecDot N)N
        // where N stands for unit vector of normal vector (n hat), d is the direction of original
        // ray, r is the direction of reflection
        else {
            nxtDirection = vecMinus(d, vecMultiply(2 * vecDot(d, N), N));
            energyCoefficient = 1;
        }
    }

    ReversedRay nxtRay =
        newRay(vecAdd(collidePoint, vecMultiply(1e-4, nxtDirection)), nxtDirection, ray.waveLength);

    return simulateSingleRay(nxtRay, objects, objectsCount, sources, sourcesCount, mt,
                             maxDepth - 1) *
           energyCoefficient;
}

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, const Object **objects,
                 int objectsCount, const LightSource **sources, int sourcesCount, MT19937 *mt,
                 int maxDepth) {
    if (sources == NULL) return;
    double fixedX = (double)x + randomReal(mt);
    double fixedY = (double)y + randomReal(mt);
    /*
     * Some basic idea about this function is to generate a reversed ray start from the camera and
     * goes through a pixel, then see if it collide with an object. If it collides with an object
     * then do something consequently.
     */
    // we divide the interval [390, 790] into 4 intervals averagely
    const int intervalCount = 4;
    const double intervalSize = (endWavelength - startWavelength) / (double)intervalCount;
    ReversedRay initRays[intervalCount];
    LightIntensity intensityResults[intervalCount];
    double randomWavelength =
        randomReal(mt) * (endWavelength - startWavelength) / intervalCount + startWavelength;

    for (int i = 0; i < intervalCount; i++) {
        initRays[i] = newRay(*camera, vecMinus((Vector){fixedX, fixedY, 0}, *camera),
                             randomWavelength + intervalSize * i);

        intensityResults[i] = simulateSingleRay(initRays[i], objects, objectsCount, sources,
                                                sourcesCount, mt, maxDepth);
    }

    ColorXYZ colorResult = {0, 0, 0};
    for (int i = 0; i < intervalCount; i++) {
        colorResult = colorAdd(
            colorResult, colorComponent(intensityResults[i] * intervalSize, // / PDF
                                        wavelengthToXYZ(randomWavelength + intervalSize * i)));
    }

    updateScreen(s, x, y, colorResult);
    return;
}
