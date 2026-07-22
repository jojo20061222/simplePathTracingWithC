#include "textures.h"
#include "base.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

Texture newTexture(double (*baseColor)(double), double (*refractiveIndexInside)(double),
                   double (*refractiveIndexOutside)(double)) {
    return (Texture){baseColor, refractiveIndexInside, refractiveIndexOutside};
}

static const int AG_LINE_COUNT = 446;
static Complex AgEta[AG_LINE_COUNT];
static const char AG_PATH_N[] = "src/static/ag_n.csv";
static const char AG_PATH_K[] = "src/static/ag_k.csv";
static const double AgLambdaStep = 0.001;

static const int AU_LINE_COUNT = 46;
static Complex AuEta[AU_LINE_COUNT];
static const char AU_PATH_N[] = "src/static/au_n.csv";
static const char AU_PATH_K[] = "src/static/au_k.csv";
static const double AuLambdaStep = 0.01;

static const int CU_LINE_COUNT = 46;
static Complex CuEta[CU_LINE_COUNT];
static const char CU_PATH_N[] = "src/static/cu_n.csv";
static const char CU_PATH_K[] = "src/static/cu_k.csv";
static double CuLambdaStep = 0.01;

static const double startLambda = 390;

void readFile(const char *filePathN, const char *filePathK, Complex Eta[], int lineCount) {
    printf("%s %s %d\n", filePathN, filePathK, lineCount);
    /*
    int lineCountFromFile;
    const char *failedPath;
    int errorCode = 0;

    FILE *fN = fopen(filePathN, "r");
    if (fN == NULL) {
        fprintf(stderr, "fuck\n");
        exit(1);
    }
    fscanf(fN, "%d", &lineCount);
    if (lineCount != lineCountFromFile) {
        exit(1);
    }
    for (int i = 0; i < lineCount; i++) {
        double lambda;
        int successfulCount = fscanf(fN, "%lf,%lf", &lambda, &Eta[i].R);
        if (successfulCount == 0) {
            exit(1);
        }
    }
    fclose(fN);
    printf("File loaded successfully: %s\n", filePathN);


    FILE *fK = fopen(filePathK, "r");
    if (fK == NULL) {
        exit(1);
    }
    fscanf(fK, "%d", &lineCount);
    if (lineCount != lineCountFromFile) {
        exit(1);
    }
    for (int i = 0; i < lineCount; i++) {
        double lambda;
        int successfulCount = fscanf(fK, "%lf,%lf", &lambda, &Eta[i].I);
        if (successfulCount == 0) {
            exit(1);
        }
    }
    fclose(fK);
    printf("File loaded successfully: %s\n", filePathK);
*/
    return;
}

void initTexture() {
    readFile(AG_PATH_N, AG_PATH_K, AgEta, AG_LINE_COUNT);
    readFile(AU_PATH_N, AU_PATH_K, AuEta, AU_LINE_COUNT);
    readFile(CU_PATH_N, CU_PATH_K, CuEta, CU_LINE_COUNT);
}

double pureWhite(double _) { return 0.5; }

double red(double lambda) { return 0.05 + 0.75 / (1 + exp(-0.06 * (lambda - 650))); }

double plastic(double _) { return 1.5; }

double air(double _) { return 1.0; };