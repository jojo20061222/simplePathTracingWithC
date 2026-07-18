// #include "lightSource.h"
#include <stdio.h>
/*
CollideLightSourceResult collideLightSource(const LightSource *source, const ReversedRay *ray) {
    CollideResult result = source->shape->detectCollide(source->shape, ray);
    return (CollideLightSourceResult){result.collide, result.t, source->intensity};
}
    */

static const int startWavelength = 390;
static double convertTable[90][3];

void initLightIntensity() {
    FILE *csv = fopen("wavelengthToXYZ", "r");
    for (int i = 0; i < 90; i++) {
        int lambda;
        scanf("%d,", &lambda);
        scanf("%lf,%lf,%lf", &convertTable[lambda - startWavelength][0],
              &convertTable[lambda - startWavelength][1],
              &convertTable[lambda - startWavelength][2]);
    }
    fclose(csv);
}

int main() {
    initLightIntensity();
    for (int i = 0; i < 90; i++) {
        printf("%d, %lf, %lf, %lf\n", i + startWavelength, convertTable[i][0], convertTable[i][1],
               convertTable[i][2]);
    }
}