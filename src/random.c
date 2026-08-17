#include "random.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * @author seaofvoices
 * @brief To ensure the quality of Monte Carlo method, we cannot use the srand() and rand() included
 * in standard library in C. Instead, we implement our own random engine based on mt19937 algorithm.
 * The definition and source is mainly referenced from wikipedia, and some of the optimization is
 * done by Gemini to erase the branch prediction.
 */

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

uint32_t randomInt(MT19937 *mt, uint32_t n) {
    // https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
    // this algorithm is essentially changing the division operation into multiplication to reduce
    // time
    /*
     * When we calculate x % n we expect that every bucket has floor(x / n) numbers.
     * However, the last r = x % n numbers will cause the first r bucket have one more numbers,
     * which will cause error.
     * So, to avoid that we calculate l, the lower 32 bit part of nx, to further verify the if it's
     * in the range.
     */
    uint64_t m = extractNumber(mt) * (uint64_t)n; // m = nx
    uint32_t l = (uint32_t)m;                     //

    if (l < n) { // only when l is less then n it is possible to be out of range
        // black magic. 2^32 - n === 2^32 mod n and it's faster than using 64-bit interger
        uint32_t t = -n % n;
        while (l < t) {
            m = extractNumber(mt) * (uint64_t)n;
            l = (uint32_t)m;
        }
    }

    return m >> 32;
}