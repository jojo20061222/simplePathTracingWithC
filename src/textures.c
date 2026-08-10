#include "textures.h"
#include "base.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Texture {
    Real (*baseColor)(Real lambda);
    Complex (*refractiveIndexInside)(Real lambda);
    Complex (*refractiveIndexOutside)(Real lambda);
    bool transparent;
    bool metallic;
    Real roughness;
} Texture;

Texture *newTexture(Real (*baseColor)(Real), Complex (*refractiveIndexInside)(Real),
                    Complex (*refractiveIndexOutside)(Real), bool transparent, bool metallic,
                    Real roughness) {
    Texture *tex = malloc(sizeof(*tex));
    if (tex == NULL) {
        fprintf(stderr, "malloc tex failed\n");
        exit(1);
    }

    *tex = (Texture){
        baseColor, refractiveIndexInside, refractiveIndexOutside, transparent, metallic, roughness};
    return tex;
}

#define AG_LINE_COUNT 446
static Complex AgEta[AG_LINE_COUNT];
static const char AG_PATH_N[] = "src/static/ag_n.csv";
static const char AG_PATH_K[] = "src/static/ag_k.csv";
static const Real AG_LAMBDA_STEP = 1;

#define AU_LINE_COUNT 46
static Complex AuEta[AU_LINE_COUNT];
static const char AU_PATH_N[] = "src/static/au_n.csv";
static const char AU_PATH_K[] = "src/static/au_k.csv";
static const Real AU_LAMBDA_STEP = 10;

#define CU_LINE_COUNT 46
static Complex CuEta[CU_LINE_COUNT];
static const char CU_PATH_N[] = "src/static/cu_n.csv";
static const char CU_PATH_K[] = "src/static/cu_k.csv";
static const Real CU_LAMBDA_STEP = 10;

#define FE_LINE_COUNT 46
static Complex FeEta[FE_LINE_COUNT];
static const char FE_PATH_N[] = "src/static/fe_n.csv";
static const char FE_PATH_K[] = "src/static/fe_k.csv";
static const Real FE_LAMBDA_STEP = 10;

#define GLASS_LINE_COUNT 45
static Complex GlassEta[GLASS_LINE_COUNT];
static const char GLASS_PATH_N[] = "src/static/glass_n.csv";
static const char GLASS_PATH_K[] = "src/static/glass_k.csv";
static const Real GLASS_LAMBDA_STEP = 10;

static const Real startLambda = 390;

static void readFile(const char *filePathN, const char *filePathK, Complex eta[], int lineCount) {
    int lineCountFromFile;

    FILE *fN = fopen(filePathN, "r");
    if (fN == NULL) {
        exit(1);
    }

    fscanf(fN, "%d", &lineCount);
    for (int i = 0; i < lineCount; i++) {
        Real lambda;
        int successfulCount = fscanf(fN, REAL_FMT "," REAL_FMT, &lambda, &eta[i].R);
    }
    fclose(fN);

    FILE *fK = fopen(filePathK, "r");
    if (fK == NULL) {
        exit(1);
    }
    fscanf(fK, "%d", &lineCount);
    for (int i = 0; i < lineCount; i++) {
        Real lambda;
        int successfulCount = fscanf(fK, REAL_FMT "," REAL_FMT, &lambda, &eta[i].I);
    }
    fclose(fK);
    return;
}

void initTexture() {
    readFile(AG_PATH_N, AG_PATH_K, AgEta, AG_LINE_COUNT);
    readFile(AU_PATH_N, AU_PATH_K, AuEta, AU_LINE_COUNT);
    readFile(CU_PATH_N, CU_PATH_K, CuEta, CU_LINE_COUNT);
    readFile(FE_PATH_N, FE_PATH_K, FeEta, FE_LINE_COUNT);
    readFile(GLASS_PATH_N, GLASS_PATH_K, GlassEta, GLASS_LINE_COUNT);
}

Real pureWhite(Real _) { return 0.8; }

Real pureBlack(Real _) { return 0; }

Real red(Real lambda) { return 0.05 + 0.75 / (1 + exp(-0.06 * (lambda - 650))); }

Real green(Real lambda) { return 0.05 + 0.75 / (1 + exp(-0.06 * (lambda - 650))); }

Complex getPlasticRefractivity(Real _) { return (Complex){1.5, 0.0}; }

Complex getAirRefractivity(Real _) { return (Complex){1.0, 0.0}; };

Complex getRefractivity(Real lambda, Complex eta[], Real lambdaStep, int lineCount) {
    if (lambda < startLambda) return (Complex){0, 0};
    if (lambda > startLambda + lineCount * lambdaStep) return (Complex){0, 0};
    int slot = (lambda - startLambda) / lambdaStep;
    if (slot < 0) return (Complex){0, 0};
    if (slot >= lineCount) return (Complex){0, 0};
    int preLambda = startLambda + lambdaStep * slot;
    int nxtLambda = startLambda + lambdaStep * (slot + 1);
    return (Complex){lerp(preLambda, nxtLambda, lambda, eta[slot].R, eta[slot + 1].R),
                     lerp(preLambda, nxtLambda, lambda, eta[slot].I, eta[slot + 1].I)};
}

Complex getSilverRefractivity(Real lambda) {
    return getRefractivity(lambda, AgEta, AG_LAMBDA_STEP, AG_LINE_COUNT);
}

Complex getGoldRefractivity(Real lambda) {
    return getRefractivity(lambda, AuEta, AU_LAMBDA_STEP, AU_LINE_COUNT);
}

Complex getCopperRefractivity(Real lambda) {
    return getRefractivity(lambda, CuEta, CU_LAMBDA_STEP, CU_LINE_COUNT);
}

Complex getIronRefractivity(Real lambda) {
    return getRefractivity(lambda, FeEta, FE_LAMBDA_STEP, FE_LINE_COUNT);
}

Complex getGlassRefractivity(Real lambda) {
    return getRefractivity(lambda, GlassEta, GLASS_LAMBDA_STEP, GLASS_LINE_COUNT);
}

Complex getDiamondRefractivity(Real x) {
    x /= 1000;
    return (Complex){
        sqrt(1 + 0.3306 / (1 - pow(0.1750 / x, 2)) + 4.3356 / (1 - pow(0.1060 / x, 2))), 0};
}

Texture *getGold(Real roughness) {
    return newTexture(NULL, getGoldRefractivity, getAirRefractivity, false, true, roughness);
}

Texture *getSilver(Real roughness) {
    return newTexture(NULL, getSilverRefractivity, getAirRefractivity, false, true, roughness);
}

Texture *getCopper(Real roughness) {
    return newTexture(NULL, getCopperRefractivity, getAirRefractivity, false, true, roughness);
}

Texture *getIron(Real roughness) {
    return newTexture(NULL, getIronRefractivity, getAirRefractivity, false, true, roughness);
}

Texture *getGlass(Real roughness) {
    return newTexture(NULL, getGlassRefractivity, getAirRefractivity, true, false, roughness);
}

Texture *getDiamond(Real roughness) {
    return newTexture(NULL, getDiamondRefractivity, getAirRefractivity, true, false, roughness);
}

// Wrappers
Real getTextureBaseColor(Texture *texture, Real lambda) { return texture->baseColor(lambda); }
Complex getTextureRefractiveIndexInside(Texture *texture, Real lambda) {
    return texture->refractiveIndexInside(lambda);
}
Complex getTextureRefractiveIndexOutside(Texture *texture, Real lambda) {
    return texture->refractiveIndexOutside(lambda);
}
bool getTexturemetallic(Texture *texture) { return texture->metallic; }
bool getTextureTransparent(Texture *texture) { return texture->transparent; }
Real getTextureRoughness(Texture *texture) { return texture->roughness; }

void destroyTexture(Texture *tex) {
    free(tex);
}

#undef AG_LINE_COUNT
#undef AU_LINE_COUNT
#undef CU_LINE_COUNT
#undef FE_LINE_COUNT