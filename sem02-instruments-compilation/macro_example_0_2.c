// %%cpp macro_example_0_2.c
// %run gcc -E macro_example_0_2.c -o macro_example_0_2_E.c
// %run cat macro_example_0_2_E.c

#define macro(type, var, value) type var = value;

macro(std::pair<int, int>, a, {1, 2, 3})

