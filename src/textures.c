#include "textures.h"

Texture newTexture(double (*reflectivity)(double), double diffuseFactor) {
    return (Texture){reflectivity, diffuseFactor};
}

double pureWhite(double waveLength) { return 0.5; }