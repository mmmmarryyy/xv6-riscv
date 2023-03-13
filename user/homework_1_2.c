#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char** argv) {
  int p_pipe[2];
  int c_pipe[2];

  pipe(p_pipe);
  pipe(c_pipe);

  if(fork() == 0) {
    close(p_pipe[1]);
    close(c_pipe[0]);

    char* temp_char = malloc(1);

    while(read(p_pipe[0], temp_char, 1) == 1) {
      printf("%d: received %c\n", getpid(), *temp_char);
      write(c_pipe[1], temp_char, 1);
    }

    close(p_pipe[0]);
    close(c_pipe[1]);

    free(temp_char);
  } else {
    close(p_pipe[0]);
    close(c_pipe[1]);

    char* temp_char = argv[1];

    while (*temp_char != 0) {
      write(p_pipe[1], temp_char, 1);
      temp_char++;
    }

    close(p_pipe[1]);

    while(read(c_pipe[0], temp_char, 1) == 1) {
      printf("%d: received %c\n", getpid(), *temp_char);
    }

    close(c_pipe[0]);
  }

  exit(0);
}