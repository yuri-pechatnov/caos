// %%cpp macro_example_0_2.c
// %run gcc -E macro_example_0_2.c -o macro_example_0_2_E.c
// %run cat macro_example_0_2_E.c

#define macro(type, var) type var;

macro(std::pair<int, int>, a)

