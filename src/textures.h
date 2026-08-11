#ifndef TEXTURES_H
#define TEXTURES_H
#include "base.h"
#include <stdbool.h>

typedef struct Texture Texture;

// wrappers for the internal structure
Real getTextureBaseColor(Texture *texture, Real lambda);
Complex getTextureRefractiveIndexInside(Texture *texture, Real lambda);
Complex getTextureRefractiveIndexOutside(Texture *texture, Real lambda);
bool getTexturemetallic(Texture *texture);
bool getTextureTransparent(Texture *texture);
Real getTextureRoughness(Texture *texture);

void destroyTexture(Texture *tex);

void initTexture();

Texture *newOpaqueDieletric(Complex (*refractiveIndexInside)(Texture *, Real),
                            Complex (*refractiveIndexOutside)(Texture *, Real), Real roughness,
                            Real (*baseColor)(Real));

Texture *newTransparentDieletric(Complex (*refractiveIndexInside)(Texture *, Real),
                                 Complex (*refractiveIndexOutside)(Texture *, Real), Real roughness,
                                 Real concentraction);

Texture *newMetal(Complex (*refractiveIndexInside)(Texture *, Real),
                  Complex (*refractiveIndexOutside)(Texture *, Real), Real roughness);

// base colors
Real pureWhite(Real lambda);
Real pureBlack(Real lambda);
Real red(Real lambda);

// refractivities
Complex getPlasticRefractivity(Texture *, Real);
Complex getAirRefractivity(Texture *, Real);
Complex getSilverRefractivity(Texture *, Real lambda);
Complex getGoldRefractivity(Texture *, Real lambda);
Complex getCopperRefractivity(Texture*, Real lambda);
Complex getIronRefractivity(Texture *, Real lambda);
Complex getDiamondRefractivity(Texture *, Real lambda);

Texture *getGold(Real roughness);
Texture *getSilver(Real roughness);
Texture *getCopper(Real roughness);
Texture *getIron(Real roughness);
Texture *getGlass(Real roughness);
Texture *getDiamond(Real roughness);

#endif