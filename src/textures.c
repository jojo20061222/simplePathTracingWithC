#include "textures.h"
#include "base.h"
#include "random.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * @author: seaofvoices
 * @date: 2026/08/10 20:56
 * @brief: this file provides definition and example of textures, where refractive data mainly from
 * https://refractiveindex.info and lerped by lerp.py. Some data of gems comes from
 * http://minerals.gps.caltech.edu/FILES/Index.html
 */

/*
 * 1st bit: metallic
 * 2nd bit: transparent
 * 3rd bit: opaque
 */
typedef enum {
    DIELETRIC = 0x0U,
    METALLIC = 0x1U,
    FULLY_TRANSPARENT = 0x2U,
    OPAQUE = 0x4U,
    HALF_TRANSPARENT = 0x6U
} TextureAttributes;

typedef struct Texture {
    Complex (*refractiveIndex)(Texture *self, Real lambda);
    TextureAttributes tags;
    Real roughness;
    void *data; // for those data that are not common for all textures
} Texture;

typedef struct Interface{
    Texture in, out;
} Interface;

static void mallocFailed() {
    fprintf(stderr, "malloc tex failed\n");
    exit(1);
}

// to make sure the data of diffrent textures won't confuse anyone we have to seperate the original
// "newTexture" into different functions
Texture *newOpaqueDieletric(Complex (*refractiveIndex)(Texture *, Real), Real roughness,
                            Real (*baseColor)(Real)) {
    Texture *tex = malloc(sizeof(*tex));
    if (tex == NULL) {
        mallocFailed();
    }

    *tex = (Texture){refractiveIndex, DIELETRIC | OPAQUE, roughness,
                     baseColor};
    return tex;
}

Texture *newTransparentDieletric(Complex (*refractiveIndexInside)(Texture *, Real),
                                 Complex (*refractiveIndexOutside)(Texture *, Real), Real roughness,
                                 Real concentration) {
    Texture *tex = malloc(sizeof(*tex));
    if (tex == NULL) {
        mallocFailed();
    }

    *tex = (Texture){refractiveIndexInside, refractiveIndexOutside, DIELETRIC | FULLY_TRANSPARENT,
                     roughness};
    tex->data = malloc(sizeof(Real));
    if (tex->data == NULL) {
        mallocFailed();
    }
    *(Real *)(tex->data) = concentration;
    return tex;
}

Texture *newMetal(Complex (*refractiveIndexInside)(Texture *, Real),
                  Complex (*refractiveIndexOutside)(Texture *, Real), Real roughness) {
    Texture *tex = malloc(sizeof(*tex));
    if (tex == NULL) {
        mallocFailed();
    }
    *tex = (Texture){refractiveIndexInside, refractiveIndexOutside, METALLIC | OPAQUE, roughness,
                     NULL};
    return tex;
}

typedef struct {
    Real (*sigmaS)(Real);
    Vector (*sampleNextDirection)(Vector,
                                  MT19937 *); // sample next direction through phase function
} HalfTransparentTextureData;

Texture *newHalfTransparentDieletric(Complex (*refractiveIndexInside)(Texture *, Real),
                                     Complex (*refractiveIndexOutside)(Texture *, Real),
                                     Real roughness, Real (*sigmaS)(Real),
                                     Vector (*sampleNextDirection)(Vector, MT19937 *)) {
    Texture *tex = malloc(sizeof(*tex));
    if (tex == NULL) mallocFailed();
    HalfTransparentTextureData *data = malloc(sizeof(*data));
    if (data == NULL) mallocFailed();
    data->sampleNextDirection = sampleNextDirection;
    data->sigmaS = sigmaS;
    *tex = (Texture){refractiveIndexInside, refractiveIndexOutside, DIELETRIC | HALF_TRANSPARENT,
                     roughness, data};
    return tex;
}

Vector sampleHenyeyGreenstein(Vector input, MT19937 *mt, Real g) {
    Real u = randomReal(mt), v = randomReal(mt);
    Real theta = 2 * M_PI * u;

    Real cosPhi;
    if (rabs(g) < error) {
        cosPhi = 1 - 2 * v;
    }
    else {
        Real g2 = g * g;
        Real temp = (1 - g2) / (1 + g - 2 * g * v);
        cosPhi = (1 + g2 - temp * temp) / (2 * g);
    }
    Real sinPhi = sqrt(rmax(1 - cosPhi * cosPhi, 0.0));
    Vector local = {rcos(theta) * sinPhi, rsin(theta) * sinPhi, cosPhi};

    return toWorld(local, input);
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
    int lineCountFromFile = 0;

    FILE *fN = fopen(filePathN, "r");
    if (fN == NULL) {
        exit(1);
    }

    fscanf(fN, "%d", &lineCount);
    for (int i = 0; i < lineCount; i++) {
        Real lambda = 0;
        int successfulCount = fscanf(fN, REAL_FMT "," REAL_FMT, &lambda, &eta[i].R);
    }
    fclose(fN);

    FILE *fK = fopen(filePathK, "r");
    if (fK == NULL) {
        exit(1);
    }
    fscanf(fK, "%d", &lineCount);
    for (int i = 0; i < lineCount; i++) {
        Real lambda = 0;
        int successfulCount = fscanf(fK, REAL_FMT "," REAL_FMT, &lambda, &eta[i].I);
    }
    fclose(fK);
    return;
}

#define SAPPHIRE_LINE_COUNT 46
// let real part of EC be E perp C and the imaginary part be E para C since I want to reuse the code
// of "getRefractivity" function
static Complex EC[SAPPHIRE_LINE_COUNT];
static const char SAPPHIRE_PATH[] = "src/static/sapphire_epsilon.csv";
static const Real SAPPHIRE_STEP = 10;

// since no direct k can be directly used by sapphire, we need a different way to read the epsilon
static void readSapphireEpsilon() {
    int lineCountFromFile = 0;
    FILE *f = fopen(SAPPHIRE_PATH, "r");

    fscanf(f, "%d", &lineCountFromFile);

    for (int i = 0; i < SAPPHIRE_LINE_COUNT; i++) {
        Real lambda;
        fscanf(f, REAL_FMT "," REAL_FMT "," REAL_FMT, &lambda, &EC[i].R, &EC[i].I);
    }

    fclose(f);
    return;
}

void initTexture() {
    readFile(AG_PATH_N, AG_PATH_K, AgEta, AG_LINE_COUNT);
    readFile(AU_PATH_N, AU_PATH_K, AuEta, AU_LINE_COUNT);
    readFile(CU_PATH_N, CU_PATH_K, CuEta, CU_LINE_COUNT);
    readFile(FE_PATH_N, FE_PATH_K, FeEta, FE_LINE_COUNT);
    readFile(GLASS_PATH_N, GLASS_PATH_K, GlassEta, GLASS_LINE_COUNT);
    readSapphireEpsilon();
}

Real pureWhite(Real _) { return 0.8; }

Real pureBlack(Real _) { return 0; }

Real red(Real lambda) { return 0.05 + 0.75 / (1 + rexp(-0.06 * (lambda - 650))); }

Real green(Real lambda) { return 0.05 + 0.75 / (1 + rexp(-0.06 * (lambda - 650))); }

Complex getPlasticRefractivity(Texture *__, Real _) { return (Complex){1.5, 0.0}; }

Complex getAirRefractivity(Texture *__, Real _) { return (Complex){1.0, 0.0}; };

// get refractivity from tables above
Complex getRefractivity(Real lambda, Complex eta[], Real lambdaStep, int lineCount) {
    if (lambda < startLambda) return (Complex){0, 0};
    if (lambda > startLambda + lineCount * lambdaStep) return (Complex){0, 0};
    int slot = (lambda - startLambda) / lambdaStep;
    if (slot < 0) return (Complex){0, 0};
    if (slot + 1 >= lineCount) return (Complex){0, 0};
    int preLambda = startLambda + lambdaStep * slot;
    int nxtLambda = startLambda + lambdaStep * (slot + 1);
    return (Complex){lerp(preLambda, nxtLambda, lambda, eta[slot].R, eta[slot + 1].R),
                     lerp(preLambda, nxtLambda, lambda, eta[slot].I, eta[slot + 1].I)};
}

Complex getSilverRefractivity(Texture *_, Real lambda) {
    return getRefractivity(lambda, AgEta, AG_LAMBDA_STEP, AG_LINE_COUNT);
}

Complex getGoldRefractivity(Texture *_, Real lambda) {
    return getRefractivity(lambda, AuEta, AU_LAMBDA_STEP, AU_LINE_COUNT);
}

Complex getCopperRefractivity(Texture *_, Real lambda) {
    return getRefractivity(lambda, CuEta, CU_LAMBDA_STEP, CU_LINE_COUNT);
}

Complex getIronRefractivity(Texture *_, Real lambda) {
    return getRefractivity(lambda, FeEta, FE_LAMBDA_STEP, FE_LINE_COUNT);
}

Complex getGlassRefractivity(Texture *_, Real lambda) {
    return getRefractivity(lambda, GlassEta, GLASS_LAMBDA_STEP, GLASS_LINE_COUNT);
}

Complex getDiamondRefractivity(Texture *_, Real x) {
    x /= 1000;
    return (Complex){
        sqrt(1 + 0.3306 / (1 - pow(0.1750 / x, 2)) + 4.3356 / (1 - pow(0.1060 / x, 2))), 0};
}

Complex getSapphireRefractivity(Texture *self, Real lambda) {
    // little trick: let function getRefractivity lerp and edge check for me
    Complex eCombined = getRefractivity(lambda, EC, SAPPHIRE_STEP, SAPPHIRE_LINE_COUNT);

    lambda /= 1000; // convert lambda from nanometer to micrometer
    // calculate the n part
    static const Real a = 1.5039759;
    static const Real b = 0.0740288;
    static const Real c = 0.55069141;
    static const Real d = 0.1216529;
    static const Real e = 6.5927379;
    static const Real f = 20.072248;
    Real lambda2 = lambda * lambda;
    Real nterm1 = a * lambda2 / (lambda2 - b * b);
    Real nterm2 = c * lambda2 / (lambda2 - d * d);
    Real nterm3 = e * lambda2 / (lambda2 - f * f);
    Real n = sqrt(1 + nterm1 + nterm2 + nterm3);
    // calcuate the k part
    Real EperpC = eCombined.R;
    Real EparaC = eCombined.I;

    Real epsilon = (EperpC + EparaC) / 2;
    Real concentration = *(Real *)self->data;
    Real alpha = rlog(10) * epsilon * concentration * 1e-4; // convert alpha to micrometer^(-1)
    Real k = alpha * lambda / (4 * M_PI);

    return (Complex){n, k};
}

Texture *newSapphire(Real roughness, Real concentration) {
    return newTransparentDieletric(getSapphireRefractivity, getAirRefractivity, roughness,
                                   concentration);
}

Texture *newGold(Real roughness) {
    return newMetal(getGoldRefractivity, getAirRefractivity, roughness);
}

Texture *newSilver(Real roughness) {
    return newMetal(getSilverRefractivity, getAirRefractivity, roughness);
}

Texture *newCopper(Real roughness) {
    return newMetal(getCopperRefractivity, getAirRefractivity, roughness);
}

Texture *newIron(Real roughness) {
    return newMetal(getIronRefractivity, getAirRefractivity, roughness);
}

Texture *newGlass(Real roughness) {
    // for glass we don't care about its color (it has a fixed extinction coefficient), so
    // concentration remains 0
    return newTransparentDieletric(getGlassRefractivity, getAirRefractivity, roughness, 0);
}

Texture *newDiamond(Real roughness) {
    // for the same reason as glass, so concentration remains 0
    return newTransparentDieletric(getDiamondRefractivity, getAirRefractivity, roughness, 0);
}

// Wrappers
Real getTextureBaseColor(Texture *texture, Real lambda) {
    Real (*baseColor)(Real) = texture->data;
    return baseColor(lambda);
}

Real getTextureSigmaS(Texture *texture, Real lambda) {
    HalfTransparentTextureData *data = texture->data;
    return data->sigmaS(lambda);
}

Vector sampleTextureNextDirection(Texture *texture, Vector currentDirection, MT19937 *mt) {
    HalfTransparentTextureData *data = texture->data;
    return data->sampleNextDirection(currentDirection, mt);
}

Complex getTextureRefractiveIndexInside(Texture *texture, Real lambda) {
    return texture->refractiveIndexInside(texture, lambda);
}

Complex getTextureRefractiveIndexOutside(Texture *texture, Real lambda) {
    return texture->refractiveIndexOutside(texture, lambda);
}

bool isTexturemetallic(Texture *texture) { return texture->tags & METALLIC; }
bool isTextureFullyTransparent(Texture *texture) {
    static const uint32_t transparent_mask = HALF_TRANSPARENT;
    return (texture->tags & HALF_TRANSPARENT) == FULLY_TRANSPARENT;
}
bool isTextureHalfTransparent(Texture *texture) {
    return (texture->tags & HALF_TRANSPARENT) == HALF_TRANSPARENT;
}

Real getTextureRoughness(Texture *texture) { return texture->roughness; }

void destroyTexture(Texture *tex) {
    if (isTextureFullyTransparent(tex) || isTextureHalfTransparent(tex)) {
        // for opaque materials, data is a function
        // they shall not be freed
        free(tex->data);
    }
    free(tex);
}

#undef AG_LINE_COUNT
#undef AU_LINE_COUNT
#undef CU_LINE_COUNT
#undef FE_LINE_COUNT
#undef GLASS_LINE_COUNT
#undef SAPPHIRE_LINE_COUNT