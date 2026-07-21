#include "textures.h"

Texture newTexture(double (*reflectivity)(double), double (*refractiveIndex)(double)) {
    return (Texture){reflectivity, refractiveIndex};
}

double pureWhite(double _) { return 0.5; }

double plastic(double _) { return 1.5; }