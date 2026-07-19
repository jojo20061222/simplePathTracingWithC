#include "lightSource.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

CollideLightSourceResult collideLightSource(const LightSource *source, const ReversedRay *ray) {
    CollideResult result = source->shape->detectCollide(source->shape, ray);
    return (CollideLightSourceResult){result.collide, result.t, source->intensity(ray->waveLength)};
}

static const int startWavelength = 390;
static const int waveLengthStep = 5;
static const int dataCount = 89;
// 390 ~ 835 nm
static ColorXYZ convertTable[90];

void initLightIntensity() {
    FILE *csv = fopen("src/wavelengthToXYZ.csv", "r");

    if (csv == NULL) {
        fprintf(stderr, "Not able to open the csv file\n");
        exit(1);
    }

    for (int i = 0; i < 89; i++) {
        int lambda;
        fscanf(csv, "%d,", &lambda);
        fscanf(csv, "%lf,%lf,%lf", &convertTable[i].X, &convertTable[i].Y, &convertTable[i].Z);
    }
    fclose(csv);
}

ColorXYZ wavelengthToXYZ(double waveLength) {
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
    double sRGB;
    if (fabs(C) < 0.0031308) {
        sRGB = 12.92 * C;
    }
    else {
        sRGB = 1.055 * pow(C, 0.41666) - 0.055;
    }
    return (sRGB) / (1 + sRGB) * 255;
}

ScreenColor XYZtoRGB(ColorXYZ xyz) {
    // Source - https://stackoverflow.com/a/45238704
    // Posted by blazs, modified by community. See post 'Timeline' for change history
    // Retrieved 2026-07-18, License - CC BY-SA 4.0
    double X = xyz.X;
    double Y = xyz.Y;
    double Z = xyz.Z;
    uint8_t R = adj(3.2404542 * X - 1.5371385 * Y - 0.4985314 * Z);
    uint8_t G = adj(-0.9692660 * X + 1.8760108 * Y + 0.0415560 * Z);
    uint8_t B = adj(0.0556434 * X - 0.2040259 * Y + 1.0572252 * Z);
    return (ScreenColor){R, G, B, 0};
}

double getBlackbodyIntensity(double wavelength, double T) {
    double lambda = wavelength * 1e-9;
    
    static const double h = 6.62607015e-34;
    static const double c = 2.99792458e+8;
    static const double kB = 1.380649e-23;
    
    static const double c1 = 2.0 * h * c * c;
    static const double c2 = (h * c) / kB;
    
    double exponent = c2 / (lambda * T);
    
    if (exponent > 700.0) return 0.0; 
    
    return c1 / (pow(lambda, 5.0) * (exp(exponent) - 1.0));
}

double tungstenLamp(double wavelength) {
    return getBlackbodyIntensity(wavelength, 2800) / getBlackbodyIntensity(555, 2800);
}