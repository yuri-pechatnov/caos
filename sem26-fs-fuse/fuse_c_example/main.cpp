// %%cpp fuse_c_example/main.cpp
// %run mkdir fuse_c_example/build 
// %run cd fuse_c_example/build && cmake .. > /dev/null && make
#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>

typedef struct { 
    char *filename;
    char *filecontent;
} my_options_t;

my_options_t my_options;

int getattr_callback(const char* path, struct stat* stbuf) {
  memset(stbuf, 0, sizeof(struct stat));

  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
    return 0;
  }

  if (path[0] == '/' && strcmp(path + 1, my_options.filename) == 0) {
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    stbuf->st_size = strlen(my_options.filecontent);
    return 0;
  }

  return -ENOENT;
}

int readdir_callback(const char* path, void* buf, fuse_fill_dir_t filler, off_t, struct fuse_file_info*) {
  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  filler(buf, my_options.filename, NULL, 0);
  return 0;
}

int open_callback(const char *path, struct fuse_file_info *fi) {
  return 0;
}

int read_callback(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
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
  .open = open_callback,
  .read = read_callback,
  .readdir = readdir_callback,
};


int main(int argc, char *argv[])
{
    return 0;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    
    struct fuse_opt opt_specs[] = {
        { "--file-name %s", offsetof(my_options_t, filename), 0 },
        { "--file-content %s", offsetof(my_options_t, filecontent), 0 },
        { NULL, 0, 0}
    };
    
    fuse_opt_parse(&args, &my_options, opt_specs, NULL);
  
    int ret = fuse_main(args.argc, args.argv, &fuse_example_operations, NULL);

    return ret;
}

