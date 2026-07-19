#ifndef TEXTURES_H
#define TEXTURES_H

typedef struct {
    double (*reflectivity)(double waveLength);
    double diffuseFactor;
} Texture;

Texture newTexture(double (*reflectivity)(double), double diffuseFactor);

double pureWhite(double waveLength);

#endif