#include "lightSource.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * @author seaofvoices
 * @brief This file defines how light behaves and relate energy and color. It also gives some
 * classic example of light sources such as sunlight and standard white light at 6,500 K
 */

LightSource newLightSource(Shape *shape, LightIntensity (*intensity)(Real)) {
    return (LightSource){shape, intensity};
}

CollideLightSourceResult collideLightSource(LightSource *source, ReversedRay *ray) {
    CollideResult result = detectCollide(source->shape, ray);
    return (CollideLightSourceResult){result.collide, result.distance, result.normalVector,
                                      source->intensity(ray->wavelength)};
}

static const int startWavelength = 390;
static const int wavelengthStep = 5;
static const int dataCount = 89;
// 390 ~ 835 nm
static ColorXYZ convertTable[90];

void initLightIntensity() {
    FILE *csv = fopen("src/static/wavelengthToXYZ.csv", "r");

    if (csv == NULL) {
        fprintf(stderr, "Not able to open the csv file\n");
        exit(1);
    }

    for (int i = 0; i < 89; i++) {
        int lambda;
        fscanf(csv, "%d,", &lambda);
        fscanf(csv, REAL_FMT "," REAL_FMT "," REAL_FMT, &convertTable[i].X, &convertTable[i].Y,
               &convertTable[i].Z);
    }
    fclose(csv);
}

ColorXYZ wavelengthToXYZ(Real wavelength) {
    if (wavelength < startWavelength || wavelength > startWavelength + wavelengthStep * dataCount)
        return (ColorXYZ){0, 0, 0};

    // we shall find which two the slots that should be used
    int slot = (wavelength - startWavelength) / wavelengthStep;
    // so we use slot and slot + 1.
    // Theoretically slot + 1 should < end of wavelength data except wavelength is exactly equal to
    // 830 nm.
    if (slot == 89) return convertTable[89];

    Real slot1WaveLength = startWavelength + wavelengthStep * slot;
    Real slot2WaveLength = startWavelength + wavelengthStep * (slot + 1);
    // lerp
    Real X = lerp(slot1WaveLength, slot2WaveLength, wavelength, convertTable[slot].X,
                  convertTable[slot + 1].X);
    Real Y = lerp(slot1WaveLength, slot2WaveLength, wavelength, convertTable[slot].Y,
                  convertTable[slot + 1].Y);
    Real Z = lerp(slot1WaveLength, slot2WaveLength, wavelength, convertTable[slot].Z,
                  convertTable[slot + 1].Z);

    return (ColorXYZ){X, Y, Z};
}

static inline Real adj(Real C) {
    Real sRGB;
    if (C < 0) C = 0; // clamp
    if (rabs(C) < 0.0031308) {
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
    Real X = xyz.X;
    Real Y = xyz.Y;
    Real Z = xyz.Z;

    uint8_t R = adj(3.2404542 * X - 1.5371385 * Y - 0.4985314 * Z);
    uint8_t G = adj(-0.9692660 * X + 1.8760108 * Y + 0.0415560 * Z);
    uint8_t B = adj(0.0556434 * X - 0.2040259 * Y + 1.0572252 * Z);
    return (ScreenColor){R, G, B, 0};
}

Real getBlackbodyIntensity(Real wavelength, Real T) {
    Real lambda = wavelength * 1e-9;

    static const Real h = 6.62607015e-34;
    static const Real c = 2.99792458e+8;
    static const Real kB = 1.380649e-23;

    static const Real c1 = 2.0 * h * c * c;
    static const Real c2 = (h * c) / kB;

    Real exponent = c2 / (lambda * T);

    if (exponent > 700.0) return 0.0;

    return c1 / (pow(lambda, 5.0) * (exp(exponent) - 1.0));
}

Real tungstenLampBase(Real wavelength) {
    return getBlackbodyIntensity(wavelength, 2800) /
           215561768139.854309; // getBlackbodyIntensity(555, 2800)
}

Real sunlightBase(Real wavelength) {
    return getBlackbodyIntensity(wavelength, 5500) /
           20481294390815.578125; // getBlackbodyIntensity(555, 5500)
}

Real whiteLightBase(Real wavelength) {
    return getBlackbodyIntensity(wavelength, 6500) / getBlackbodyIntensity(555, 6500);
}
