// %%cpp read_piece.c

union message {
    int i;
    char arr[4];
};



union message value;
   

        while(readed_count < 4) {
            r = read(socket_fd, &value + readed_count, sizeof(value) - readed_count);
            log_printf("client readed %d bytes\n", r);
            if (r > 0) {
                readed_count += r;
            } else if (r < 0) {
                assert(errno == EAGAIN);
            } else if (readed_count == 0) {
                return 0;
            }
        }
        printf("%d\n", value);
    }

