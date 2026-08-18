#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H
#include "base.h"
#include "object.h"
#include "shapes.h"
#include <stdbool.h>

typedef struct {
    Shape *shape;
    LightIntensity (*intensity)(Real wavelength);
} LightSource;

LightSource newLightSource(Shape *shape, LightIntensity (*intensity)(Real));

typedef struct {
    bool collide;
    Real distance;
    Vector normalVector;
    LightIntensity intensity;
} CollideLightSourceResult;

typedef struct {
    Real X, Y, Z;
} ColorXYZ;

void initLightIntensity();

ColorXYZ wavelengthToXYZ(Real wavelength);

ScreenColor XYZtoRGB(ColorXYZ xyz);

static inline ColorXYZ colorAdd(ColorXYZ a, ColorXYZ b) {
    return (ColorXYZ){a.X + b.X, a.Y + b.Y, a.Z + b.Z};
}

static inline ColorXYZ colorComponent(Real c, ColorXYZ color) {
    return (ColorXYZ){c * color.X, c * color.Y, c * color.Z};
}

CollideLightSourceResult collideLightSource(LightSource *source, ReversedRay *ray);

Real getBlackbodyIntensity(Real T, Real wavelength);

Real tungstenLampBase(Real wavelength);

Real sunlightBase(Real wavelength);

Real whiteLightBase(Real wavelength);

#endif