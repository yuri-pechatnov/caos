
#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
#ifdef WIN32
    printf("Defined WIN32\n");
#else
    printf("Not WIN32\n");
#endif
    if (argc < 2) {
        printf("Need at least 2 arguments\n");
        return 1;
    }
    HANDLE fileHandle = CreateFileA(
        argv[1], GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        char errorBuffer[1024];
        if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, GetLastError(),
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           errorBuffer, sizeof(errorBuffer), NULL))
        {
            printf("Format message failed with 0x%x\n", GetLastError());
            return -1;
        }
        printf("Can't open file: %s\n", errorBuffer);
        return -1;
    }
    
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    DWORD bytes_read;
    BOOL success;
    success = ReadFile(fileHandle, buffer, sizeof(buffer),
                       &bytes_read, NULL);
    if (!success) {
        perror("Error reading file"); // Это ошибка, perror смотрит в errno, а не в GetLastError()
        CloseHandle(fileHandle);
        return -1;
    }
    printf("Bytes read: %d\n'''%s'''\n", bytes_read, buffer);
    CloseHandle(fileHandle);
    return 0;
}

