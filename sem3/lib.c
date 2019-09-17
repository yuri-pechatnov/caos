

#include <stdint.h>

typedef union {
    double double_val;
    uint64_t uint64_val;
    struct {
    
        uint64_t mantissa_val : 52;
        uint64_t exp_val : 11;
        uint64_t sign_val : 1;
    };
} double_parser_t;

uint64_t get_sign(double x) {
    double_parser_t parser = {.double_val = x};
    return parser.sign_val;
}

uint64_t get_mantissa(double x) {
    double_parser_t parser = {.double_val = x};
    return parser.mantissa_val;
}

uint64_t get_exp(double x) {
    double_parser_t parser = {.double_val = x};
    return parser.exp_val;
}


