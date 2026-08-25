#ifndef TEXTURES_H
#define TEXTURES_H
#include "base.h"
#include "random.h"
#include <stdbool.h>

typedef struct Texture Texture;

typedef struct Interface Interface;

// wrappers for Interface
void destroyInterface(Interface *interface);
Interface *newInterface(Texture *in, Texture *out);
void destroyTexture(Texture *tex);
Real getInterfaceRoughness(Interface *interface);
bool isInterfaceInsideHalfTransparent(Interface *interface);
bool isInterfaceOutsideHalfTransparent(Interface *interface);
bool isInterfaceInsideFullyTransparent(Interface *interface);
bool isInterfaceOutsideFullyTransparent(Interface *interface);
bool isInterfaceInsideTransparent(Interface *interface);
bool isInterfaceOutsideTransparent(Interface *interface);
bool isInterfaceInsideMetallic(Interface *interface);
bool isInterfaceOutsideMetallic(Interface *interface);
Complex getInterfaceInsideRefractiveIndex(Interface *interface, Real lambda);
Complex getInterfaceOutsideRefractiveIndex(Interface *interface, Real lambda);
Vector sampleInterfaceInsideNextDirection(Interface *interface, Vector currentDirection,
                                          MT19937 *mt);
Vector sampleInterfaceOutsideNextDirection(Interface *interface, Vector currentDirection,
                                           MT19937 *mt);
Real getInterfaceInsideSigmaS(Interface *interface, Real lambda);
Real getInterfaceOutsideSigmaS(Interface *interface, Real lambda);
Real getInterfaceInsideBaseColor(Interface *interface, Real lambda);
Real getInterfaceOutsideBaseColor(Interface *interface, Real lambda);

// read refractivity data
void initTexture();

// new Textures
Texture *newOpaqueDieletric(Complex (*refractiveIndex)(Texture *, Real), Real (*baseColor)(Real));
Texture *newTransparentDieletric(Complex (*refractiveIndex)(Texture *, Real), Real concentration);
Texture *newMetal(Complex (*refractiveIndex)(Texture *, Real));
Texture *newHalfTransparentDieletric(Complex (*refractiveIndex)(Texture *, Real),
                                     Real (*sigmaS)(Real),
                                     Vector (*sampleNextDirection)(Vector, MT19937 *));

// base colors
Real pureWhite(Real lambda);
Real pureBlack(Real lambda);
Real red(Real lambda);

// refractivities
Complex getPlasticRefractivity(Texture *, Real);
Complex getAirRefractivity(Texture *, Real);
Complex getSilverRefractivity(Texture *, Real lambda);
Complex getGoldRefractivity(Texture *, Real lambda);
Complex getCopperRefractivity(Texture *, Real lambda);
Complex getIronRefractivity(Texture *, Real lambda);
Complex getDiamondRefractivity(Texture *, Real lambda);

// base materials
Texture *newAir();
Texture *newGold(Real roughness);
Texture *newSilver(Real roughness);
Texture *newCopper(Real roughness);
Texture *newIron(Real roughness);
Texture *newGlass(Real roughness);
Texture *newDiamond(Real roughness);
Texture *newSapphire(Real roughness, Real concentration);

// base h-G sample
Vector sampleHenyeyGreenstein(Vector input, MT19937 *mt, Real g);

// Wrappers for textures
Real getInterfaceRoughness(Interface *interface);

#endif