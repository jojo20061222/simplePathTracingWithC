#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H
#include "base.h"
#include "object.h"
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
    return (LightIntensity){a.R + b.R, a.G + b.G, a.B + b.B};
}

static inline LightIntensity irradiate(LightIntensity intensity, ObjectColor color) {
    return (LightIntensity){intensity.R * color.R, intensity.G * color.G, intensity.B * color.B};
}

static inline ScreenColor intensityToColor(LightIntensity intensity) {
    return (ScreenColor){(uint8_t)(255 * intensity.R / (1 + intensity.R)),
                         (uint8_t)(255 * intensity.G / (1 + intensity.G)),
                         (uint8_t)(255 * intensity.B / (1 + intensity.B))};
}

#endif