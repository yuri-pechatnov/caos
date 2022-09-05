// %%cpp div.c
// %run gcc -m64 -masm=intel -O3 div.c -S -o div.S
// %run cat div.S | ./asm_filter_useless

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

const int ALIGNMENT = 32;
const int MARKER_OF_BAD_ALLOC = 1;

void*
allocate_aligned_memory(size_t size) {
    void* raw_ptr = aligned_alloc(ALIGNMENT, ALIGNMENT * size);
    return raw_ptr;
}

void
is_valid_ptr(void* ptr) {
    if (!ptr) {
        fprintf(stderr, "Couldn't allocate memory\n");
        exit(MARKER_OF_BAD_ALLOC);
    }
}

void
read_matrix(float* mat_, uint32_t rows, uint32_t cols) {
    float* mat = __builtin_assume_aligned(mat_, 32);
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            float value = 0;
            scanf("%f", &value);
            mat[i * cols + j] = value;
        }
    }
}

float*
transpose_matrix(const float* matrix_, uint32_t mat_rows,
                        uint32_t mat_cols) {
    uint64_t n_elems = mat_cols * mat_rows;
    float* matrix = __builtin_assume_aligned(matrix_, 32);
    float* transposed_mat = allocate_aligned_memory(n_elems * sizeof
                                                    (float));
    if (!transposed_mat) {
        return NULL;
    }

    for (size_t i = 0; i < mat_cols; ++i) {
        for (size_t j = 0; j < mat_rows; ++j) {
            transposed_mat[i * mat_rows + j] = matrix[j * mat_cols + i];
        }
    }

    return transposed_mat;
}

float*
mult_matrixes(const float* restrict first_mat_,
                const float* restrict second_mat_,
                    uint32_t rows, uint32_t cols) {
    float* first_mat =  __builtin_assume_aligned(first_mat_, 32);
    float* second_mat = __builtin_assume_aligned(second_mat_, 32);

    const size_t size = rows * cols * sizeof(float);
    float* second_transp_mat = transpose_matrix(second_mat, cols, rows);
    float* mult = allocate_aligned_memory(size);

    if (!second_transp_mat || !mult) {
        return NULL;
    }

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < rows; ++j) {
            float value = 0;
            for (size_t k = 0; k < cols; ++k) {
                value += first_mat[i * cols + k]
                                * second_transp_mat[j * cols + k];
            }
            mult[i * rows + j] = value;
        }
    }

    return mult;
}

void print_matrix(const float* mat_, uint32_t rows,
                        uint32_t cols) {
    float* mat = __builtin_assume_aligned(mat_, 32);

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            printf("%f ", mat[i * cols + j]);
        }
        printf("\n");
    }
}

int main() {
    uint32_t mat_rows = 0;
    uint32_t mat_cols = 0;
    scanf("%u %u", &mat_rows, &mat_cols);

    uint64_t n_elems = mat_cols * mat_rows;
    float* first_mat = allocate_aligned_memory(n_elems * sizeof(float));
    float* second_mat = allocate_aligned_memory(n_elems * sizeof(float));

    is_valid_ptr(first_mat);
    is_valid_ptr(second_mat);

    read_matrix(first_mat, mat_rows, mat_cols);
    read_matrix(second_mat, mat_cols, mat_rows);

    float* mult = mult_matrixes(first_mat, second_mat, mat_rows, mat_cols);

    is_valid_ptr(mult);
    print_matrix(mult, mat_rows, mat_rows);

    free(first_mat);
    free(second_mat);
    free(mult);
    return 0;
}

