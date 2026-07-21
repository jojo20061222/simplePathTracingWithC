#ifndef TEXTURES_H
#define TEXTURES_H

typedef struct {
    double (*reflectivity)(double lambda);
    double (*refrectiveIndex)(double lambda);
} Texture;

Texture newTexture(double (*reflectivity)(double), double (*refrectiveIndex)(double));

double pureWhite(double lambda);

double plastic(double lambda);

#endif