#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H
#include "base.h"
#include "object.h"
#include "shapes.h"
#include <stdbool.h>

typedef struct {
    Shape *shape;
    LightIntensity (*intensity)(double waveLength);
} LightSource;

typedef struct {
    bool collide;
    double t;
    LightIntensity intensity;
} CollideLightSourceResult;

typedef struct {
    double X, Y, Z;
} ColorXYZ;

void initLightIntensity();

ColorXYZ wavelengthToXYZ(double waveLength);

ScreenColor XYZtoRGB(ColorXYZ xyz);

static inline ColorXYZ colorAdd(ColorXYZ a, ColorXYZ b) {
    return (ColorXYZ){a.X + b.X, a.Y + b.Y, a.Z + b.Z};
}

static inline ColorXYZ colorComponent(double c, ColorXYZ color) {
    return (ColorXYZ){c * color.X, c * color.Y, c * color.Z};
}

CollideLightSourceResult collideLightSource(const LightSource *source, const ReversedRay *ray);

double getBlackbodyIntensity(double T, double waveLength);

double tungstenLampBase(double waveLength);

double sunlightBase(double waveLength);

double whiteLightBase(double wavelength);

typedef struct {
    Shape shape;
    Vector direction;
} ParallelLightShape;

ParallelLightShape getParallelLightShape(Vector direction);

#endif