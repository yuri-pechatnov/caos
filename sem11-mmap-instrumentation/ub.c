// %%cpp ub.c

int main(int argc, char** argv) {
    return (argc * (int)((1ull << 31) - 1)) + 1;
}

