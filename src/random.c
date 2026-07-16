#include "random.h"
#include <stdlib.h>

inline uint32_t MOD624(uint32_t x) {
    return x >= 624 ? x - 624 : 624;
}

MT19937 *initMT19937(uint32_t seed) {
    MT19937 *mt = malloc(sizeof(uint32_t) * 624);
    
}

inline void twist(MT19937 *mt) {
    for (int i = 0; i < 624; i++) {
        uint32_t x = (mt->mt[i] & 0x80000000) + (mt->mt[MOD624(i + 1)] & 0x7fffffffU);
        mt->mt[i] = (x >> 1) ^ (mt->mt[(i + 397)]);
        if (x & 1) {
            mt->mt[i] ^= 0x9908b0df;
        }
    }
}

uint32_t extractNumber(MT19937 *mt) {
    if (mt->mti == 0) twist(mt);
    uint32_t x = mt->mt[mt->mti];
    x ^= x >> 11;
    x ^= x << 7 & 2636928640U;
    x ^= x << 15 & 4022730752;
    x ^= x >> 18;
    mt->mti = (mt->mti + 1) >= 624 ? 0 : mt->mti + 1;
    return x;
}

