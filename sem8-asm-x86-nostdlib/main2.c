//%run ls -la main2.exe  # Заметьте, что размер стал сильно больше
//%run ldd main2.exe
//%run objdump -M intel -d main2.exe

#include <unistd.h>
#include <sys/syscall.h>

int main() {
    syscall(SYS_write, 1, "Hello!", 7);
    close(1);
    return 0;
}

