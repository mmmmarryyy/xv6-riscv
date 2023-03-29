#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SYSSLEEPLOCK_INIT     0
#define SYSSLEEPLOCK_DELETE   1
#define SYSSLEEPLOCK_ACQUIRE  2
#define SYSSLEEPLOCK_RELEASE  3
#define STDERR 2


void string_representation_of_syssleeplock_errors(int iderror) {
  if (iderror >= 0) {
    return;
  } else if (iderror == -1) {

    fprintf(STDERR, "something went wrong with idlock\n");
    exit(1);

  } else if (iderror == -2) {

    fprintf(STDERR, "something went wrong in init of syssleeplock function\n");
    exit(1);

  } else if (iderror == -3) {

    fprintf(STDERR, "something went wrong with state of syssleeplock\n");
    exit(1);

  } else if (iderror == -4) {

    fprintf(STDERR, "something went wrong with type_of_request\n");
    exit(1);

  } else {

    fprintf(STDERR, "unknown error\n");
    exit(1);

  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(STDERR, "missing argument: expected at least 2\n");
    exit(1);
  }

  int p_pipe[2];
  int c_pipe[2];

  if (pipe(p_pipe) < 0 || pipe(c_pipe) < 0) {
     printf("pipe creating error\n");
     exit(-1);
   }

  int idlock = syssleeplock(SYSSLEEPLOCK_INIT, 0);

  string_representation_of_syssleeplock_errors(idlock);

  int fork_pid = fork();

  if (fork_pid == -1) {
    close(p_pipe[0]);
    close(p_pipe[1]);
    close(c_pipe[0]);
    close(c_pipe[1]);

    fprintf(STDERR, "unable to fork\n");
    exit(1);
  }

  if (fork_pid == 0) {

    close(p_pipe[1]);
    close(c_pipe[0]);

    char temp_char;

    while (read(p_pipe[0], &temp_char, 1) == 1) {
      string_representation_of_syssleeplock_errors(syssleeplock(SYSSLEEPLOCK_ACQUIRE, idlock));
      printf("%d: received %c\n", getpid(), temp_char);

      string_representation_of_syssleeplock_errors(syssleeplock(SYSSLEEPLOCK_RELEASE, idlock));

      if (write(c_pipe[1], &temp_char, 1) != 1) {
        close(p_pipe[0]);
        close(c_pipe[1]);

        fprintf(STDERR, "unable to write char\n");
        exit(1);
      }
    }

    close(p_pipe[0]);
    close(c_pipe[1]);

  } else {

    close(p_pipe[0]);
    close(c_pipe[1]);

    char* temp_char = argv[1];

    while (*temp_char != 0) {

      if (write(p_pipe[1], temp_char, 1) != 1) {
        close(p_pipe[1]);
        close(c_pipe[0]);

        fprintf(STDERR, "unable to write char\n");
        exit(1);
      }

      temp_char++;
    }

    close(p_pipe[1]);

    char another_temp_char;

    while (read(c_pipe[0], &another_temp_char, 1) == 1) {
      string_representation_of_syssleeplock_errors(syssleeplock(SYSSLEEPLOCK_ACQUIRE, idlock));
      printf("%d: received %c\n", getpid(), another_temp_char);

      string_representation_of_syssleeplock_errors(syssleeplock(SYSSLEEPLOCK_RELEASE, idlock));
    }

    close(c_pipe[0]);
    wait(0);

    syssleeplock(SYSSLEEPLOCK_DELETE, idlock);
  }

  exit(0);
}