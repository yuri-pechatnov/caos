
#include <assert.h>
#include <stdint.h>

unsigned int satsum(unsigned int x, unsigned int y) {
    unsigned int z;
    if (__builtin_uadd_overflow(x, y, &z)) {
        return -1;
    }
    return z;
}

int main() {
    assert(satsum((1LL << 31) - 1, (1LL << 31) - 1) == ((1LL << 31) - 1) * 2);
    assert(satsum((1LL << 31) + 1, (1LL << 31) + 1) == (unsigned int)-1);
    return 0;
}

