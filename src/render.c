#include "render.h"
#include "base.h"
#include "lightSource.h"
#include "object.h"
#include "random.h"
#include "shapes.h"
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * @author seaofvoices
 * @brief The main logic to make all the things come true. Simulate the phsical world and project it
 * onto the screen.
 */

static const Real startWavelength = 390;
static const Real endWavelength = 790;

typedef struct VirtualScreen {
    ScreenColor *buffer;
    unsigned long long *renderCount;
    ColorXYZ *mean;
#if defined(ADAPTIVE_SAMPLING)
    unsigned long long *threshold;
    struct {
        Real X2, Y2, Z2;
    } *m2;
    bool *convergent;
#endif
    int width, height;
} VirtualScreen;

static void mallocFailedExit() {
    fprintf(stderr, "malloc new screen failed\n");
    exit(1);
}

VirtualScreen *newScreen(int width, int height) {
    VirtualScreen *result = malloc(sizeof(*result));
    if (result == NULL) mallocFailedExit();

    result->width = width;
    result->height = height;

    result->buffer = calloc(width * height, sizeof(*result->buffer));
    if (result->buffer == NULL) mallocFailedExit();

    result->renderCount = calloc(width * height, sizeof(*result->renderCount));
    if (result->renderCount == NULL) mallocFailedExit();
    result->mean = calloc(width * height, sizeof(*result->mean));
    if (result->mean == NULL) mallocFailedExit();
#if defined(ADAPTIVE_SAMPLING)
    // Welford's algorithm needs m2 to calculate variance
    // we calculate variance of x, y, z so that colors will be considered
    result->m2 = calloc(width * height, sizeof(*result->m2));
    if (result->m2 == NULL) mallocFailedExit();

    // Adaptive sampling
    result->convergent = calloc(width * height, sizeof(*result->convergent));
    if (result->convergent == NULL) mallocFailedExit();

    result->threshold = calloc(width * height, sizeof(*result->threshold));
    if (result->threshold == NULL) mallocFailedExit();
#endif

    return result;
}

void destroyScreen(VirtualScreen *screen) {
    free(screen->buffer);
    free(screen->mean);
    free(screen->renderCount);
#if defined(ADAPTIVE_SAMPLING)
    free(screen->m2);
    free(screen->convergent);
    free(scaling->threshold);
#endif
    free(screen);
    return;
}

int getScreenWidth(VirtualScreen *vs) { return vs->width; }

int getScreenHeight(VirtualScreen *vs) { return vs->height; }

uint32_t getPixel(VirtualScreen *vs, int i) { return vs->buffer[i].pixel; }

uint32_t *getBuffer(VirtualScreen *vs) { return (uint32_t *)vs->buffer; }

int getIndex(VirtualScreen *vs, int x, int y) { return (vs->height - y - 1) * vs->width + x; }

int getX(VirtualScreen *vs, int i) { return i % vs->width; }

int getY(VirtualScreen *vs, int i) { return vs->height - (i / vs->width) - 1; }

// update the color of screen and update the error of such pixel
static inline void updateScreen(VirtualScreen *vs, int x, int y, ColorXYZ color) {
    // online mean update
    // Welford's algorithm
    int i = getIndex(vs, x, y);
    int n = ++vs->renderCount[i];

    Real dX = color.X - vs->mean[i].X;
    Real dY = color.Y - vs->mean[i].Y;
    Real dZ = color.Z - vs->mean[i].Z;
    vs->mean[i].X += dX / n;
    vs->mean[i].Y += dY / n;
    vs->mean[i].Z += dZ / n;

#if defined(ADAPTIVE_SAMPLING)
    vs->m2[i].X2 += dX * (color.X - vs->mean[i].X);
    vs->m2[i].Y2 += dY * (color.Y - vs->mean[i].Y);
    vs->m2[i].Z2 += dZ * (color.Z - vs->mean[i].Z);

    if (color.X == 0 && color.Y == 0 && color.Z == 0) {
        // won't stop until it really samples somthing
        vs->threshold[i]++;
    }

#endif
    vs->buffer[i] = XYZtoRGB(vs->mean[i]);
}

#if defined(ADAPTIVE_SAMPLING)
static inline Real getSingleRelativeError(Real mean, Real m2, Real n) {
    Real relativeError;
    if (n <= 1) {
        relativeError = INFINITY;
    }
    else {
        Real stdDev = sqrt(m2 / (Real)(n - 1));
        relativeError = stdDev / (rabs(mean) + error);
    }
    return relativeError;
}

Real getRelativeError(VirtualScreen *vs, int x, int y) {
    // calculate sample standard deviation
    int i = getIndex(vs, x, y);
    int n = vs->renderCount[i];
    Real relativeErrorX = getSingleRelativeError(vs->mean[i].X, vs->m2[i].X2, n);
    Real relativeErrorY = getSingleRelativeError(vs->mean[i].Y, vs->m2[i].Y2, n);
    Real relativeErrorZ = getSingleRelativeError(vs->mean[i].Z, vs->m2[i].Z2, n);
    return fmax(relativeErrorX, fmax(relativeErrorY, relativeErrorZ));
}
bool *accessConvergent(VirtualScreen *vs, int x, int y) {
    int i = getIndex(vs, x, y);
    return &vs->convergent[i];
}
#endif

// to avoid collide with the same object again, we move a little from the collide point
static inline Point fixedNextStart(Point curPoint, Vector N, bool transmission) {
    static const Real moveEpsilon = 1e-4;
    if (!transmission) return (Point)vecAdd(curPoint, vecMultiply(moveEpsilon, N));
    return (Point)vecAdd(curPoint, vecMultiply(-moveEpsilon, N));
}

static inline Vector GGXVectorSample(MT19937 *mt, Real alpha) {
    Real u1 = randomReal(mt), u2 = randomReal(mt);
    Real theta = 2 * M_PI * u1;
    Real cosPhi = sqrt((1 - u2) / (1 + (alpha * alpha - 1) * u2));
    Real sinPhi = sqrt(fmax(0, 1 - cosPhi * cosPhi));
    // <cos theta sinPhi, sin theta sin phi, cos phi>
    return (Vector){rcos(theta) * sinPhi, rsin(theta) * sinPhi, cosPhi};
}

static inline Vector cosineVectorSample(MT19937 *mt) {
    // first we generate a random point within a circle (uniformly)
    // To convert this into distribution of probabilty, we find the probability near
    // center of the circle (within 0 <= d <= r): (pi*r^2) / (piR^2) we want to use a
    // uniform distributed variable u1 to decide if the point is in such area, so u =
    // (r/R)^2, r = Rsqrt(u)
    // https://stackoverflow.com/questions/5837572/generate-a-random-point-within-a-circle-uniformly
    Real u1 = randomReal(mt), u2 = randomReal(mt);
    Real sinPhi = sqrt(1 - u1), theta = 2 * M_PI * u2;

    // then we shall project it onto an sphere, then we get the cosine distribution
    // normally suppose the normal vector point towards z-axis, then base on spherical
    // coordinate <1, theta, phi> <=> <rcos(theta)rsin(phi), rsin(theta)rsin(phi), rcos(phi)>
    // where rsin(phi) is sqrt(1 - r^2), or sqrt(1 - u1)
    Vector local = {rcos(theta) * sinPhi, rsin(theta) * sinPhi, sqrt(u1)};
    return local;
}

static inline Vector toWorld(Vector local, Vector N) {
    Vector helper = (rabs(N.x) > 0.9) ? ((Vector){0, 1, 0}) : ((Vector){1, 0, 0});
    Vector T = unitVector(vecCross(helper, N));
    Vector B = vecCross(N, T);
    // project the unturned to turned
    Vector nxtDirection = (Vector){local.x * T.x + local.y * B.x + local.z * N.x,
                                   local.x * T.y + local.y * B.y + local.z * N.y,
                                   local.x * T.z + local.y * B.z + local.z * N.z};
    return nxtDirection;
}

typedef struct {
    CollideResult firstCollideObjectResult;
    Object *firstCollideObject;
} SingleRayCollideObjectResult;

static inline SingleRayCollideObjectResult singleRayCollideObject(Object **objects,
                                                                  ReversedRay *ray) {
    // test the first object to touch, ignore light source
    int objectsCount = getSize(objects);

    CollideResult firstCollideObjectResult = {false, INFINITY};
    Object *firstCollideObject = NULL;
    for (int i = 0; i < objectsCount; i++) {
        CollideResult collideResult = collideObject(objects[i], ray);
        if (!collideResult.collide) continue;
        if (collideResult.distance <= firstCollideObjectResult.distance) {
            firstCollideObjectResult = collideResult;
            firstCollideObject = objects[i];
        }
    }

    return (SingleRayCollideObjectResult){firstCollideObjectResult, firstCollideObject};
}

typedef struct {
    CollideLightSourceResult firstCollideSourceResult;
    LightSource *firstCollideSource;
} SingleRayCollideSourceResult;

static inline SingleRayCollideSourceResult singleRayCollideSource(LightSource **sources,
                                                                  ReversedRay *ray) {
    int sourcesCount = getSize(sources);
    // then we do the same on light source
    CollideLightSourceResult firstCollideSourceResult = {false, INFINITY, 0};
    LightSource *firstCollideSource = NULL;
    for (int i = 0; i < sourcesCount; i++) {
        CollideLightSourceResult collideResult = collideLightSource(sources[i], ray);
        if (!collideResult.collide) continue;
        if (collideResult.distance <= firstCollideSourceResult.distance) {
            firstCollideSourceResult = collideResult;
            firstCollideSource = sources[i];
        }
    }
    return (SingleRayCollideSourceResult){firstCollideSourceResult, firstCollideSource};
}

static inline Vector getRefractionDirection(Real n1R, Real n2R, Real cosTheta1, Vector I,
                                            Vector N) {
    /*
     * We simply assume that
     * >> I = <rsin(Theta1), -rcos(Theta1)>
     * >> O = <rsin(Theta2), -rcos(Theta2)>
     * >> N = <0, 1>
     *
     * Since there exist a simple rotating Jacobian that rotate these vectors to world,
     * we simply use these simplified vectors to do linear combinations and the results won't
     * change
     *
     * With snell's law, n1 rsin(Theta1) = n2 rsin(Theta2)
     *
     * Hence,
     * >> (n1/n2)rsin(Theta1) = rsin(Theta2)
     * we take relative refractivity as eta
     * >> eta * rsin(Theta1) = rsin(Theta2)
     * >> eta^2(1 - cos^2(Theta1)) = (1 - cos^2(Theta2))
     * >> sqrt(1 - eta^2(1 - cos^2(Theta1))) = rcos(Theta2)
     *
     * We first do the cosine part
     * >> -sqrt(1 - eta^2(1 - cos^2(Theta1))) N
     *
     * Then the sine part
     * >> eta * I + eta * rcos(Theta1) N
     * Sum up the reault
     * >> eta * I + (eta * rcos(Theta1) - sqrt(1 - eta^2(1 - cos^2(Theta1))))N
     */
    Real eta = n1R / n2R;
    Real eta2 = eta * eta;
    Real sin2Theta1 = fmax(1 - cosTheta1 * cosTheta1, 0);
    Real coefficientOfSecondPart = eta * cosTheta1 - sqrt(fmax(0, 1 - eta2 * sin2Theta1));
    Vector O = vecAdd(vecMultiply(eta, I), vecMultiply(coefficientOfSecondPart, N));
    return O;
}

static inline Real getGeometryTermDenom(Real NdotL, Real NdotV, Real alpha) {
    Real alpha2 = alpha * alpha;
    Real NdotV2 = NdotV * NdotV;
    Real NdotL2 = NdotL * NdotL;
    return (NdotL * sqrt(alpha2 + (1 - alpha2) * NdotV2) +
            NdotV * sqrt(alpha2 + (1 - alpha2) * NdotL2));
}

static inline Real getGrometryTerm(Real NdotV, Real NdotL, Real alpha) {
    Real absNdotV = rabs(NdotV);
    Real absNdotL = rabs(NdotL);
    return 2 * absNdotV * absNdotL / getGeometryTermDenom(absNdotL, absNdotV, alpha);
}

typedef struct {
    Real scalingFactor;
    ReversedRay nxtRay;
} NextSimulationData;

// Sample through BRDF
static inline NextSimulationData sampleBRDF(Vector m, Vector v, Vector N, Real alpha, Real F,
                                            Point collidePoint, ReversedRay currentRay) {
    Vector l = vecMinus(vecMultiply(2 * vecDot(v, m), m), v); // direction of reflection
    // Cook-Torrance's BRDF
    // calculate V * (v dot h) / (n dot h) part = DG / PDF
    Real NdotV = vecDot(N, v);
    Real NdotL = vecDot(N, l);
    Real NdotM = vecDot(N, m);
    Real MdotV = vecDot(m, v);

    static const Real error = 1e-6;

    // light goes below the surface is invalid
    if (NdotV <= error || NdotL <= error || NdotM <= error) {
        return (NextSimulationData){0};
    }

    // calculating 4 * Fterm * Vterm * (N dot L) * (V dot H) / (N dot H)
    // Using highly-correlated Smith's model
    Real denom = getGeometryTermDenom(NdotL, NdotV, alpha);
    Real scalingFactor = (2.0 * NdotL * MdotV) / (denom * NdotM);

    Vector nxtDirection = l;
    Point nxtStart = fixedNextStart(collidePoint, N, false);
    ReversedRay nxtRay = getRay(nxtStart, nxtDirection, currentRay.wavelength);

    return (NextSimulationData){scalingFactor, nxtRay};
}

// similar to sample refraction, we have to switch to this branch instead of using sampleBRDF when
// the roughness (alpha) is to small, or the surface is too smooth. In such case the PDF of ggx
// sampling will approaches to infinity.
static inline NextSimulationData sampleSpecular(Vector N, Vector v, Real F, Point collidePoint,
                                                ReversedRay currentRay) {
    Vector l = vecMinus(vecMultiply(2 * vecDot(v, N), N), v);
    Vector nxtStart = fixedNextStart(collidePoint, N, false);
    ReversedRay nxtRay = getRay(nxtStart, l, currentRay.wavelength);

    return (NextSimulationData){F, nxtRay};
}

static inline NextSimulationData sampleBTDF(Vector N, Vector v, Vector m, Real alpha,
                                            Point collidePoint, ReversedRay ray, Complex n1,
                                            Complex n2) {
    // BTDF
    // calculate (v dot m)G / ((v dot n) (n dot m))
    Vector I = ray.direction;
    Real n1R = n1.R;
    Real n2R = n2.R;
    Real cosTheta1 = vecDot(m, v); // cosine of incident angle
    Vector l = getRefractionDirection(n1R, n2R, cosTheta1, I, m);
    Real NdotL = vecDot(N, l); // left
    Real NdotV = vecDot(N, v);
    Real NdotM = vecDot(N, m);

    // check if this refraction is valid
    // 1. n dot l must <= 0 to ensure the reversed ray goes through the macrosurface
    // 2. n dot v must >= 0 to ensure the reversed ray comes from the right direction
    // 3. n dot m must >= 0 to ensure the microsurface sampled correctly
    if (NdotL >= -error || NdotV <= error || NdotM <= error) {
        return (NextSimulationData){0};
    }

    Real MdotL = vecDot(m, l);
    Real MdotV = vecDot(m, v);

    // check the ray actually goes through the microsurface
    if (MdotL * MdotV >= -error) {
        return (NextSimulationData){0};
    }

    // N dot L is natrally negative, and we shall use its absolute value in calculation
    Real absNdotL = rabs(NdotL);
    Real absMdotL = rabs(MdotL);
    Real absMdotV = rabs(MdotV);

    // calculate Geometry term
    Real GTermDenom = getGeometryTermDenom(absNdotL, NdotV, alpha);
    Real GTerm = (2 * NdotV * absNdotL) / GTermDenom;

    // final factor is (m dot v)G(1-F) / (n dot v * n dot m), however we have times 1 - F by simply
    // throwing a dice that has opportunity of 1 - F to get in to this branch. So, there is no need
    // to multiply by 1 - F again.
    Real eta = n1R / n2R;
    Real scalingFactor = eta * eta * (absMdotV * GTerm) / (NdotV * NdotM);
    Point nxtStart = fixedNextStart(collidePoint, N, true);
    ReversedRay nxtRay = getRay(nxtStart, l, ray.wavelength);

    return (NextSimulationData){scalingFactor, nxtRay};
}

// sample the next direction (nxtRay) and scaling factor through regular refraction to avoid
// accuracy error in ggx and BTDF
// if roughness is exactly 0 then pdf at N = M will approaches infinity, which is the main reason we
// cannot use that method
static inline NextSimulationData sampleRefraction(Vector N, Vector v, Point collidePoint,
                                                  ReversedRay ray, Complex n1, Complex n2) {
    // get the direction of refraction ray
    Real n1R = n1.R;
    Real n2R = n2.R;
    Real cosTheta1 = vecDot(v, N);
    Vector I = ray.direction;
    Vector l = getRefractionDirection(n1R, n2R, cosTheta1, I, N);

    // we move a little bit from the surface so that the ray will not be captured by the same object
    // or surface again immediately
    Point nxtStart = fixedNextStart(collidePoint, N, true);
    ReversedRay nxtRay = getRay(nxtStart, l, ray.wavelength);

    Real eta = n1R / n2R;

    // the spherical angle of the refraction is "compressed", so we have to compensate that by
    // giving a factor eta^2
    return (NextSimulationData){eta * eta, nxtRay};
}

// calculate the F term
static inline Real getFresnelTerm(Complex n1, Complex n2, Real cosTheta) {
    Real cosThetaForF;

    // check total internal reflection
    // if this is triggered, 100% goes into branch reflection
    // metal is neglected since metals won't refract or diffuse
    if (n1.R > n2.R) {
        // TIR need n1 sinTheta >= n2
        // or squared n1^2 sin^2Theta >= n2^2
        Real n1R2 = n1.R * n1.R;
        Real n2R2 = n2.R * n2.R;
        Real sin2Theta = 1 - cosTheta * cosTheta;
        if (n1R2 * sin2Theta >= n2R2) {
            cosThetaForF = 0;
        }
        else {
            // use angle of refraction instead
            // ignore the imaginary part of n since metal does not have refraction
            // since n1 sinTheta1 = n2 sinTheta2 we have sinTheta2 = n1/n2 sinTheta1
            Real sin2Theta2 = n1R2 / n2R2 * sin2Theta;
            cosThetaForF = sqrt(fmax(0, 1 - sin2Theta2));
        }
    }
    else {
        cosThetaForF = cosTheta;
    }

    // Schlick's approximation
    // F = F0 + (1 - F0) * y^5
    Real x = cplxModulus(cplxDivide(cplxMinus(n1, n2), cplxAdd(n1, n2)));
    Real F0 = x * x;
    Real y = 1 - cosThetaForF;
    Real y2 = y * y;
    Real y5 = y2 * y2 * y;
    Real F = F0 + (1 - F0) * y5;

    return F;
}

static const Real moveEpsilon = 1e-4;

static inline NextSimulationData sampleDiffusion(MT19937 *mt, Vector N, ReversedRay ray,
                                                 Point collidePoint, Object *firstCollideObject) {

    Vector local = cosineVectorSample(mt);
    Vector nxtDirection = toWorld(local, N);
    Point nxtStart = vecAdd(collidePoint, vecMultiply(moveEpsilon, N));
    ReversedRay nxtRay = getRay(nxtStart, nxtDirection, ray.wavelength);

    // after diffusion the light should have the color of the material
    Real baseColor = getBaseColor(firstCollideObject, ray.wavelength);
    Real scalingFactor = baseColor;

    return (NextSimulationData){scalingFactor, nxtRay};
}

static inline NextSimulationData sampleBSDF(CollideResult *firstCollideObjectResult,
                                            Object *firstCollideObject, Real *extinctionFactor,
                                            ReversedRay ray, MT19937 *mt) {
    static const NextSimulationData notCollideData = {0};

    Real travelingDistance = firstCollideObjectResult->distance;
    Real scalingFactor = rexp(-*extinctionFactor * travelingDistance);
    Vector d = ray.direction;
    Point collidePoint = vecAdd(ray.start, vecMultiply(travelingDistance, d));
    // Now the normal vector is still point toward outside
    Vector N = firstCollideObjectResult->normalVector;

    // n1 symbolizes the refractivity of the material the reversed ray is currently in
    // n2 symbolizes the refractivity of material on the other side
    Complex n1, n2;
    if (vecDot(N, d) > 0) {
        // N vecDot d < 0 implys the ray is now in the object
        // n1 = inside, n2 = outside, since we goes from inside to outside
        N = (Vector){-N.x, -N.y, -N.z};
        n1 = getRefractiveIndexInside(firstCollideObject, ray.wavelength);
        n2 = getRefractiveIndexOutside(firstCollideObject, ray.wavelength);
    }
    else {
        n1 = getRefractiveIndexOutside(firstCollideObject, ray.wavelength);
        n2 = getRefractiveIndexInside(firstCollideObject, ray.wavelength);
    }
    // after the operations above, the normal vector should point on the side where reversed
    // rays come from

    Vector v = vecNegative(ray.direction); // direction from collide point to observer
    Real roughness = getRoughness(firstCollideObject);
    // By Disney's guidelines we square the roughness here to get alpha
    Real alpha = roughness * roughness;

    Vector m = roughness >= error ? toWorld(GGXVectorSample(mt, alpha), N) : N;
    // calculate F part
    Real VdotM = vecDot(v, m);
    // v dot m must >= 0, or else the sample fails
    // this should not happen (or at least should not happen often) since the sample itself
    // should not sample a microfacet that is inversed
    if (VdotM <= 0) return notCollideData;
    Real cosTheta = VdotM;

    // by Fresnel's Equation, we concluded that F should reflect and 1-F should goes through the
    // surface and so refraction or diffusion
    bool metallic = getmetallic(firstCollideObject);
    Real F = getFresnelTerm(n1, n2, cosTheta);

    // here we implement BSDF
    NextSimulationData nxt;

    if (metallic || randomReal(mt) <= F) {
        if (roughness < error) {
            nxt = sampleSpecular(N, v, F, collidePoint, ray);
        }
        else {
            nxt = sampleBRDF(m, v, N, alpha, F, collidePoint, ray);
        }

        if (nxt.scalingFactor == 0) return notCollideData;

        scalingFactor *= nxt.scalingFactor;
        // since metallic texture won't go into the 1 - F branch so we do have to * F
        if (metallic) scalingFactor *= F;
    }
    else {
        // the left branch is 1 - F
        bool transparent = getTransparent(firstCollideObject);

        if (transparent) {
            // Note that we have returned a value in reflection branch so the proabability of
            // this branch is 1 - F There is no need to * (1 - F) in the result

            if (roughness < error) {
                nxt = sampleRefraction(N, v, collidePoint, ray, n1, n2);
            }
            else {
                nxt = sampleBTDF(N, v, m, alpha, collidePoint, ray, n1, n2);
            }

            if (nxt.scalingFactor == 0) return notCollideData;

            scalingFactor *= nxt.scalingFactor;

            // calculate the extinction factor for next ray
            // Beer-Lembert's Law
            // 4pi k / lambda and convert to (10^(-4)m^(-1)) or 0.1mm^(-1) as unit
            *extinctionFactor = 4 * M_PI * n2.I / ray.wavelength * 1e5;
        }

        else {
            // Branch Diffusion, only non-transparent materials need to do this
            nxt = sampleDiffusion(mt, N, ray, collidePoint, firstCollideObject);
            scalingFactor *= nxt.scalingFactor;
        }
    }

    return (NextSimulationData){scalingFactor, nxt.nxtRay};
}

static inline Real getNormalDistributionTerm(Real NdotM, Real alpha) {
    Real alpha2 = alpha * alpha;
    Real NdotM2 = NdotM * NdotM;
    Real denomPart = (NdotM2 * (alpha2 - 1) + 1);
    return alpha2 / (M_PI * denomPart * denomPart);
}

/*
 * A function that helps distinguish if this event is reflection or refraction, or, simply invalid
 * Return 1 for reflection, 2 for refraction, 0 for invalid
 */
static inline int distinguishReflectionOrRefraction(Vector l, Vector v, Vector N) {
    if (vecDot(N, l) < 0) {
        N = vecNegative(N);
    }
    Real NdotL = vecDot(N, l);
    Real NdotV = vecDot(N, v);
    if (NdotL > error && NdotV > error) {
        return 1; // reflection
    }
    else if (NdotL >= -error || NdotV <= error) {
        return 0; // invalid if the light and the surface is near parallel
    }
    return 2; // refraction
}

static inline Real BSDF(Complex n1, Complex n2, Vector l, Vector v, Vector N, Real alpha,
                        Object *object, Real lambda, bool transmission) {
    bool metallic = getmetallic(object);
    bool transparent = getTransparent(object);
    // we have to derive m by l, v
    // first we need to check if this is reflection (or diffusion) or refraction
    if (vecDot(N, l) < 0) {
        N = vecNegative(N);
    }
    Vector m;
    Real NdotL = vecDot(N, l);
    Real NdotV = vecDot(N, v);
    Real NdotM, MdotL, MdotV;

    if (!transmission) { // this means the light did not go through the surface
        if (NdotL * NdotV <= error) return 0;
        m = unitVector(vecAdd(l, v)); // m is parallel to l + v
        NdotM = vecDot(N, m);
        MdotL = vecDot(m, l);
        MdotV = vecDot(m, v);

        if (MdotL * MdotV <= error || NdotM <= error) {
            return 0;
        }
    }
    else {
        if (NdotL * NdotV >= -error) return 0;
        Vector n1v = vecMultiply(n1.R, v);
        Vector n2l = vecMultiply(n2.R, l);
        m = unitVector(vecAdd(n1v, n2l));
        if (vecDot(N, m) < 0) {
            m = vecNegative(m);
        }
        NdotM = vecDot(N, m);
        MdotL = vecDot(m, l);
        MdotV = vecDot(m, v);

        if (MdotL * MdotV >= -error || NdotM <= error) {
            return 0;
        }
    }

    // F term (TIR is checked so no need to check again here)
    Real F = getFresnelTerm(n1, n2, rabs(MdotV));

    // G term
    Real G = getGrometryTerm(NdotV, NdotL, alpha);

    // D term
    Real D = getNormalDistributionTerm(NdotM, alpha);

    if (transmission) {
        // this means the ray must in the BTDF branch
        Real firstTerm = rabs(MdotL * MdotV / (NdotL * NdotV));
        Real secondTermDenom = n2.R * MdotL + n1.R * MdotV;
        Real secondTerm = (n1.R * n1.R * D * (1 - F) * G) / (secondTermDenom * secondTermDenom);
        Real BTDF = firstTerm * secondTerm;
        return BTDF;
    }

    Real BRDF = D * F * G / rabs(4 * (NdotL) * (NdotV));

    if (!metallic && !transparent) {
        // the light may comes from BRDF or diffusion branch
        Real diffusion = getBaseColor(object, lambda) / M_PI;
        return BRDF + diffusion * (1 - F);
    }

    return BRDF;
}

static inline bool visibilityTest(ReversedRay *shadowRay, Real distance, Object **objects,
                                  LightSource **sources, LightSource *src) {
    // visibility test
    for (int i = 0; i < getSize(objects); i++) {
        // any object on the path of light will block the ray
        if (collideObject(objects[i], shadowRay).distance < distance - error) {
            return false;
        }
    }

    for (int i = 0; i < getSize(sources); i++) {
        if (sources[i] == src) {
            continue;
        }
        if (collideLightSource(sources[i], shadowRay).distance < distance - error) {
            return false;
        }
    }
    return true;
}

static inline LightIntensity sampleNEE(Complex n1, Complex n2,
                                       CollideResult *firstCollideObjectResult,
                                       Object *firstCollideObject, Point collidePoint,
                                       Object **objects, LightSource **sources, MT19937 *mt,
                                       ReversedRay ray, Real *pLight) {
    int sourcesCount = getSize(sources);
    Real scalingFactor = sourcesCount;
    // randomly choose a light source in sources
    LightSource *src = sources[randomInt(mt, sourcesCount)];
    LightIntensity intensity = src->intensity(ray.wavelength);

    Point point = randomPoint(src->shape, mt);
    Real randomPointPDF = PDFofPoint(src->shape, point);
    Vector lightN = NormalVectorOfPoint(src->shape, point);

    Vector v = vecNegative(ray.direction);
    Vector l = unitVector(vecMinus(point, collidePoint));
    // in BSDF and distinguishReclectionOrRefraction the functions will flip the normal vector based
    // on (N dot l) so no need to flip here
    Vector N = firstCollideObjectResult->normalVector;
    Real roughness = getRoughness(firstCollideObject);
    Real alpha = roughness * roughness;

    int distinguishRes = distinguishReflectionOrRefraction(l, v, N);
    if (distinguishRes == 0) { // invalid
        return 0;
    }
    bool transmission = distinguishRes - 1; // the light will transmission or not

    Point nxtStart = fixedNextStart(collidePoint, N, transmission);
    l = unitVector(vecMinus(point, nxtStart));

    Real BSDFTerm = BSDF(n1, n2, l, v, N, alpha, firstCollideObject, ray.wavelength, transmission);

    ReversedRay shadowRay = getRay(nxtStart, l, ray.wavelength);
    Real distance = lengthOf(vecMinus(point, nxtStart));
    bool reach = visibilityTest(&shadowRay, distance, objects, sources, src);
    if (!reach) return 0;
    // convert dA to d(omega) so that p(d(omega)) = p(dA) * jacobian
    Real jacobian = distance * distance / rabs(vecDot(lightN, l)); // |x|^2 / cos(theta)
    *pLight = randomPointPDF * jacobian / scalingFactor; // prevent multiple times of calculation
    return intensity * BSDFTerm / (*pLight) * rabs(vecDot(N, l));
}

static inline Real pNEE(ReversedRay *shadowRay, LightSource **sources,
                        SingleRayCollideSourceResult *res) {
    int sourcesSize = getSize(sources);
    // PDF to select the specific source PDF = 1 / n
    Real selectSourceTerm = 1.0 / (Real)sourcesSize;

    LightSource *collideSource = res->firstCollideSource;
    if (collideSource == NULL) return 0;

    Real distance = res->firstCollideSourceResult.distance;
    Point collidePoint = vecAdd(shadowRay->start, vecMultiply(distance, shadowRay->direction));

    Vector lightN = NormalVectorOfPoint(collideSource->shape, collidePoint);
    Real cosineTerm = rabs(vecDot(lightN, shadowRay->direction));
    if (cosineTerm < error) return 0;

    Real surfaceTerm = PDFofPoint(collideSource->shape, collidePoint);

    return selectSourceTerm * distance * distance * surfaceTerm / cosineTerm;
}

static inline Real pBSDF(Complex n1, Complex n2, Object *collideObject, Vector l, Vector v,
                         Vector N) {

    int distinguishRes = distinguishReflectionOrRefraction(l, v, N);
    if (distinguishRes == 0) return 0; // invalid
    bool transmission = distinguishRes - 1;
    Vector m;
    if (transmission) {
        if (!getTransparent(collideObject)) {
            // if the object is not transparent, then tranmission is not possible
            return 0;
        }
        Vector n1v = vecMultiply(n1.R, v);
        Vector n2l = vecMultiply(n2.R, l);
        m = unitVector(vecAdd(n1v, n2l));
    }
    else {
        m = unitVector(vecAdd(v, l));
    }
    Real MdotL = vecDot(m, l);
    Real MdotV = vecDot(m, v);
    Real NdotM = vecDot(m, N);
    Real roughness = getRoughness(collideObject);
    Real alpha = roughness * roughness;
    Real D = getNormalDistributionTerm(NdotM, alpha);
    Real F = getFresnelTerm(n1, n2, rabs(MdotV));

    if (transmission) {
        // check if the light goes through the microfacet
        if (MdotL * MdotV > -error || NdotM < error) {
            // if it did not, then reject
            return 0;
        }
        return D 
    }
    else {
        // check if the light goes through the microfacet
        if (MdotL * MdotV < error || NdotM < error) {
            // if it did, then reject
            return 0;
        }
    }
}

static LightIntensity simulateSingleRay(ReversedRay ray, Object **objects, LightSource **sources,
                                        MT19937 *mt, int maxDepth, Real extinctionFactor,
                                        bool directLight) {
    Real res = 0;
    Real BSDFFactor = 1;

    for (int i = 0; i < maxDepth; i++) {
        // first we see if this light first touches a light source
        // to attain that, we need to simulate both collide with sources and objects to decide which
        // is touched first.

        SingleRayCollideObjectResult collideObjectResult = singleRayCollideObject(objects, &ray);
        CollideResult firstCollideObjectResult = collideObjectResult.firstCollideObjectResult;
        Object *firstCollideObject = collideObjectResult.firstCollideObject;

        SingleRayCollideSourceResult collideSourceResult = singleRayCollideSource(sources, &ray);
        CollideLightSourceResult firstCollideSourceResult =
            collideSourceResult.firstCollideSourceResult;
        LightSource *firstCollideSource = collideSourceResult.firstCollideSource;

        // case 1: collide with nothing.
        if (firstCollideObject == NULL && firstCollideSource == NULL) {
            return 0.0;
        }

        // case 2: first collide with source, so we return the intensity of light source directly
        if (firstCollideSourceResult.distance <= firstCollideObjectResult.distance &&
            firstCollideSource != NULL) {
            Real travelingDistance = firstCollideSourceResult.distance;
            BSDFFactor *= rexp(-extinctionFactor * travelingDistance);
            res += BSDFFactor * firstCollideSource->intensity(ray.wavelength);
            break;
        }

        // case 3: we collide with an object first
        NextSimulationData BSDFnxt =
            sampleBSDF(&firstCollideObjectResult, firstCollideObject, &extinctionFactor, ray, mt);

        BSDFFactor *= BSDFnxt.scalingFactor;
        ray = BSDFnxt.nxtRay;
        directLight = false;
    }

    return res;
}

#if defined(ADAPTIVE_SAMPLING)
// form a touch map before actual rendering
// if a reversed ray touches an object then it's marked as true (touched)
// otherwise it is marked as false
static bool *newTouchMap(Camera *camera, VirtualScreen *vs, Object **objects,
                         LightSource **sources) {
    int width = vs->width;
    int height = vs->height;
    bool *touch = calloc(width * height, sizeof(*touch));

    for (int i = 0; i < width * height; i++) {
        int x = getX(vs, i), y = getY(vs, i);
        // it won't reflect so a wavelength is fine
        ReversedRay initRay = getInitRay(camera, (double)x, (double)y, 0);
        SingleRayCollideResult result = singleRayCollide(objects, sources, initRay);
        if (result.firstCollideObject != NULL || result.firstCollideSource != NULL) {
            touch[i] = true;
        }
    }

    return touch;
}
#endif

static inline bool inBoundary(int var, int limit) { return var >= 0 && var < limit; }

#if defined(ADAPTIVE_SAMPLING)
void setThreshold(Camera *camera, VirtualScreen *vs, Object **objects, LightSource **sources,
                  int sampleThreshold) {
    bool *touchMap = newTouchMap(camera, vs, objects, sources);
    int width = vs->width;
    int height = vs->height;
    for (int i = 0; i < width * height; i++) {
        // to make antialiasing work, we cannot block the pixels around the pixels that touches
        // object
        // consider 3 * 3 pixels around a pixel, if one of them touches object then we set a normal
        // threshold for them
        int x = getX(vs, i), y = getY(vs, i);

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                // neighbor x, y
                int nx = x + dx;
                int ny = y + dy;
                if (!inBoundary(nx, width) || !inBoundary(ny, height)) continue;
                int ni = getIndex(vs, nx, ny);
                // if neighbor touches objects then set threshold to default value
                if (touchMap[ni]) {
                    vs->threshold[i] = sampleThreshold;
                }
            }
        }
    }
    // we use "new"TouchMap so we have to free here
    free(touchMap);
}

bool isThresholdAchived(VirtualScreen *vs, int x, int y) {
    int i = getIndex(vs, x, y);
    return vs->renderCount[i] >= vs->threshold[i];
}
#endif

void renderPixel(Camera *camera, VirtualScreen *vs, int x, int y, Object **objects,
                 LightSource **sources, MT19937 *mt, int maxDepth) {
    if (sources == NULL) return;

    // anti-aliasing by giving the init point a little shift
    Real fixedX = (Real)x + randomReal(mt);
    Real fixedY = (Real)y + randomReal(mt);
    /*
     * Some basic idea about this function is to generate a reversed ray start from the camera and
     * goes through a pixel, then see if it collide with an object. If it collides with an object
     * then do something consequently.
     */

    // randomly choose a wavelength from the interval
    Real intervalSize = endWavelength - startWavelength;
    ReversedRay initRay;
    LightIntensity intensityResult;
    Real randomWavelength = randomReal(mt) * intervalSize + startWavelength;

    initRay = getInitRay(camera, fixedX, fixedY, randomWavelength);

    intensityResult = simulateSingleRay(initRay, objects, sources, mt, maxDepth, 0, true);

    if (!isfinite(intensityResult)) return;

    // here we times by interval size because of pdf
    ColorXYZ colorResult =
        colorComponent(intensityResult * intervalSize, wavelengthToXYZ(randomWavelength));

    updateScreen(vs, x, y, colorResult);
    return;
}
