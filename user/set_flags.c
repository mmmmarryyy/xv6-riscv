#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    if (argc < 4) {
        write(2, "too few arguments", 17);
        exit(-1);
    }

    set_flags(argv[1][0] - '0', argv[2][0] - '0', argv[3][0] - '0');
    return 0;
}