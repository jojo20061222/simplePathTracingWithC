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

ColorXYZ getXYZ(double waveLength);

ScreenColor XYZtoRGB(ColorXYZ xyz);

static inline ColorXYZ addColor(ColorXYZ a, ColorXYZ b) {
    return (ColorXYZ){a.X + b.X, a.Y + b.Y, a.Z + b.Z};
}

static inline ColorXYZ colorComponent(double c, ColorXYZ color) {
    return 
}

CollideLightSourceResult collideLightSource(const LightSource *source, const ReversedRay *ray);

#endif