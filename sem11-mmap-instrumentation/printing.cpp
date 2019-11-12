// %%cpp printing.cpp

int work_hard_1(int n) {
    int ret = 0;
    for (int i = 0; i < n; i++) ret ^= i;
    return ret;
}

int work_hard_2(int n) {
    int ret = 0;
    for (int i = 0; i < n; i++) ret ^= work_hard_1(i * 3);
    return ret;
}

int main() {
    return work_hard_2(1000);
}

