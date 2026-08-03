#include "render.h"
#include "base.h"
#include "lightSource.h"
#include "object.h"
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
    return result;
}

void freeScreen(VirtualScreen *screen) {
    free(screen->buffer);
    free(screen->mean);
    free(screen->renderCount);
    free(screen);
    return;
}

static inline void updateScreen(VirtualScreen *vs, int x, int y, ColorXYZ color) {
    int i = (vs->height - y - 1) * vs->width + x;
    // online mean update
    int n = ++vs->renderCount[i];
    vs->mean[i].X += (color.X - vs->mean[i].X) / n;
    vs->mean[i].Y += (color.Y - vs->mean[i].Y) / n;
    vs->mean[i].Z += (color.Z - vs->mean[i].Z) / n;

    vs->buffer[i] = XYZtoRGB(vs->mean[i]);
}

static inline Vector GGXVectorSample(MT19937 *mt, double alpha) {
    double u1 = randomReal(mt), u2 = randomReal(mt);
    double theta = 2 * M_PI * u1;
    double cosPhi = sqrt((1 - u2) / (1 + (alpha * alpha - 1) * u2));
    double sinPhi = sqrt(fmax(0, 1 - cosPhi * cosPhi));
    // <cos theta sinPhi, sin theta sin phi, cos phi>
    return (Vector){cos(theta) * sinPhi, sin(theta) * sinPhi, cosPhi};
}

static inline Vector toWorld(Vector local, Vector N) {
    Vector helper = (fabs(N.x) > 0.9) ? ((Vector){0, 1, 0}) : ((Vector){1, 0, 0});
    Vector T = unitVector(vecCross(helper, N));
    Vector B = vecCross(N, T);
    // project the unturned to turned
    Vector nxtDirection = (Vector){local.x * T.x + local.y * B.x + local.z * N.x,
                                   local.x * T.y + local.y * B.y + local.z * N.y,
                                   local.x * T.z + local.y * B.z + local.z * N.z};
    return nxtDirection;
}

static LightIntensity simulateSingleRay(ReversedRay ray, Object **objects, LightSource **sources,
                                        MT19937 *mt, int maxDepth) {
    if (maxDepth == 0) return 0;

    int objectsCount = getSize(objects);
    int sourcesCount = getSize(sources);

    // first we see if this light first touches a light source
    // to attain that, we need to simulate both collide with sources and objects to decide which is
    // touched first.

    // test the first object to touch, ignore light source
    CollideResult firstCollideObjectResult = {false, INFINITY};
    Object *firstCollideObject = NULL;
    for (int i = 0; i < objectsCount; i++) {
        CollideResult collideResult = collideObject(objects[i], &ray);
        if (!collideResult.collide) continue;
        if (collideResult.t <= firstCollideObjectResult.t) {
            firstCollideObjectResult = collideResult;
            firstCollideObject = objects[i];
        }
    }

    // then we do the same on light source
    CollideLightSourceResult firstCollideSourceResult = {false, INFINITY, 0};
    LightSource *firstCollideSource = NULL;
    for (int i = 0; i < sourcesCount; i++) {
        CollideLightSourceResult collideResult = collideLightSource(sources[i], &ray);
        if (!collideResult.collide) continue;
        if (collideResult.t <= firstCollideSourceResult.t) {
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
    // Now the normal vector is still point toward outside
    Vector N = firstCollideObjectResult.normalVector;

    Complex n1, n2;
    if (vecDot(N, d) > 0) {
        // N vecDot d < 0 implys the ray is now in the object
        // n1 = inside, n2 = outside, since we goes from inside to outside
        N = (Vector){-N.x, -N.y, -N.z};
        n1 = getRefractiveIndexInside(firstCollideObject, ray.waveLength);
        n2 = getRefractiveIndexOutside(firstCollideObject, ray.waveLength);
    }
    else {
        n1 = getRefractiveIndexOutside(firstCollideObject, ray.waveLength);
        n2 = getRefractiveIndexInside(firstCollideObject, ray.waveLength);
    }
    // after the operations above, the normal vector should point on the side where reversed rays
    // come from

    Vector v = vecNegative(ray.direction); // direction from collide point to observer
    double roughness = getRoughness(firstCollideObject);
    // By Disney's guidelines we square the roughness here to get alpha
    double alpha = roughness * roughness;

    Vector h = toWorld(GGXVectorSample(mt, alpha), N);        // sample normal vector through GGX
    Vector l = vecMinus(vecMultiply(2 * vecDot(v, h), h), v); // direction of reflection
    // calculate F part
    double VdotH = vecDot(v, h);
    if (VdotH <= 0) return 0;
    double cosTheta = VdotH;

    // check total internal reflection
    if (n1.R > n2.R) {
        // TIR need n1 sinTheta >= n2
        // or squared n1^2 sin^2Theta >= n2^2
        double n1R2 = n1.R * n1.R;
        double n2R2 = n2.R * n2.R;
        double sin2Theta = 1 - cosTheta * cosTheta;
        if (n1R2 * sin2Theta >= n2R2) {
            cosTheta = 0;
        }
    }

    // Schlick's approximation
    // F = F0 + (1 - F0) * y^5
    double x = cplxModulus(cplxDivide(cplxMinus(n1, n2), cplxAdd(n1, n2)));
    double F0 = x * x;
    double y = 1 - cosTheta;
    double y2 = y * y;
    double y5 = y2 * y2 * y;
    double F = F0 + (1 - F0) * y5;
    bool metallic = getmetallic(firstCollideObject);

    if (metallic || randomReal(mt) <= F) {
        // BRDF
        // calculate V * (v dot h) / (n dot h) part = DG / PDF
        double NdotV = vecDot(N, v);
        double NdotL = vecDot(N, l);
        double NdotH = vecDot(N, h);

        static const double error = 1e-6;

        if (NdotV <= error || NdotL <= error || NdotH <= error) {
            return 0;
        }

        // calculating 4 * F * V * (N dot L) * (V dot H) / (N dot H)
        double alpha2 = alpha * alpha;
        double NdotV2 = NdotV * NdotV;
        double NdotL2 = NdotL * NdotL;
        double denom = (NdotL * sqrt(alpha2 + (1 - alpha2) * NdotV2) +
                        NdotV * sqrt(alpha2 + (1 - alpha2) * NdotL2));
        double finalFactor = (2.0 * NdotL * VdotH) / (denom * NdotH);

        // since metallic won't go into the 1 - F branch so we do have to * F
        if (metallic) finalFactor *= F;

        Vector nxtDirection = l;
        // to avoid collide with the same object again, we move a little from the collide point
        Point nxtStart = vecAdd(collidePoint, vecMultiply(1e-4, nxtDirection));
        ReversedRay nxtRay = newRay(nxtStart, nxtDirection, ray.waveLength);
        double result = finalFactor * simulateSingleRay(nxtRay, objects, sources, mt, maxDepth - 1);
        return result;
    }

    if (metallic) return 0; // metal only has specular

    bool transparent = getTransparent(firstCollideObject);

    if (transparent) {
        // Note that we have returned a value in reflection branch so the proabability of this
        // branch is 1 - F
        // There is no need to * (1 - F) in the result

        // First we test the condition that all transparent object have smooth
        // surface (roughness = 0)
        Vector I = d;
        double cosTheta1 = -vecDot(N, I);
        assert(cosTheta1 >= 0);
        double n1R = n1.R;
        double n2R = n2.R;
        /*
         * We simply assume that
         * >> I = <sin(Theta1), -cos(Theta1)>
         * >> O = <sin(Theta2), -cos(Theta2)>
         * >> N = <0, 1>
         *
         * Since there exist a simple rotating Jacobian that rotate these vectors to world,
         * we simply use these simplified vectors to do linear combinations and the results won't
         * change
         *
         * With snell's law, n1 sin(Theta1) = n2 sin(Theta2)
         *
         * Hence,
         * >> (n1/n2)sin(Theta1) = sin(Theta2)
         * we take relative refractivity as eta
         * >> eta * sin(Theta1) = sin(Theta2)
         * >> eta^2(1 - cos^2(Theta1)) = (1 - cos^2(Theta2))
         * >> sqrt(1 - eta^2(1 - cos^2(Theta1))) = cos(Theta2)
         *
         * We first do the cosine part
         * >> -sqrt(1 - eta^2(1 - cos^2(Theta1))) N
         *
         * Then the sine part
         * >> eta * I + eta * cos(Theta1) N
         * Sum up the reault
         * >> eta * I + (eta * cos(Theta1) - sqrt(1 - eta^2(1 - cos^2(Theta1))))N
         */
        double eta = n1R / n2R;
        double coefficientOfSecondPart =
            eta * cosTheta1 - sqrt(1 - eta * eta * (1 - cosTheta1 * cosTheta1));
        Vector nxtDirection = vecAdd(vecMultiply(eta, I), vecMultiply(coefficientOfSecondPart, N));
        Point nxtStart = vecAdd(collidePoint, vecMultiply(1e-4, nxtDirection));
        ReversedRay nxtRay = newRay(nxtStart, nxtDirection, ray.waveLength);
        // this will not cause burst of value so we don't check here
        double result = simulateSingleRay(nxtRay, objects, sources, mt, maxDepth - 1);
        return result;
    }

    else {
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
        // normally suppose the normal vector point towards z-axis, then base on spherical
        // coordinate <1, theta, phi> <=> <cos(theta)sin(phi), sin(theta)sin(phi), cos(phi)>
        // where sin(phi) is sqrt(1 - r^2), or sqrt(1 - u1)
        Vector local = {cos(theta) * sinPhi, sin(theta) * sinPhi, sqrt(u1)};
        Vector nxtDirection = toWorld(local, N);
        ReversedRay nxtRay = newRay(vecAdd(collidePoint, vecMultiply(1e-4, nxtDirection)),
                                    nxtDirection, ray.waveLength);

        return simulateSingleRay(nxtRay, objects, sources, mt, maxDepth - 1) *
               getBaseColor(firstCollideObject, ray.waveLength);
    }

    return 0;
}

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, Object **objects,
                 LightSource **sources, MT19937 *mt, int maxDepth) {
    if (sources == NULL) return;
    double fixedX = (double)x + randomReal(mt);
    double fixedY = (double)y + randomReal(mt);
    /*
     * Some basic idea about this function is to generate a reversed ray start from the camera and
     * goes through a pixel, then see if it collide with an object. If it collides with an object
     * then do something consequently.
     */
    double intervalSize = endWavelength - startWavelength;
    ReversedRay initRay;
    LightIntensity intensityResult;
    double randomWavelength = randomReal(mt) * intervalSize + startWavelength;

    initRay = newRay(*camera, vecMinus((Vector){fixedX, fixedY, 0}, *camera), randomWavelength);

    intensityResult = simulateSingleRay(initRay, objects, sources, mt, maxDepth);

    if (!isfinite(intensityResult)) return;

    ColorXYZ colorResult =
        colorComponent(intensityResult * intervalSize, wavelengthToXYZ(randomWavelength));

    updateScreen(s, x, y, colorResult);
    return;
}
