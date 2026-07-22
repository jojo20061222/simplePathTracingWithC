#ifndef TEXTURES_H
#define TEXTURES_H

typedef struct {
    double (*baseColor)(double lambda);
    double (*refractiveIndexOutside)(double lambda);
    double (*refractiveIndexInside)(double lambda);
} Texture;

Texture newTexture(double (*baseColor)(double), double (*refractiveIndexInside)(double), double (*refractiveIndexOutside)(double));

double pureWhite(double lambda);

double plastic(double lambda);

double air(double lambda);

double red(double lambda);

#endif