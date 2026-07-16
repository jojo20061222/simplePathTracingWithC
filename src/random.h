#ifndef MT19937_H
#define MT19937_H
#include <stdint.h>

typedef struct {
    uint32_t *mt;
    uint32_t mti;
} MT19937;

MT19937 *initMT19937(uint32_t seed);

uint32_t extractNumber(MT19937 *mt);

void destroyMT19937(MT19937 *mt);

#endif