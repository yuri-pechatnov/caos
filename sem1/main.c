
#include <assert.h>

int check_increment(int x);
unsigned int unsigned_check_increment(unsigned int x);

int main() {
    assert(unsigned_check_increment(1));
    assert(!unsigned_check_increment((1LL << 32) - 1));
    check_increment((1LL << 31) - 1);
    return 0;
}

