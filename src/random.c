#include "random.h"
#include <limits.h>
#include <stdlib.h>

static inline uint32_t MOD624(uint32_t x) { return x >= 624 ? x - 624 : x; }

MT19937 *initMT19937(uint32_t seed) {
    MT19937 *mt = malloc(sizeof(MT19937));
    mt->mt = malloc(sizeof(uint32_t) * 624);
    mt->mt[0] = seed;
    mt->mti = 0;

    for (int i = 1; i < 624; i++) {
        mt->mt[i] = 1812433253U * (mt->mt[i - 1] ^ mt->mt[i - 1] >> 30) + i;
    }
    return mt;
}

static inline void twist(MT19937 *mt) {
    uint32_t x;
    for (int i = 0; i < 624; i++) {
        x = (mt->mt[i] & 0x80000000U) | (mt->mt[MOD624(i + 1)] & 0x7fffffffU);
        mt->mt[i] = (x >> 1) ^ mt->mt[MOD624(i + 397)];
        if (x & 1) mt->mt[i] ^= 0x9908b0dfU;
    }
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
    mt->mti = MOD624(mt->mti + 1);
    return x;
}

void destroyMT19937(MT19937 *mt) {
    free(mt->mt);
    free(mt);
}