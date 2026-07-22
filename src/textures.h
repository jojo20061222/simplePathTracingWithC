#ifndef TEXTURES_H
#define TEXTURES_H

typedef struct {
    double (*baseColor)(double lambda);
    double (*refractiveIndexOutSide)(double lambda);
    double (*refractiveIndexInside)
} Texture;

Texture newTexture(double (*baseColor)(double), double (*refrectiveIndex)(double));

double pureWhite(double lambda);

double plastic(double lambda);

double red(double lambda);

#endif