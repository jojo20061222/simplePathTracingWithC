#include "random.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

MT19937 *initMT19937(uint32_t seed) {
    MT19937 *mt = malloc(sizeof(MT19937));
    if (mt == NULL) {
        fprintf(stderr, "malloc mt failed\n");
        exit(1);
    }
    mt->mt = malloc(sizeof(uint32_t) * 624);
    if (mt->mt == NULL) {
        fprintf(stderr, "malloc mt failed\n");
        exit(1);
    }
    mt->mt[0] = seed;
    mt->mti = 0;

    for (int i = 1; i < 624; i++) {
        mt->mt[i] = 1812433253U * (mt->mt[i - 1] ^ mt->mt[i - 1] >> 30) + i;
    }
    return mt;
}

static inline void twist(MT19937 *mt) {
    uint32_t x;
    int i;

    // here I apply the optimized version vibe coded by Gemini to eliminate branch prediction
    // the updated version need no MOD624
    for (i = 0; i < 227; i++) {
        x = (mt->mt[i] & 0x80000000U) | (mt->mt[i + 1] & 0x7fffffffU);
        mt->mt[i] = mt->mt[i + 397] ^ (x >> 1) ^ ((x & 1) ? 0x9908b0dfU : 0);
    }

    for (; i < 623; i++) {
        x = (mt->mt[i] & 0x80000000U) | (mt->mt[i + 1] & 0x7fffffffU);
        mt->mt[i] = mt->mt[i - 227] ^ (x >> 1) ^ ((x & 1) ? 0x9908b0dfU : 0);
    }

    x = (mt->mt[623] & 0x80000000U) | (mt->mt[0] & 0x7fffffffU);
    mt->mt[623] = mt->mt[396] ^ (x >> 1) ^ ((x & 1) ? 0x9908b0dfU : 0);
}

uint32_t extractNumber(MT19937 *mt) {
    if (mt->mti == 0) {
        twist(mt);
    }
    uint32_t x = mt->mt[mt->mti];
    x ^= x >> 11;
    x ^= x << 7 & 2636928640U;
    x ^= x << 15 & 4022730752U;
    x ^= x >> 18;
    mt->mti = mt->mti + 1 == 624 ? 0 : mt->mti + 1;
    return x;
}

void destroyMT19937(MT19937 *mt) {
    free(mt->mt);
    free(mt);
}

Real randomReal(MT19937 *mt) { return (Real)extractNumber(mt) / (Real)UINT_MAX; }