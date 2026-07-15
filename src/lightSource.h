#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H
#include "base.h"
#include "shapes.h"

typedef struct {
    double R, G, B;
} LightIntensity;

typedef struct {
    Shape *shape;
    LightIntensity intensity;
    Point center;
} LightSource;

static inline LightIntensity plusLightIntensity(LightIntensity a, LightIntensity b) {
    return (LightIntensity){(a.R + b.R) / (1 + a.R + b.R),
                            (a.G + b.G) / (1 + a.G + b.G),
                            (a.B + b.B) / (1 + a.B + b.B)};
}

#endif