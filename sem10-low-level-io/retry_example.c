// %%cpp retry_example.c
// %run gcc -D_USE_READ retry_example.c -o retry_example.exe
// %run echo -n "Hello_world_1!" | ./retry_example.exe 
// %run gcc -D_USE_READ retry_example.c -o retry_example.exe
// %run (echo -n "Hello_" ; sleep 0.2 ; echo -n "world_2!") | ./retry_example.exe  
// %run gcc -D_USE_READ_RETRY retry_example.c -o retry_example.exe
// %run (echo -n "Hello_" ; sleep 0.2 ; echo -n "world_3!") | ./retry_example.exe  
// %run gcc -D_USE_SCANF retry_example.c -o retry_example.exe
// %run (echo -n "Hello_" ; sleep 0.2 ; echo -n "world_4!") | ./retry_example.exe  

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


int read_retry(int fd, char* data, int size) {
    char* cdata = data;
    while (1) {
        int read_bytes = read(fd, cdata, size);
        if (read_bytes == 0) { // если read вернул 0, значит файловый дескриптор закрыт с другого конца 
                               // или конец файла
            return cdata - data;
        }
        if (read_bytes < 0) { // если возвращено значение < 0, то это ошибка
            if (errno == EAGAIN || errno == EINTR) { // она может быть retryable
                continue;
            } else { // а может быть критичной, и нет смысла пытаться повторить попытку чтения
                return -1;
            }
        }
        // если возвращенное значение > 0, значит успешно прочитано столько байт
        cdata += read_bytes;
        size -= read_bytes;
        if (size == 0) {
            return cdata - data;
        }
    }
}


int main(int argc, char *argv[])
{ 
    char buffer[4096];
#if defined(_USE_READ) || defined(_USE_READ_RETRY)
  #if defined(_USE_READ)
    int bytes_read = read(0, buffer, sizeof(buffer)); 
  #endif
  #if defined(_USE_READ_RETRY)
    int bytes_read = read_retry(0, buffer, sizeof(buffer)); 
  #endif    
    if (bytes_read < 0) {
        perror("Error reading file");
        return -1;
    }
#endif
#if defined(_USE_SCANF)
    scanf("%s", buffer);
    int bytes_read = strlen(buffer);
#endif  
    printf("Read '%.*s'", bytes_read, buffer);
    
    return 0;
}

