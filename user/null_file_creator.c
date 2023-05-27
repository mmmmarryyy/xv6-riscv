#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "user/user.h"

int create_null_file(const char *file, int number_of_bytes) {
    int fd;

    if ((fd = open(file, O_WRONLY | O_CREATE | O_TRUNC)) < 0) {
        return -1;
    }

    static char null_file[BSIZE];
    memset(null_file, 0, BSIZE);

    while (number_of_bytes) {
        int to_write = number_of_bytes < BSIZE ? number_of_bytes : BSIZE;
        number_of_bytes -= to_write;
        if (write(fd, null_file, to_write) != to_write) {
            close(fd);
            return -1;
        }
    }

    close(fd);
    return 0;
}

int string_to_uint(const char *number, int *result) {
    *result = 0;

    for (const char *iter = number; *iter; ++iter) {
        
        if (*iter < '0' || *iter > '9') {
            return -1;
        }

        int digit = *iter - '0';
        *result *= 10;
        *result += digit;
    }

    return 0;
}

int main(int argc, const char **argv) {
    if (argc != 4) {
        printf("incorrect number of argument for creating null file\n");
        exit(-1);
    }

    const char *file = argv[1];
    int number_of_bytes, error;

    if (string_to_uint(argv[2], &number_of_bytes)) {
        printf("incorrect number of argument for creating null file\n");
        exit(-1);
    }

    if (strcmp(argv[3], "1")) {
        if ((1u << 31) <= number_of_bytes * BSIZE) {
            printf("creating such big file: failed\n");
            exit(-1);
        }
        number_of_bytes *= BSIZE;
    }
    
    if ((error = create_null_file(file, number_of_bytes))) {
        printf("creating null file: failed\n");
    }
    exit(error);
}