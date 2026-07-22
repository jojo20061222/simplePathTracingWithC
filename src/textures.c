#include "textures.h"
#include <math.h>

Texture newTexture(double (*baseColor)(double), double (*refractiveIndexInside)(double),
                   double (*refractiveIndexOutside)(double)) {
    return (Texture){baseColor, refractiveIndexInside, refractiveIndexOutside};
}

double pureWhite(double _) { return 0.5; }

double red(double lambda) { return 0.05 + 0.75 / (1 + exp(-0.06 * (lambda - 650))); }

double plastic(double _) { return 1.5; }

double air(double _) { return 1.0; };