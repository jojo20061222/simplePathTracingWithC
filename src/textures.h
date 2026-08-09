#ifndef TEXTURES_H
#define TEXTURES_H
#include "base.h"
#include <stdbool.h>

typedef struct Texture Texture;

Real getTextureBaseColor(Texture *texture, Real lambda);
Complex getTextureRefractiveIndexInside(Texture *texture, Real lambda);
Complex getTextureRefractiveIndexOutside(Texture *texture, Real lambda);
bool getTexturemetallic(Texture *texture);
bool getTextureTransparent(Texture *texture);
Real getTextureRoughness(Texture *texture);

Texture *newTexture(Real (*baseColor)(Real), Complex (*refractiveIndexInside)(Real),
                    Complex (*refractiveIndexOutside)(Real), bool transparent, bool metallic,
                    Real roughness);

void initTexture();

// base colors
Real pureWhite(Real lambda);
Real pureBlack(Real lambda);
Real red(Real lambda);

// refractivities
Complex getPlasticRefractivity(Real lambda);
Complex getAirRefractivity(Real lambda);
Complex getSilverRefractivity(Real lambda);
Complex getGoldRefractivity(Real lambda);
Complex getCopperRefractivity(Real lambda);
Complex getIronRefractivity(Real lambda);
Complex getDiamondRefractivity(Real lambda);

Texture *getGold(Real roughness);
Texture *getSilver(Real roughness);
Texture *getCopper(Real roughness);
Texture *getIron(Real roughness);
Texture *getGlass(Real roughness);
Texture *getDiamond(Real roughness);

#endif