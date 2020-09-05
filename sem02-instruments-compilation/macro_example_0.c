// %%cpp macro_example_0.c
// %run gcc -E macro_example_0.c -o macro_example_E.c
// %run cat macro_example_E.c

#define people students
#define goodbye(x) Good bye x! 

Hello people!
goodbye(bad grades)

