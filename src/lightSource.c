#include "lightSource.h"
#include <math.h>
#include <stdio.h>
/*
CollideLightSourceResult collideLightSource(const LightSource *source, const ReversedRay *ray) {
    CollideResult result = source->shape->detectCollide(source->shape, ray);
    return (CollideLightSourceResult){result.collide, result.t, source->intensity};
}
    */

static const int startWavelength = 390;
static const int waveLengthStep = 5;
static const int dataCount = 89;
static ColorXYZ convertTable[90];

void initLightIntensity() {
    FILE *csv = fopen("wavelengthToXYZ.csv", "r");
    for (int i = 0; i < 89; i++) {
        int lambda;
        fscanf(csv, "%d,", &lambda);
        fscanf(csv, "%lf,%lf,%lf", &convertTable[i].X, &convertTable[i].Y, &convertTable[i].Z);
    }
    fclose(csv);
}

ColorXYZ getXYZ(double waveLength) {
    if (waveLength < startWavelength || waveLength > startWavelength + waveLengthStep * dataCount)
        return (ColorXYZ){0, 0, 0};

    // we shall find which two the slots that should be used
    int slot = (waveLength - startWavelength) / waveLengthStep;
    // so we use slot and slot + 1.
    // Theoretically slot + 1 should < end of wavelength data except wavelength is exactly equal to
    // 830 nm.
    if (slot == 89) return convertTable[89];

    double slot1WaveLength = startWavelength + waveLengthStep * slot;
    double slot2WaveLength = startWavelength + waveLengthStep * (slot + 1);
    // lerp
    double X = (convertTable[slot].X * (slot2WaveLength - waveLength) +
                convertTable[slot + 1].X * (waveLength - slot1WaveLength)) /
               waveLengthStep;
    double Y = (convertTable[slot].Y * (slot2WaveLength - waveLength) +
                convertTable[slot + 1].Y * (waveLength - slot1WaveLength)) /
               waveLengthStep;
    double Z = (convertTable[slot].Z * (slot2WaveLength - waveLength) +
                convertTable[slot + 1].Z * (waveLength - slot1WaveLength)) /
               waveLengthStep;
    return (ColorXYZ){X, Y, Z};
}

static inline double adj(double C) {
    if (fabs(C) < 0.0031308) {
        return 12.92 * C;
    }
    return 1.055 * pow(C, 0.41666) - 0.055;
}

ScreenColor XYZtoRGB(ColorXYZ xyz) {
    // Source - https://stackoverflow.com/a/45238704
    // Posted by blazs, modified by community. See post 'Timeline' for change history
    // Retrieved 2026-07-18, License - CC BY-SA 4.0
    double X = xyz.X;
    double Y = xyz.Y;
    double Z = xyz.Z;
    double R = adj(3.2404542 * X - 1.5371385 * Y - 0.4985314 * Z);
    double G = adj(-0.9692660 * X + 1.8760108 * Y + 0.0415560 * Z);
    double B = adj(0.0556434 * X - 0.2040259 * Y + 1.0572252 * Z);
    return (ScreenColor){R, G, B, 0};
}