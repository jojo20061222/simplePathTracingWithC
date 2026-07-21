#include "textures.h"

Texture newTexture(double (*baseColor)(double), double (*refractiveIndex)(double)) {
    return (Texture){baseColor, refractiveIndex};
}

double pureWhite(double _) { return 0.5; }

double plastic(double _) { return 1.5; }