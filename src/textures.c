#include "textures.h"
#include "base.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

Texture newTexture(double (*baseColor)(double), Complex (*refractiveIndexInside)(double),
                   Complex (*refractiveIndexOutside)(double), bool transparent, bool metalic, double roughness) {
    return (Texture){baseColor, refractiveIndexInside, refractiveIndexOutside, transparent,
                     metalic, roughness};
}

static const int AG_LINE_COUNT = 446;
static Complex AgEta[AG_LINE_COUNT];
static const char AG_PATH_N[] = "src/static/ag_n.csv";
static const char AG_PATH_K[] = "src/static/ag_k.csv";
static const double AG_LAMBDA_STEP = 1;

static const int AU_LINE_COUNT = 46;
static Complex AuEta[AU_LINE_COUNT];
static const char AU_PATH_N[] = "src/static/au_n.csv";
static const char AU_PATH_K[] = "src/static/au_k.csv";
static const double AU_LAMBDA_STEP = 10;

static const int CU_LINE_COUNT = 46;
static Complex CuEta[CU_LINE_COUNT];
static const char CU_PATH_N[] = "src/static/cu_n.csv";
static const char CU_PATH_K[] = "src/static/cu_k.csv";
static const double CU_LAMBDA_STEP = 10;

static const int FE_LINE_COUNT = 46;
static Complex FeEta[FE_LINE_COUNT];
static const char FE_PATH_N[] = "src/static/fe_n.csv";
static const char FE_PATH_K[] = "src/static/fe_k.csv";
static const double FE_LAMBDA_STEP = 10;

static const double startLambda = 390;

static void readFile(const char *filePathN, const char *filePathK, Complex eta[], int lineCount) {
    int lineCountFromFile;

    FILE *fN = fopen(filePathN, "r");
    if (fN == NULL) {
        exit(1);
    }

    fscanf(fN, "%d", &lineCount);
    for (int i = 0; i < lineCount; i++) {
        double lambda;
        int successfulCount = fscanf(fN, "%lf,%lf", &lambda, &eta[i].R);
    }
    fclose(fN);

    FILE *fK = fopen(filePathK, "r");
    if (fK == NULL) {
        exit(1);
    }
    fscanf(fK, "%d", &lineCount);
    for (int i = 0; i < lineCount; i++) {
        double lambda;
        int successfulCount = fscanf(fK, "%lf,%lf", &lambda, &eta[i].I);
    }
    fclose(fK);
    return;
}

void initTexture() {
    readFile(AG_PATH_N, AG_PATH_K, AgEta, AG_LINE_COUNT);
    readFile(AU_PATH_N, AU_PATH_K, AuEta, AU_LINE_COUNT);
    readFile(CU_PATH_N, CU_PATH_K, CuEta, CU_LINE_COUNT);
    readFile(FE_PATH_N, FE_PATH_K, FeEta, FE_LINE_COUNT);
}

double pureWhite(double _) { return 0.5; }

double red(double lambda) { return 0.05 + 0.75 / (1 + exp(-0.06 * (lambda - 650))); }

Complex getPlasticRefractivity(double _) { return (Complex){1.5, 0.0}; }

Complex getAirRefractivity(double _) { return (Complex){1.0, 0.0}; };

Complex getRefractivity(double lambda, Complex eta[], double lambdaStep, int lineCount) {
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

Complex getSilverRefractivity(double lambda) {
    return getRefractivity(lambda, AgEta, AG_LAMBDA_STEP, AG_LINE_COUNT);
}

Complex getGoldRefractivity(double lambda) {
    return getRefractivity(lambda, AuEta, AU_LAMBDA_STEP, AU_LINE_COUNT);
}

Complex getCopperRefractivity(double lambda) {
    return getRefractivity(lambda, CuEta, CU_LAMBDA_STEP, CU_LINE_COUNT);
}

Complex getIronRefractivity(double lambda) {
    return getRefractivity(lambda, FeEta, FE_LAMBDA_STEP, FE_LINE_COUNT);
}

Texture getGold(double roughness) {
    return newTexture(NULL, getGoldRefractivity, getAirRefractivity, false, true, roughness);
}

Texture getSilver(double roughness) {
    return newTexture(NULL, getSilverRefractivity, getAirRefractivity, false, true, roughness);
}

Texture getCopper(double roughness) {
    return newTexture(NULL, getCopperRefractivity, getAirRefractivity, false, true, roughness);
}

Texture getIron(double roughness) {
    return newTexture(NULL, getIronRefractivity, getAirRefractivity, false, true, roughness);
}