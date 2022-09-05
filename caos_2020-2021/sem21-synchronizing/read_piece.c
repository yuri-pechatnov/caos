// %%cpp read_piece.c

union message {
    int i;
    char arr[4];
};
union message value;
  
//...

int readed_count = 0;
while(readed_count < 4) {
    r = read(socket_fd, &value + readed_count, sizeof(value) - readed_count);
    if (r > 0) {
        readed_count += r;
    } else if (r < 0) {
        assert(errno == EAGAIN);
    } else if (r == 0) {
        assert(0 && "can't read value");
    }
}

