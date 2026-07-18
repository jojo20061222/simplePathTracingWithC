#include "lightSource.h"
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
    double X = convertTable[slot].X + 
}