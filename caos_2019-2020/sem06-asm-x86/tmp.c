
extern int N;
extern int *A;
extern int *B;
extern int *R;

extern void summ() {
    for (int i = 0; i < N; ++i) {
        R[i] = A[i] + B[i];
    }
}

