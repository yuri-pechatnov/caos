// %%cpp macro_example_0.c
// %run gcc -E macro_example_0.c -o macro_example_0_E.c
// %run cat macro_example_0_E.c

#define people students and students
#define goodbye(var) Good bye var! 

Hello people!
#undef people
Hello people!
goodbye(bad grades)

