#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
  int p[2];

  char *argv[2];
  argv[0] = "wc";
  argv[1] = 0;

  pipe(p);

  if(fork() == 0) {
    close(0);

    dup(p[0]);

    close(p[0]);
    close(p[1]);

    exec("wc", argv);
  } else {
    close(p[0]);

    char buffer[256]; //256 - max size of buffer
    int read_bytes = read(0, buffer, sizeof(buffer));

    write(p[1], buffer, read_bytes);

    close(p[1]);

    wait(0);
  }

  exit(0);
}
