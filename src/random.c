#include "random.h"
#include <stdlib.h>

MT19937 initMT19937(uint32_t seed) {
    return (MT19937){
        malloc(sizeof(uint32_t) * 624),
        0
    };
}

uint32_t extractNumber()