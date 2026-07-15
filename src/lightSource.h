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
} LightSource;

#endif