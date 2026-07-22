#include "textures.h"
#include <stdlib.h>
#include "base.h"
#include <math.h>
#include <stdio.h>

Texture newTexture(double (*baseColor)(double), double (*refractiveIndexInside)(double),
                   double (*refractiveIndexOutside)(double)) {
    return (Texture){baseColor, refractiveIndexInside, refractiveIndexOutside};
}

static const int AG_LINE_COUNT = 446;
static Complex AgEta[AG_LINE_COUNT];
static const char AG_PATH_N[] = "src/static/ag_n.csv";
static const char AG_PATH_K[] = "src/static/ag_k.csv";
static double AgLambdaStep;


static const int AU_LINE_COUNT = 46;
static Complex AuEta[AU_LINE_COUNT];
static const char AU_PATH_N[] = "src/static/au_n.csv";
static const char AU_PATH_K[] = "src/static/au_k.csv";
static const double AuLambdaStep = 0.01;

static const int CU_LINE_COUNT = 46;
static Complex CuEta[CU_LINE_COUNT];
static const char CU_PATH_N[] = "src/static/cu_n.csv";
static const char CU_PATH_K[] = "src/static/cu_k.csv";
static double 

static const double startLambda = 390;

void initTexture() {
    int LineCount;
    // init Ag (Silver)
    FILE *Ag = fopen(AG_PATH_N, "r");
    if (Ag == NULL) {
        fprintf(stderr, "Open file %s failed.\n", AG_PATH_N);
        exit(1);
    }
    fscanf(Ag, "%d", &LineCount);
    if (LineCount != AG) }

double pureWhite(double _) { return 0.5; }

double red(double lambda) { return 0.05 + 0.75 / (1 + exp(-0.06 * (lambda - 650))); }

double plastic(double _) { return 1.5; }

double air(double _) { return 1.0; };