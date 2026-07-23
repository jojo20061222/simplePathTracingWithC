#ifndef TEXTURES_H
#define TEXTURES_H
#include "base.h"
#include <stdbool.h>

typedef struct {
    double (*baseColor)(double lambda);
    Complex (*refractiveIndexInside)(double lambda);
    Complex (*refractiveIndexOutside)(double lambda);
    bool transparent;
    bool metalic;
} Texture;

Texture newTexture(double (*baseColor)(double), Complex (*refractiveIndexInside)(double),
                   Complex (*refractiveIndexOutside)(double), bool transparent, bool metalic);

void initTexture();

// base colors
double pureWhite(double lambda);
double red(double lambda);

// refractivities
Complex getPlasticRefractivity(double lambda);
Complex getAirRefractivity(double lambda);
Complex getSilverRefractivity(double lambda);
Complex getGoldRefractivity(double lambda);
Complex getCopperRefractivity(double lambda);
Complex getIronRefractivity(double lambda);

Texture getGold();
Texture getSilver();
Texture getCopper();
Texture getIron();

#endif