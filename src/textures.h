#ifndef TEXTURES_H
#define TEXTURES_H
#include "base.h"
#include <stdbool.h>

typedef struct Texture Texture;

double getTextureBaseColor(Texture *texture, double lambda);
Complex getTextureRefractiveIndexInside(Texture *texture, double lambda);
Complex getTextureRefractiveIndexOutside(Texture *texture, double lambda);
bool getTextureMetalic(Texture *texture);
bool getTextureTransparent(Texture *texture);
double getTextureRoughness(Texture *texture);


Texture *newTexture(double (*baseColor)(double), Complex (*refractiveIndexInside)(double),
                   Complex (*refractiveIndexOutside)(double), bool transparent, bool metalic,
                   double roughness);

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
Complex getDiamondRefractivity(double lambda);

Texture *getGold(double roughness);
Texture *getSilver(double roughness);
Texture *getCopper(double roughness);
Texture *getIron(double roughness);
Texture *getGlass();
Texture *getDiamond();

#endif