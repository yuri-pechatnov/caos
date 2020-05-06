// %%cpp fuse_c_example/main.c
// %run mkdir fuse_c_example/build 2>&1 | grep -v "File exists"
// %run cd fuse_c_example/build && cmake .. > /dev/null && make
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#ifdef FUSE2
    #define FUSE_USE_VERSION 26
#else
    #define FUSE_USE_VERSION 30
#endif
#include <fuse.h>

typedef struct { 
    char* filename;
    char* filecontent;
    char* log;
} my_options_t;
my_options_t my_options;


void print_cwd() {
    if (my_options.log) {
        FILE* f = fopen(my_options.log, "at");
        char buffer[1000];
        getcwd(buffer, sizeof(buffer));
        fprintf(f, "Current working dir: %s\n", buffer);
        fclose(f);
    }
}


int getattr_callback(const char* path, struct stat* stbuf
#ifndef FUSE2
    , struct fuse_file_info *fi
#endif
) {
#ifndef FUSE2
    (void) fi;
#endif   
    if (strcmp(path, "/") == 0) {
        *stbuf = (struct stat) {.st_nlink = 2, .st_mode = S_IFDIR | 0755};
        return 0;
    }
    if (path[0] == '/' && strcmp(path + 1, my_options.filename) == 0) {
        *stbuf = (struct stat) {.st_nlink = 2, .st_mode = S_IFREG | 0777, .st_size = (__off_t)strlen(my_options.filecontent)};
        return 0;
    }
    return -ENOENT;
}

int readdir_callback(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi
#ifndef FUSE2
    , enum fuse_readdir_flags flags
#endif
) {
#ifdef FUSE2
    (void) offset; (void) fi;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, my_options.filename, NULL, 0);
#else
    (void) offset; (void) fi; (void)flags;
    filler(buf, ".", NULL, 0, (enum fuse_fill_dir_flags)0);
    filler(buf, "..", NULL, 0, (enum fuse_fill_dir_flags)0);
    filler(buf, my_options.filename, NULL, 0, (enum fuse_fill_dir_flags)0);
#endif   
    return 0;
}

int read_callback(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    print_cwd();
    // "/", "/my_file"
    if (path[0] == '/' && strcmp(path + 1, my_options.filename) == 0) {
        size_t len = strlen(my_options.filecontent);
        if (offset >= len) {
            return 0;
        }
        size = (offset + size <= len) ? size : (len - offset);
        memcpy(buf, my_options.filecontent + offset, size);
        return size;
    }
    return -ENOENT;
}

struct fuse_operations fuse_example_operations = {
    .getattr = getattr_callback,
    .read = read_callback,
    .readdir = readdir_callback,
};

struct fuse_opt opt_specs[] = {
    { "--file-name %s", offsetof(my_options_t, filename), 0 },
    { "--file-content %s", offsetof(my_options_t, filecontent), 0 },
    { "--log %s", offsetof(my_options_t, log), 0 },
    FUSE_OPT_END
};

int main(int argc, char** argv) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    fuse_opt_parse(&args, &my_options, opt_specs, NULL);
    print_cwd();
    int ret = fuse_main(args.argc, args.argv, &fuse_example_operations, NULL);
    fuse_opt_free_args(&args);
    return ret;
}

