#ifndef TEXTURES_H
#define TEXTURES_H
#include "base.h"
#include <stdbool.h>

typedef struct {
    double (*baseColor)(double lambda);
    Complex (*refractiveIndexOutside)(double lambda);
    Complex (*refractiveIndexInside)(double lambda);
    bool transparent;
    bool metalic;
} Texture;

Texture newTexture(double (*baseColor)(double), Complex (*refractiveIndexInside)(double),
                   Complex (*refractiveIndexOutside)(double), bool transparent, bool metalic);

double pureWhite(double lambda);

Complex plastic(double lambda);

Complex air(double lambda);

double red(double lambda);

void initTexture();

#endif