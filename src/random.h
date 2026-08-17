#ifndef RANDOM_H
#define RANDOM_H
#include <limits.h>
#include <stdint.h>
#include "base.h"

typedef struct {
    uint32_t *mt;
    uint32_t mti;
} MT19937;

MT19937 *initMT19937(uint32_t seed);

uint32_t extractNumber(MT19937 *mt);

void destroyMT19937(MT19937 *mt);

// randomly generate a real number in [0, 1]
Real randomReal(MT19937 *mt);

// randomly generate a integer in [0, n - 1] 
uint32_t randomInt(MT19937 *mt, uint32_t n);

#endif